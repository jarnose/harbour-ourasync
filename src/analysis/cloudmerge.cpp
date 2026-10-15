#include "cloudmerge.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace Oura {

namespace {

using Contributors = std::vector<std::pair<std::string, int>>;

Contributors contributorsOf(const Json *obj)
{
    Contributors out;
    if (!obj || !obj->isObject())
        return out;
    for (const auto &m : obj->members())
        if (m.second.hasNumber())
            out.emplace_back(m.first, static_cast<int>(std::lround(m.second.asNumber())));
    return out;
}

std::optional<int> intOf(const Json &j, const char *key)
{
    if (auto v = j.optNum(key))
        return static_cast<int>(std::lround(*v));
    return std::nullopt;
}

std::optional<double> minutesOf(const Json &j, const char *seconds)
{
    if (auto v = j.optNum(seconds))
        return *v / 60.0;
    return std::nullopt;
}

// Samples of a `{"interval":300,"items":[...],"timestamp":"..."}` series.
std::vector<TimedValue> seriesOf(const Json *obj, int64_t fallbackStart)
{
    std::vector<TimedValue> out;
    if (!obj || !obj->isObject())
        return out;
    const Json *items = obj->get("items");
    if (!items || !items->isArray())
        return out;
    const double interval = obj->optNum("interval").value_or(300);
    int64_t start = fallbackStart;
    if (auto t = parseIso8601(obj->str("timestamp")))
        start = *t;
    for (size_t i = 0; i < items->items().size(); ++i) {
        const Json &v = items->items()[i];
        if (v.hasNumber() && v.asNumber() > 0)
            out.push_back({ start + static_cast<int64_t>(static_cast<double>(i) * interval), v.asNumber() });
    }
    return out;
}

Night nightFromCloud(const Json &j, const AnalysisOptions &options, int64_t start, int64_t end)
{
    Night n;
    n.fromCloud = true;
    n.startUnix = start;
    n.endUnix = end;
    n.date = j.str("day");
    if (n.date.size() < 10)
        n.date = localDate(end, options.utcOffset(end));
    n.date = n.date.substr(0, 10);
    n.inBedMin = j.optNum("time_in_bed").value_or(static_cast<double>(end - start)) / 60.0;

    const auto deep = minutesOf(j, "deep_sleep_duration");
    const auto light = minutesOf(j, "light_sleep_duration");
    const auto rem = minutesOf(j, "rem_sleep_duration");
    const auto awake = minutesOf(j, "awake_time");
    n.deepMin = deep.value_or(0);
    n.lightMin = light.value_or(0);
    n.remMin = rem.value_or(0);
    n.awakeMin = awake.value_or(0);
    if (auto total = minutesOf(j, "total_sleep_duration"))
        n.asleepMin = *total;
    else
        n.asleepMin = n.deepMin + n.lightMin + n.remMin;

    n.efficiencyPct = j.optNum("efficiency");
    n.latencyMin = minutesOf(j, "latency");
    n.wasoMin = awake;
    n.restingHr = j.optNum("lowest_heart_rate");
    n.avgHrv = j.optNum("average_hrv");
    n.avgBreath = j.optNum("average_breath");

    // Oura's phase string: one digit per five minutes, 1 deep, 2 light, 3 REM,
    // 4 awake: the same codes the rest of the app uses.
    // The export also carries a 30-second string; use it when there is one.
    std::string phases = j.str("sleep_phase_30_sec");
    int epochSeconds = 30;
    if (phases.empty()) {
        phases = j.str("sleep_phase_5_min");
        epochSeconds = 300;
    }
    if (!phases.empty()) {
        n.staged = true;
        n.epochSeconds = epochSeconds;
        n.hypnogramStart = start;
        for (char c : phases)
            n.hypnogram.push_back(c >= '1' && c <= '4' ? static_cast<uint8_t>(c - '0') : static_cast<uint8_t>(StageUnknown));

        // Oura's counts need 30-second epochs; five-minute phases are repeated.
        OuraNightEpochs epochsIn;
        epochsIn.epochs = static_cast<int>((end - start) / 30);
        const size_t repeat = static_cast<size_t>(epochSeconds / 30);
        for (uint8_t code : n.hypnogram)
            for (size_t r = 0; r < repeat; ++r)
                epochsIn.stages.push_back(code);
        if (epochsIn.stages.size() > static_cast<size_t>(epochsIn.epochs))
            epochsIn.stages.resize(static_cast<size_t>(epochsIn.epochs));
        int tod = static_cast<int>((((start + options.utcOffset(start)) % 86400) + 86400) % 86400 / 60);
        if (tod >= 720)
            tod -= 1440;
        epochsIn.startMinutesOfDay = tod;
        n.oura = summariseNight(epochsIn);   // motion unknown: no restfulness counts
    }

    n.hr = seriesOf(j.get("heart_rate"), start);
    n.hrv = seriesOf(j.get("hrv"), start);

    // This app's own estimate, from what the cloud says about the night.
    NightInput in;
    in.age = options.ageYears;
    in.asleepMin = n.asleepMin;
    in.efficiencyPct = n.efficiencyPct;
    in.onsetLatencyMin = n.latencyMin;
    in.wasoMin = n.wasoMin;
    if (n.staged && n.asleepMin > 0) {
        in.deepPct = n.deepMin / n.asleepMin * 100.0;
        in.remPct = n.remMin / n.asleepMin * 100.0;
    }
    n.score = scoreNight(in);
    return n;
}

bool overlaps(const Night &n, int64_t start, int64_t end)
{
    return n.startUnix < end && start < n.endUnix;
}

void markMainNights(std::vector<Night> &nights)
{
    std::map<std::string, size_t> longest;
    for (size_t i = 0; i < nights.size(); ++i) {
        auto it = longest.find(nights[i].date);
        if (it == longest.end() || nights[nights[i].date.empty() ? 0 : it->second].asleepMin < nights[i].asleepMin)
            longest[nights[i].date] = i;
    }
    for (size_t i = 0; i < nights.size(); ++i)
        nights[i].main = longest[nights[i].date] == i;
}

Day &dayFor(std::map<std::string, size_t> &index, std::vector<Day> &days, const std::string &date)
{
    auto it = index.find(date);
    if (it != index.end())
        return days[it->second];
    Day d;
    d.date = date;
    d.fromCloud = true;
    d.hr.assign(24, HourStat());
    d.metByHour.assign(24, 0.0);
    days.push_back(std::move(d));
    index[date] = days.size() - 1;
    return days.back();
}

} // namespace

void mergeCloud(Summary &summary, const std::vector<CloudDoc> &docs, const std::vector<CloudHrPoint> &hr,
                const AnalysisOptions &options)
{
    if (docs.empty() && hr.empty())
        return;

    // Last document per kind and day wins (a re-import replaces).
    std::map<std::string, const CloudDoc *> daily[6];
    enum { DSleep, DReadiness, DActivity, DSpo2, DStress, DResilience };
    std::vector<const CloudDoc *> sleepDocs;
    for (const CloudDoc &d : docs) {
        if (d.kind == CloudKind::DailySleep) daily[DSleep][d.day] = &d;
        else if (d.kind == CloudKind::DailyReadiness) daily[DReadiness][d.day] = &d;
        else if (d.kind == CloudKind::DailyActivity) daily[DActivity][d.day] = &d;
        else if (d.kind == CloudKind::DailySpo2) daily[DSpo2][d.day] = &d;
        else if (d.kind == CloudKind::DailyStress) daily[DStress][d.day] = &d;
        else if (d.kind == CloudKind::DailyResilience) daily[DResilience][d.day] = &d;
        else if (d.kind == CloudKind::Sleep) sleepDocs.push_back(&d);
    }

    // ---- nights ----
    for (const CloudDoc *d : sleepDocs) {
        const auto start = parseIso8601(d->json.str("bedtime_start"));
        const auto end = parseIso8601(d->json.str("bedtime_end"));
        if (!start || !end || *end <= *start)
            continue;
        bool covered = false;
        for (const Night &n : summary.nights)
            if (overlaps(n, *start, *end))
                covered = true;
        if (covered)
            continue;
        summary.nights.push_back(nightFromCloud(d->json, options, *start, *end));
    }
    std::sort(summary.nights.begin(), summary.nights.end(),
              [](const Night &a, const Night &b) { return a.startUnix < b.startUnix; });
    markMainNights(summary.nights);

    for (Night &n : summary.nights) {
        if (!n.main)
            continue;
        if (auto it = daily[DSleep].find(n.date); it != daily[DSleep].end()) {
            n.ouraScore = intOf(it->second->json, "score");
            n.ouraContributors = contributorsOf(it->second->json.get("contributors"));
        }
        if (!n.tempDeviation) {
            if (auto it = daily[DReadiness].find(n.date); it != daily[DReadiness].end())
                n.tempDeviation = it->second->json.optNum("temperature_deviation");
        }
        if (!n.avgSpo2) {
            if (auto it = daily[DSpo2].find(n.date); it != daily[DSpo2].end())
                if (const Json *p = it->second->json.get("spo2_percentage"))
                    n.avgSpo2 = p->optNum("average");
        }
    }

    // ---- days ----
    std::map<std::string, size_t> index;
    for (size_t i = 0; i < summary.days.size(); ++i)
        index[summary.days[i].date] = i;

    for (const auto &entry : daily[DActivity]) {
        const Json &j = entry.second->json;
        const bool existed = index.count(entry.first) > 0;
        Day &d = dayFor(index, summary.days, entry.first);
        d.steps = intOf(j, "steps");
        d.totalKcal = intOf(j, "total_calories");
        d.ouraActivityScore = intOf(j, "score");
        d.activityContributors = contributorsOf(j.get("contributors"));
        if (!existed) {
            d.activeKcal = j.optNum("active_calories").value_or(0);
            d.lightMin = static_cast<int>(j.num("low_activity_time") / 60);
            d.moderateMin = static_cast<int>(j.num("medium_activity_time") / 60);
            d.vigorousMin = static_cast<int>(j.num("high_activity_time") / 60);
            d.sedentaryMin = static_cast<int>(j.num("sedentary_time") / 60);
            d.coveredMin = d.lightMin + d.moderateMin + d.vigorousMin + d.sedentaryMin;
            d.avgMet = j.optNum("average_met_minutes");
            // MET per minute, folded to local hours: the series starts at local
            // midnight.
            if (const Json *met = j.get("met")) {
                const double interval = met->optNum("interval").value_or(60);
                const Json *items = met->get("items");
                std::vector<double> sum(24, 0.0);
                std::vector<int> count(24, 0);
                if (items)
                    for (size_t i = 0; i < items->items().size(); ++i) {
                        const Json &v = items->items()[i];
                        if (!v.hasNumber())
                            continue;
                        const size_t hour = std::min<size_t>(23, static_cast<size_t>(static_cast<double>(i) * interval / 3600.0));
                        sum[hour] += v.asNumber();
                        ++count[hour];
                    }
                for (size_t h = 0; h < 24; ++h)
                    d.metByHour[h] = count[h] ? sum[h] / count[h] : 0.0;
            }
        }
    }
    for (const auto &entry : daily[DReadiness]) {
        Day &d = dayFor(index, summary.days, entry.first);
        d.readinessScore = intOf(entry.second->json, "score");
        d.readinessContributors = contributorsOf(entry.second->json.get("contributors"));
        d.temperatureDeviation = entry.second->json.optNum("temperature_deviation");
    }
    for (const auto &entry : daily[DStress]) {
        Day &d = dayFor(index, summary.days, entry.first);
        const Json &j = entry.second->json;
        if (auto v = j.optNum("stress_high"))
            d.stressHighMin = static_cast<int>(*v / 60);
        if (auto v = j.optNum("recovery_high"))
            d.recoveryHighMin = static_cast<int>(*v / 60);
        d.stressSummary = j.str("day_summary");
    }

    // Heart rate by local hour, for the days the ring did not supply any.
    if (!hr.empty()) {
        std::map<std::string, std::vector<std::vector<double>>> perDay;
        for (const CloudHrPoint &p : hr) {
            const int off = options.utcOffset(p.utc);
            const std::string date = localDate(p.utc, off);
            auto it = index.find(date);
            if (it == index.end() || !summary.days[it->second].fromCloud)
                continue;
            auto &hours = perDay[date];
            if (hours.empty())
                hours.resize(24);
            const int64_t local = p.utc + off;
            const int hour = static_cast<int>(((local % 86400) + 86400) % 86400 / 3600);
            hours[static_cast<size_t>(hour)].push_back(p.bpm);
        }
        for (auto &entry : perDay)
            for (size_t h = 0; h < 24; ++h)
                if (!entry.second[h].empty())
                    summary.days[index[entry.first]].hr[h] = hourStatOf(entry.second[h]);
    }

    std::sort(summary.days.begin(), summary.days.end(), [](const Day &a, const Day &b) { return a.date < b.date; });
}

} // namespace Oura

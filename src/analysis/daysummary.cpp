#include "daysummary.h"
#include "stager.h"

#include "ring/events.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <unordered_map>

namespace Oura {

// ---- calendar ---------------------------------------------------------------

int64_t daysFromCivil(int64_t y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<int64_t>(doe) - 719468;
}

namespace {

void civilFromDays(int64_t z, int64_t &y, unsigned &m, unsigned &d)
{
    z += 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y += m <= 2;
}

int64_t floorDiv(int64_t a, int64_t b)
{
    int64_t q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0)))
        --q;
    return q;
}

} // namespace

std::string localDate(int64_t utc, int offsetSeconds)
{
    int64_t y;
    unsigned m, d;
    civilFromDays(floorDiv(utc + offsetSeconds, 86400), y, m, d);
    char buf[48];
    std::snprintf(buf, sizeof buf, "%04lld-%02u-%02u", static_cast<long long>(y), m, d);
    return buf;
}

// ---- temperature ------------------------------------------------------------

std::optional<double> nightlyTemperature(const std::vector<double> &celsius)
{
    constexpr size_t kWindow = 30;
    constexpr int kRangeThreshold = 250; // 2.50 C in centi-degrees
    constexpr size_t kMinWindows = 4;

    uint16_t ring[7] = { 0, 0, 0, 0, 0, 0, 0 };
    size_t idx = 0;
    int winMin = 65535;
    int winMax = 0;
    std::vector<int> maxima;

    for (size_t i = 0; i < celsius.size(); ++i) {
        const double centi = std::round(celsius[i] * 100.0);
        ring[idx] = static_cast<uint16_t>(std::min(65535.0, std::max(0.0, centi)));
        idx = idx == 6 ? 0 : idx + 1;
        uint16_t sorted[7];
        std::copy(ring, ring + 7, sorted);
        std::sort(sorted, sorted + 7);
        const int median = sorted[3];
        if (median != 0) {
            winMin = std::min(winMin, median);
            winMax = std::max(winMax, median);
        }
        if ((i + 1) % kWindow == 0) {
            if (winMax >= winMin && winMax != 0 && winMax - winMin < kRangeThreshold)
                maxima.push_back(winMax);
            winMin = 65535;
            winMax = 0;
        }
    }
    if (maxima.size() < kMinWindows)
        return std::nullopt;
    return *std::min_element(maxima.begin(), maxima.end()) / 100.0;
}

// ---- helpers ----------------------------------------------------------------

namespace {

constexpr int kEpochSeconds = 30;            // one sleep_phase_data epoch
constexpr double kMinWindowSeconds = 20 * 60;
constexpr double kMaxWindowSeconds = 20 * 3600;
constexpr double kMergeGapSeconds = 60 * 60; // a short wake does not split a night

struct Window
{
    double start = 0;
    double end = 0;
    bool approx = false;
};

struct Run
{
    double start = 0;
    double end = 0;
    std::vector<uint8_t> codes;
    bool approx = false;
};

double median(std::vector<double> v)
{
    if (v.empty())
        return 0;
    std::sort(v.begin(), v.end());
    const size_t mid = v.size() / 2;
    return v.size() % 2 ? v[mid] : (v[mid - 1] + v[mid]) / 2.0;
}

double mean(const std::vector<double> &v)
{
    double s = 0;
    for (double x : v)
        s += x;
    return v.empty() ? 0 : s / static_cast<double>(v.size());
}

void sortByTime(std::vector<TimedValue> &v)
{
    std::sort(v.begin(), v.end(), [](const TimedValue &a, const TimedValue &b) { return a.utc < b.utc; });
}

std::vector<TimedValue> slice(const std::vector<TimedValue> &v, double from, double to)
{
    std::vector<TimedValue> out;
    for (const TimedValue &p : v)
        if (p.utc >= from && p.utc <= to)
            out.push_back(p);
    return out;
}

// Plausible heart rate for a wrist or finger PPG estimate. The same gate the
// decoders upstream apply: one misdetected beat must not become "your peak".
bool plausibleBpm(double bpm)
{
    return bpm >= 30.0 && bpm <= 240.0;
}

uint8_t stageOfPhase(uint8_t phase)
{
    switch (phase) {
    case PhaseDeep: return StageDeep;
    case PhaseLight: return StageLight;
    case PhaseRem: return StageRem;
    default: return StageAwake;
    }
}

int bouts(const std::vector<uint8_t> &seq, size_t from, size_t to, uint8_t code, size_t minLen)
{
    int count = 0;
    size_t run = 0;
    for (size_t i = from; i <= to && i < seq.size(); ++i) {
        if (seq[i] == code) {
            ++run;
        } else {
            if (run >= minLen)
                ++count;
            run = 0;
        }
    }
    if (run >= minLen)
        ++count;
    return count;
}

} // namespace

// ---- the summary ------------------------------------------------------------

HourStat hourStatOf(std::vector<double> &values)
{
    HourStat s;
    if (values.empty())
        return s;
    // The 5th to 95th percentile, so one bad estimate is not "your peak heart
    // rate".
    std::sort(values.begin(), values.end());
    const size_t n = values.size();
    s.count = static_cast<int>(n);
    s.min = values[static_cast<size_t>(0.05 * static_cast<double>(n - 1))];
    s.max = values[static_cast<size_t>(std::ceil(0.95 * static_cast<double>(n - 1)))];
    s.mean = mean(values);
    return s;
}

Summary summarize(const std::vector<StoredEvent> &events, const AnalysisOptions &options)
{
    Summary out;
    RingClock clock(events);

    const auto offsetAt = [&](double utc) { return options.utcOffset(static_cast<int64_t>(utc)); };

    // Pass 1: one pass over the events, collecting typed series with real times.
    std::vector<Window> windows;
    std::vector<Run> runs;
    std::vector<TimedValue> hr5;       // hrv_event heart-rate averages
    std::vector<TimedValue> hrvSeries; // hrv_event RMSSD
    std::vector<TimedValue> beats;     // quality-gated green beats
    std::vector<TimedValue> sleepHr;   // per-window HR, asleep windows only
    std::vector<TimedValue> sleepBreath;
    std::vector<TimedValue> sleepWindows; // value = sleep_state, every record
    std::vector<TimedValue> sleepTemp;    // 0x75
    std::vector<TimedValue> periodTemp;   // 0x69
    std::vector<TimedValue> probeTemp;    // 0x46, first probe
    std::vector<TimedValue> spo2;
    std::vector<TimedValue> motionSeconds; // motion_event: seconds of motion
    std::vector<TimedValue> motionCounts;  // sleep_period_information_2: motion per window, a stager input
    std::vector<TimedValue> motionStates;  // motion_period: one state per 30 s epoch, at the epoch's end
    std::unordered_map<int64_t, double> metByMinute;
    std::optional<std::pair<int64_t, int>> battery; // unix, percent

    int prevPage = -1;
    int prevRunEpoch = -1;

    for (size_t i = 0; i < events.size(); ++i) {
        const StoredEvent &e = events[i];
        const int epoch = clock.epochOf(i);
        const double t = clock.utc(epoch, e.ds);
        if (std::isnan(t)) {
            ++out.unplacedEvents;
            continue;
        }
        const bool approx = !clock.anchored(epoch);

        switch (e.tag) {
        case Tag::BedtimePeriod: {
            if (auto b = decodeBedtime(e.body)) {
                Window w;
                w.start = clock.utc(epoch, b->startDs);
                w.end = clock.utc(epoch, b->endDs);
                w.approx = approx;
                const double len = w.end - w.start;
                if (len >= kMinWindowSeconds && len <= kMaxWindowSeconds)
                    windows.push_back(w);
            }
            break;
        }
        case Tag::SleepPhaseData: {
            if (auto page = decodeSleepPhasePage(e.body)) {
                // Pages arrive as a burst numbered from zero; a page that does
                // not follow the previous one starts a new analysed sleep.
                const bool continues = !runs.empty() && page->page == prevPage + 1 && epoch == prevRunEpoch;
                if (!continues) {
                    runs.emplace_back();
                    runs.back().approx = approx;
                }
                Run &run = runs.back();
                for (uint8_t phase : page->phases)
                    run.codes.push_back(stageOfPhase(phase));
                // The pages carry only when the ring emitted them, which is
                // when its analysis finished: the end of the sleep. So a run
                // ends at its last page and runs back at 30 s an epoch.
                run.end = t;
                prevPage = page->page;
                prevRunEpoch = epoch;
            }
            break;
        }
        case Tag::Hrv: {
            if (auto h = decodeHrv(e.body)) {
                for (size_t k = 0; k < h->hr.size(); ++k) {
                    const double at = t + static_cast<double>(k) * 300.0;
                    if (plausibleBpm(h->hr[k]))
                        hr5.push_back({ static_cast<int64_t>(at), static_cast<double>(h->hr[k]) });
                    if (h->rmssd[k] > 0)
                        hrvSeries.push_back({ static_cast<int64_t>(at), static_cast<double>(h->rmssd[k]) });
                }
            }
            break;
        }
        case Tag::GreenIbiQuality: {
            for (const GreenBeat &b : decodeGreenIbi(e.body))
                if (b.good() && plausibleBpm(b.bpm()))
                    beats.push_back({ static_cast<int64_t>(t), static_cast<double>(b.bpm()) });
            break;
        }
        case Tag::SleepPeriodInfo2: {
            if (auto p = decodeSleepPeriod(e.body)) {
                sleepWindows.push_back({ static_cast<int64_t>(t), static_cast<double>(p->sleepState) });
                motionCounts.push_back({ static_cast<int64_t>(t), static_cast<double>(std::min(30, p->motionCount)) });
                if (p->sleepState >= 1) {
                    if (plausibleBpm(p->averageHr))
                        sleepHr.push_back({ static_cast<int64_t>(t), p->averageHr });
                    if (p->breath >= 4 && p->breath <= 40)
                        sleepBreath.push_back({ static_cast<int64_t>(t), p->breath });
                }
            }
            break;
        }
        case Tag::SleepTemp:
            for (double c : decodeTemperatures(e.body))
                sleepTemp.push_back({ static_cast<int64_t>(t), c });
            break;
        case Tag::TempPeriod:
            for (double c : decodeTemperatures(e.body))
                periodTemp.push_back({ static_cast<int64_t>(t), c });
            break;
        case Tag::Temp: {
            auto temps = decodeTemperatures(e.body);
            if (!temps.empty())
                probeTemp.push_back({ static_cast<int64_t>(t), temps.front() });
            break;
        }
        case Tag::Spo2: {
            const std::vector<int> v = decodeSpo2(e.body);
            std::vector<double> valid;
            for (int p : v)
                if (p >= 70 && p <= 100)
                    valid.push_back(p);
            if (!valid.empty())
                spo2.push_back({ static_cast<int64_t>(t), mean(valid) });
            break;
        }
        case Tag::Spo2RPi: {
            std::vector<double> pct;
            for (const Spo2Sample &s : decodeSpo2RPi(e.body))
                pct.push_back(spo2PercentFromR(s.r));
            if (!pct.empty())
                spo2.push_back({ static_cast<int64_t>(t), mean(pct) });
            break;
        }
        case Tag::Motion: {
            if (auto m = decodeMotionSeconds(e.body))
                motionSeconds.push_back({ static_cast<int64_t>(t), static_cast<double>(*m) });
            break;
        }
        case Tag::MotionPeriod: {
            if (auto m = decodeMotionPeriod(e.body)) {
                // The codes end at the event's time, 30 s each.
                const size_t n = m->states.size();
                for (size_t k = 0; k < n; ++k)
                    motionStates.push_back({ static_cast<int64_t>(t) - static_cast<int64_t>((n - 1 - k) * kEpochSeconds),
                                             static_cast<double>(m->states[k]) });
            }
            break;
        }
        case Tag::ActivityInfo: {
            if (auto a = decodeActivityMet(e.body)) {
                for (size_t k = 0; k < a->met.size(); ++k) {
                    const int64_t minute = static_cast<int64_t>(std::floor(t / 60.0)) + static_cast<int64_t>(k);
                    metByMinute[minute] = a->met[k];
                }
            }
            break;
        }
        case Tag::DebugData: {
            if (auto b = decodeBatteryLog(e.body)) {
                if (!battery || static_cast<int64_t>(t) >= battery->first)
                    battery = std::make_pair(static_cast<int64_t>(t), b->percent);
            }
            break;
        }
        default:
            break;
        }
    }

    // A run's start follows from where it ends. Done once the run is complete.
    for (Run &run : runs)
        run.start = run.end - static_cast<double>(run.codes.size()) * kEpochSeconds;

    sortByTime(hr5);
    sortByTime(hrvSeries);
    sortByTime(beats);
    sortByTime(sleepHr);
    sortByTime(sleepBreath);
    sortByTime(sleepWindows);
    sortByTime(spo2);
    sortByTime(motionSeconds);
    sortByTime(motionStates);
    sortByTime(motionCounts);

    if (battery) {
        out.ringBatteryPercent = battery->second;
        out.ringBatteryUnix = battery->first;
    }

    // Newest heart rate: a quality-gated beat, else the latest five-minute mean.
    {
        const TimedValue *newest = nullptr;
        if (!beats.empty())
            newest = &beats.back();
        if (!hr5.empty() && (!newest || hr5.back().utc > newest->utc))
            newest = &hr5.back();
        if (newest) {
            out.latestHr = newest->value;
            out.latestHrUnix = newest->utc;
        }
    }

    // ---- sleep windows -------------------------------------------------------
    // A short wake can leave two adjacent bedtime records, and the ring can
    // re-run its analysis over the same night, so overlapping or nearly
    // touching windows are one night.
    std::sort(windows.begin(), windows.end(), [](const Window &a, const Window &b) { return a.start < b.start; });

    // A staged sleep whose window the ring never reported still happened.
    for (const Run &run : runs) {
        if (run.codes.empty())
            continue;
        bool covered = false;
        for (const Window &w : windows)
            if (run.end > w.start && run.start < w.end)
                covered = true;
        if (!covered && run.end - run.start >= kMinWindowSeconds)
            windows.push_back({ run.start, run.end, run.approx });
    }
    std::sort(windows.begin(), windows.end(), [](const Window &a, const Window &b) { return a.start < b.start; });

    std::vector<Window> merged;
    for (const Window &w : windows) {
        if (!merged.empty() && w.start <= merged.back().end + kMergeGapSeconds) {
            merged.back().end = std::max(merged.back().end, w.end);
            merged.back().approx = merged.back().approx || w.approx;
        } else {
            merged.push_back(w);
        }
    }

    // ---- one night per window ------------------------------------------------
    for (const Window &w : merged) {
        Night n;
        n.startUnix = static_cast<int64_t>(w.start);
        n.endUnix = static_cast<int64_t>(w.end);
        n.approximateClock = w.approx;
        n.date = localDate(n.endUnix, offsetAt(w.end));
        n.inBedMin = (w.end - w.start) / 60.0;

        // Hypnogram: paint every analysed run that overlaps the window.
        const size_t cells = static_cast<size_t>(std::max(1.0, std::ceil((w.end - w.start) / kEpochSeconds)));
        std::vector<uint8_t> stages(cells, StageUnknown);
        bool painted = false;
        const double windowEpochs = (w.end - w.start) / kEpochSeconds;
        for (const Run &run : runs) {
            if (run.end <= w.start || run.start >= w.end)
                continue;
            // Where does the run begin? The pages say only when the ring
            // emitted them, which is after the analysis. Staging is written
            // from the start of the sleep in pages of 52 epochs, so a run just
            // one page longer than the window is the window plus the padding of
            // its last page: anchor it to the window's start, and the padding
            // falls off the end. A run of any other length cannot be matched to
            // this window that way, so it ends where it was emitted, which is
            // how open_health places it (checked there against heart rate).
            double runStart = run.start;
            const double epochs = static_cast<double>(run.codes.size());
            if (epochs >= windowEpochs - 2.0 && epochs <= windowEpochs + 60.0)
                runStart = w.start;
            for (size_t k = 0; k < run.codes.size(); ++k) {
                const double at = runStart + static_cast<double>(k) * kEpochSeconds;
                if (at < w.start || at >= w.end)
                    continue;
                const size_t cell = std::min(cells - 1, static_cast<size_t>((at - w.start) / kEpochSeconds));
                stages[cell] = run.codes[k];
                painted = true;
            }
        }

        // A ring that does not stage sleep itself (Ring 4 and 5) sends the signals the stages
        // can be estimated from.
        if (!painted) {
            std::vector<TimedValue> hrFor = slice(hr5, w.start - 600, w.end + 600);
            if (hrFor.size() < 12)
                hrFor = slice(sleepHr, w.start - 600, w.end + 600);
            std::vector<TimedValue> motionFor = slice(motionSeconds, w.start, w.end);
            if (motionFor.empty())
                motionFor = slice(motionCounts, w.start, w.end);
            const std::vector<uint8_t> estimate =
                    stageFromSignals(w.start, w.end, hrFor, slice(hrvSeries, w.start - 600, w.end + 600), motionFor);
            if (!estimate.empty()) {
                for (size_t k = 0; k < estimate.size() && k < stages.size(); ++k)
                    stages[k] = estimate[k];
                painted = true;
                n.stagesEstimated = true;
            }
        }

        // Oura's own counts from the engine's state machine, for a night staged in
        // 30-second epochs.
        if (painted) {
            OuraNightEpochs epochsIn;
            epochsIn.epochs = static_cast<int>(std::floor((w.end - w.start) / kEpochSeconds));
            epochsIn.stages.assign(stages.begin(), stages.begin() + std::min<size_t>(stages.size(), static_cast<size_t>(epochsIn.epochs)));
            // The ring's own motion states (one per epoch) when it sent them, else the seconds
            // of motion of the 0x47 events summed per epoch. Either way the engine sees a
            // value per epoch, nonzero where there was movement.
            const auto states = slice(motionStates, w.start, w.end);
            const auto motion = slice(motionSeconds, w.start, w.end);
            if ((!states.empty() || !motion.empty()) && epochsIn.epochs > 0) {
                epochsIn.motion.assign(static_cast<size_t>(epochsIn.epochs), 0);
                const bool fromStates = !states.empty();
                for (const TimedValue &m : fromStates ? states : motion) {
                    const int64_t k = static_cast<int64_t>((static_cast<double>(m.utc) - w.start) / kEpochSeconds);
                    if (k < 0 || k >= epochsIn.epochs)
                        continue;
                    // A state is 0 none, 1 restless, 2 tossing, 3 active: sizes, roughly, in seconds.
                    static const int kStateSeconds[4] = { 0, 4, 12, 28 };
                    const int add = fromStates ? kStateSeconds[static_cast<size_t>(m.value) & 3] : static_cast<int>(m.value);
                    const int v = epochsIn.motion[static_cast<size_t>(k)] + add;
                    epochsIn.motion[static_cast<size_t>(k)] = static_cast<uint8_t>(std::min(v, 255));
                }
                n.ouraMotionKnown = true;
            }
            int tod = static_cast<int>((((static_cast<int64_t>(w.start) + offsetAt(w.start)) % 86400) + 86400) % 86400 / 60);
            if (tod >= 720)
                tod -= 1440;
            epochsIn.startMinutesOfDay = tod;
            n.oura = summariseNight(epochsIn);
        }

        const auto windowedHr = slice(hr5, w.start, w.end);
        const auto windowedSleepHr = slice(sleepHr, w.start, w.end);

        if (painted) {
            n.staged = true;
            n.hypnogramStart = n.startUnix;
            n.hypnogram = stages;

            size_t known = 0;
            for (uint8_t c : stages) {
                if (c == StageUnknown)
                    continue;
                ++known;
                const double minutes = kEpochSeconds / 60.0;
                switch (c) {
                case StageDeep: n.deepMin += minutes; break;
                case StageLight: n.lightMin += minutes; break;
                case StageRem: n.remMin += minutes; break;
                default: n.awakeMin += minutes; break;
                }
            }
            n.asleepMin = n.deepMin + n.lightMin + n.remMin;

            // Sleep latency, wake after onset and awakenings, from the first
            // and last asleep epoch.
            size_t onset = stages.size(), last = 0;
            for (size_t k = 0; k < stages.size(); ++k) {
                if (stages[k] >= StageDeep && stages[k] <= StageRem) {
                    onset = std::min(onset, k);
                    last = k;
                }
            }
            if (onset < stages.size()) {
                n.latencyMin = static_cast<double>(onset) * kEpochSeconds / 60.0;
                size_t wakeEpochs = 0;
                for (size_t k = onset; k <= last; ++k)
                    if (stages[k] == StageAwake)
                        ++wakeEpochs;
                n.wasoMin = static_cast<double>(wakeEpochs) * kEpochSeconds / 60.0;
                // An awakening: at least a minute awake inside the sleep span.
                n.awakenings = bouts(stages, onset, last, StageAwake, 2);
            }
            // Efficiency is only honest when nearly the whole night was staged:
            // unanalysed epochs are not evidence of wakefulness.
            if (known * 10 >= cells * 9)
                n.efficiencyPct = n.asleepMin / (static_cast<double>(known) * kEpochSeconds / 60.0) * 100.0;
        } else {
            // Not staged: the window is the ring's own call that this was
            // sleep. Use its per-window records to see how much of it was.
            const auto records = slice(sleepWindows, w.start, w.end);
            size_t asleepRecords = 0;
            for (const TimedValue &r : records)
                if (r.value >= 1)
                    ++asleepRecords;
            n.asleepEstimated = true;
            n.asleepMin = records.size() >= 10
                    ? n.inBedMin * static_cast<double>(asleepRecords) / static_cast<double>(records.size())
                    : n.inBedMin;
        }

        // Physiology through the night.
        n.hr = windowedHr;
        n.hr.insert(n.hr.end(), windowedSleepHr.begin(), windowedSleepHr.end());
        sortByTime(n.hr);
        n.hrv = slice(hrvSeries, w.start, w.end);
        n.breath = slice(sleepBreath, w.start, w.end);
        n.spo2 = slice(spo2, w.start, w.end);

        if (!windowedHr.empty()) {
            double lowest = 1e9;
            for (const TimedValue &p : windowedHr)
                lowest = std::min(lowest, p.value);
            n.restingHr = lowest;
        } else if (!windowedSleepHr.empty()) {
            double lowest = 1e9;
            for (const TimedValue &p : windowedSleepHr)
                lowest = std::min(lowest, p.value);
            n.restingHr = lowest;
        }
        if (!n.hrv.empty()) {
            std::vector<double> v;
            for (const TimedValue &p : n.hrv)
                v.push_back(p.value);
            n.avgHrv = mean(v);
        }
        if (!n.breath.empty()) {
            std::vector<double> v;
            for (const TimedValue &p : n.breath)
                v.push_back(p.value);
            n.avgBreath = median(v);
        }
        if (!n.spo2.empty()) {
            std::vector<double> v;
            for (const TimedValue &p : n.spo2)
                v.push_back(p.value);
            n.avgSpo2 = mean(v);
        }

        // Skin temperature: the dedicated sleep-temperature stream when it has
        // enough samples, else everything the ring logged in the window.
        {
            std::vector<TimedValue> series = slice(sleepTemp, w.start, w.end);
            if (series.size() < 120) {
                series = slice(sleepTemp, w.start, w.end);
                const auto period = slice(periodTemp, w.start, w.end);
                const auto probe = slice(probeTemp, w.start, w.end);
                series.insert(series.end(), period.begin(), period.end());
                series.insert(series.end(), probe.begin(), probe.end());
                sortByTime(series);
            }
            n.temp = series;
            if (!series.empty()) {
                std::vector<double> v;
                for (const TimedValue &p : series)
                    v.push_back(p.value);
                n.skinTemp = nightlyTemperature(v);
                if (!n.skinTemp)
                    n.skinTemp = mean(v);
            }
        }

        out.nights.push_back(std::move(n));
    }

    // Per date, the longest sleep is the night and the rest are naps.
    {
        std::map<std::string, size_t> longest;
        for (size_t i = 0; i < out.nights.size(); ++i) {
            const Night &n = out.nights[i];
            auto it = longest.find(n.date);
            if (it == longest.end() || out.nights[it->second].asleepMin < n.asleepMin)
                longest[n.date] = i;
        }
        for (size_t i = 0; i < out.nights.size(); ++i)
            out.nights[i].main = longest[out.nights[i].date] == i;
    }

    // Scores need the wearer's own baselines, taken only from nights before the
    // one being scored.
    {
        std::vector<double> priorRhr, priorHrv, priorTemp;
        for (Night &n : out.nights) {
            const auto lastN = [](const std::vector<double> &v, size_t count) {
                return v.size() > count ? std::vector<double>(v.end() - static_cast<long>(count), v.end()) : v;
            };
            if (n.skinTemp) {
                const std::vector<double> base = lastN(priorTemp, 14);
                if (base.size() >= 3)
                    n.tempDeviation = *n.skinTemp - mean(base);
            }

            NightInput in;
            in.age = options.ageYears;
            in.asleepMin = n.asleepMin;
            if (n.efficiencyPct)
                in.efficiencyPct = n.efficiencyPct;
            if (n.latencyMin)
                in.onsetLatencyMin = n.latencyMin;
            if (n.wasoMin)
                in.wasoMin = n.wasoMin;
            if (n.awakenings)
                in.awakenings = *n.awakenings;
            if (n.staged && n.asleepMin > 0) {
                in.deepPct = n.deepMin / n.asleepMin * 100.0;
                in.remPct = n.remMin / n.asleepMin * 100.0;
            }
            if (n.restingHr) {
                in.restingHr = n.restingHr;
                in.restingHrBaseline = causalBaseline(lastN(priorRhr, 14));
            }
            if (n.avgHrv) {
                in.hrvMs = n.avgHrv;
                in.hrvBaseline = causalBaseline(lastN(priorHrv, 14));
            }
            if (n.main)
                n.score = scoreNight(in);

            // Only a real night feeds the baselines; a nap would drag them.
            if (n.main && n.asleepMin >= 180) {
                if (n.restingHr)
                    priorRhr.push_back(*n.restingHr);
                if (n.avgHrv)
                    priorHrv.push_back(*n.avgHrv);
                if (n.skinTemp)
                    priorTemp.push_back(*n.skinTemp);
            }
        }
    }

    // ---- days ----------------------------------------------------------------
    {
        struct HourAccumulator
        {
            std::vector<double> values;
            double metSum = 0;
            int metCount = 0;
        };
        std::map<std::string, std::vector<HourAccumulator>> byDay;
        std::map<std::string, Day> days;

        const auto dayOf = [&](double utc, int &hour) {
            const int off = offsetAt(utc);
            const int64_t local = static_cast<int64_t>(std::floor(utc)) + off;
            hour = static_cast<int>(floorDiv(local, 3600) % 24 + 24) % 24;
            return localDate(static_cast<int64_t>(utc), off);
        };
        const auto accumulatorsFor = [&](const std::string &date) -> std::vector<HourAccumulator> & {
            auto it = byDay.find(date);
            if (it == byDay.end())
                it = byDay.emplace(date, std::vector<HourAccumulator>(24)).first;
            return it->second;
        };

        for (const std::vector<TimedValue> *series : { &beats, &hr5, &sleepHr }) {
            for (const TimedValue &p : *series) {
                int hour = 0;
                const std::string date = dayOf(static_cast<double>(p.utc), hour);
                accumulatorsFor(date)[static_cast<size_t>(hour)].values.push_back(p.value);
            }
        }

        std::map<std::string, double> metTotals;
        std::map<std::string, int> metMinutes;
        std::map<std::string, std::vector<double>> minuteMet;   // per local minute of the day, NaN where none
        for (const auto &entry : metByMinute) {
            const double utc = static_cast<double>(entry.first) * 60.0;
            int hour = 0;
            const std::string date = dayOf(utc, hour);
            const double met = entry.second;

            auto &acc = accumulatorsFor(date)[static_cast<size_t>(hour)];
            acc.metSum += met;
            ++acc.metCount;

            Day &day = days[date];
            day.date = date;
            ++day.coveredMin;
            if (met >= 6.0)
                ++day.vigorousMin;
            else if (met >= 3.0)
                ++day.moderateMin;
            else if (met >= 1.5)
                ++day.lightMin;
            else
                ++day.sedentaryMin;
            // Minutes below 2 MET are not counted: the cloud's active calories fit best that way.
            if (met >= 2.0)
                day.activeKcal += (met - 1.0) * options.weightKg / 60.0;
            {
                const int64_t local = static_cast<int64_t>(std::floor(utc)) + offsetAt(utc);
                const size_t minute = static_cast<size_t>(((local % 86400) + 86400) % 86400 / 60);
                auto &m = minuteMet[date];
                if (m.empty())
                    m.assign(1440, std::nan(""));
                m[minute] = met;
            }
            metTotals[date] += met;
            ++metMinutes[date];
        }

        for (auto &entry : byDay) {
            Day &day = days[entry.first];
            day.date = entry.first;
            day.hr.assign(24, HourStat());
            day.metByHour.assign(24, 0.0);
            for (size_t h = 0; h < 24; ++h) {
                HourAccumulator &acc = entry.second[h];
                if (acc.metCount > 0)
                    day.metByHour[h] = acc.metSum / acc.metCount;
                if (acc.values.empty())
                    continue;
                day.hr[h] = hourStatOf(acc.values);
            }
            if (metMinutes[entry.first] > 0)
                day.avgMet = metTotals[entry.first] / metMinutes[entry.first];
            day.activeKcal = std::round(day.activeKcal);
        }
        // An activity score for every day that has the six before it.
        {
            const auto dateBack = [](const std::string &date, int back) {
                const int y = std::atoi(date.substr(0, 4).c_str());
                const unsigned m = static_cast<unsigned>(std::atoi(date.substr(5, 2).c_str()));
                const unsigned d = static_cast<unsigned>(std::atoi(date.substr(8, 2).c_str()));
                return localDate((daysFromCivil(y, m, d) - back) * 86400, 0);
            };
            std::map<std::string, Day>::iterator it = days.begin();
            for (; it != days.end(); ++it) {
                Day &day = it->second;
                const auto mm = minuteMet.find(day.date);
                if (day.coveredMin < 720 || mm == minuteMet.end())
                    continue;
                std::vector<ActivityDay> week;
                for (int back = 6; back >= 0; --back) {
                    const std::string date = dateBack(day.date, back);
                    const auto dm = days.find(date);
                    const auto mv = minuteMet.find(date);
                    if (dm == days.end() || mv == minuteMet.end() || dm->second.coveredMin < 300)
                        break;
                    week.push_back(activityDayFromMinutes(mv->second, dm->second.activeKcal, options.activeKcalGoal));
                }
                if (const auto score = activityScore(week)) {
                    day.estimatedActivityScore = score->score;
                    day.estimatedActivityContributors.assign(score->contributors.begin(), score->contributors.end());
                }
            }
        }
        for (auto &entry : days)
            out.days.push_back(std::move(entry.second));
    }

    return out;
}

} // namespace Oura

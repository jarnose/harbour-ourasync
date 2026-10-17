#include "readiness_estimate.h"

#include "oura_night_score.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>
#include <optional>
#include <utility>

namespace Oura {

namespace {

using Curve = std::vector<std::pair<double, double>>;

double interpolate(const Curve &c, double x)
{
    if (x <= c.front().first)
        return c.front().second;
    if (x >= c.back().first)
        return c.back().second;
    for (size_t i = 1; i < c.size(); ++i)
        if (x <= c[i].first) {
            const double span = c[i].first - c[i - 1].first;
            const double t = span > 0 ? (x - c[i - 1].first) / span : 0;
            return c[i - 1].second + t * (c[i].second - c[i - 1].second);
        }
    return c.back().second;
}

// The fitted curves: median contributor per bin of the quantity named beside each.
const Curve kPreviousNight = {   // the night's sleep score
    { 66, 49.5 }, { 72, 64 }, { 75, 70 }, { 77, 75 }, { 78, 77 }, { 79, 80 }, { 80, 82 }, { 81, 84.5 },
    { 82, 88 }, { 83, 91 }, { 84, 93 }, { 85, 94 }, { 86, 96.5 }, { 87, 99 }, { 89, 100 },
};
const Curve kBodyTemperature = {   // minus the size of the night's temperature deviation, deg C
    { -0.48, 71.5 }, { -0.35, 91.5 }, { -0.28, 95 }, { -0.23, 96 }, { -0.2, 98 }, { -0.15, 99 }, { -0.09, 100 },
};
const Curve kRestingHr = {   // minus (the night's lowest heart rate minus its 60-day median), bpm
    { -6, 41 }, { -3, 64 }, { -2, 74 }, { -1, 81 }, { 0, 90 }, { 1, 96 }, { 2, 100 },
};
const Curve kHrvBalance = {   // the last 7 nights' HRV over the 60 nights before
    { 0.858, 68 }, { 0.903, 74 }, { 0.926, 75 }, { 0.944, 78 }, { 0.956, 81 }, { 0.982, 85 }, { 1.005, 85 },
    { 1.036, 87 }, { 1.065, 88 }, { 1.087, 90 }, { 1.158, 91 }, { 1.197, 92 },
};
const Curve kSleepBalance = {   // mean sleep of the last 7 nights, hours
    { 6.6, 75 }, { 6.74, 80 }, { 6.88, 82.5 }, { 6.99, 83 }, { 7.07, 85 }, { 7.15, 87 }, { 7.23, 88 },
    { 7.33, 91 }, { 7.46, 95 }, { 7.65, 99.5 }, { 7.86, 100 },
};
const Curve kRecoveryIndex = {   // minutes from the lowest heart rate to waking
    { 55, 53.5 }, { 100, 64.5 }, { 140, 74 }, { 175, 80.5 }, { 215, 78.5 }, { 250, 81.5 }, { 265, 94 },
    { 285, 92 }, { 310, 91.5 }, { 345, 100 },
};
const Curve kActivityBalance = {   // active calories of the last 7 days over the 60 before
    { 0.659, 97 }, { 0.772, 92 }, { 0.825, 91 }, { 0.907, 88 }, { 0.971, 85 }, { 1.046, 84 }, { 1.137, 80 },
    { 1.2, 77 }, { 1.282, 75 }, { 1.431, 71 }, { 1.65, 68 },
};
const Curve kRegularity = {   // minus the spread of the last 14 nights' midpoints, minutes
    { -36.7, 88 }, { -29.3, 90 }, { -24.8, 91 }, { -21.4, 93 }, { -14.8, 95 }, { -9, 97 },
};
// Yesterday's activity score hardly moves the contributor: its fitted curve is flat.
constexpr double kPreviousDayActivity = 84;

// Contributor weights (percent of the score) in the order above, and the penalties for a low body
// temperature or resting heart rate.
const double kWeights[9] = { 0.092, 0.102, 0.142, 0.094, 0.148, 0.096, 0.146, 0.138, 0.032 };
constexpr double kTemperaturePenalty = 0.292;
constexpr double kRestingHrPenalty = 0.101;

std::string shifted(const std::string &date, int back)
{
    const int y = std::atoi(date.substr(0, 4).c_str());
    const unsigned m = static_cast<unsigned>(std::atoi(date.substr(5, 2).c_str()));
    const unsigned d = static_cast<unsigned>(std::atoi(date.substr(8, 2).c_str()));
    return localDate((daysFromCivil(y, m, d) - back) * 86400, 0);
}

double medianOf(std::vector<double> v)
{
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}

double meanOf(const std::vector<double> &v)
{
    double s = 0;
    for (double x : v)
        s += x;
    return s / static_cast<double>(v.size());
}

double spreadOf(const std::vector<double> &v)
{
    const double m = meanOf(v);
    double s = 0;
    for (double x : v)
        s += (x - m) * (x - m);
    return std::sqrt(s / static_cast<double>(v.size()));
}

} // namespace

void estimateReadiness(Summary &summary, const AnalysisOptions &options, bool alsoWhereOuraHasOne)
{
    std::map<std::string, const Night *> nightOf;   // the longest sleep of each date
    for (const Night &n : summary.nights)
        if (n.main && !n.date.empty())
            nightOf[n.date] = &n;
    std::map<std::string, const Day *> dayOf;
    for (const Day &d : summary.days)
        dayOf[d.date] = &d;

    // Minutes after the start of the half day (12:00 to 11:59), as the fit took them.
    const auto tod = [&](int64_t utc) {
        const int64_t local = utc + options.utcOffset(utc);
        const int m = static_cast<int>(((local % 86400) + 86400) % 86400 / 60);
        return m < 720 ? m : m - 1440;
    };
    const auto lowestHr = [](const Night *n) -> std::optional<double> {
        if (n && n->restingHr)
            return n->restingHr;
        return std::nullopt;
    };

    for (Day &day : summary.days) {
        if (day.readinessScore && !alsoWhereOuraHasOne)
            continue;
        const auto today = nightOf.find(day.date);
        if (today == nightOf.end())
            continue;
        const Night &night = *today->second;
        if (!night.restingHr || night.asleepMin <= 0)
            continue;

        // Previous night: the night's sleep score, Oura's if it has one.
        double score = -1;
        if (night.ouraScore)
            score = *night.ouraScore;
        else if (const auto s = ouraScoreForNight(night, options.ageYears, summary.nights))
            score = s->score;
        else if (night.score.valid)
            score = night.score.score;
        if (score < 0)
            continue;

        // Body temperature.
        const double deviation = night.tempDeviation ? std::fabs(*night.tempDeviation)
                                                     : (day.temperatureDeviation ? std::fabs(*day.temperatureDeviation) : 0.0);

        // Resting heart rate against its 60-day median.
        std::vector<double> lows;
        for (int k = 1; k <= 60; ++k)
            if (const auto low = lowestHr(nightOf.count(shifted(day.date, k)) ? nightOf[shifted(day.date, k)] : nullptr))
                lows.push_back(*low);
        if (lows.size() < 5)
            continue;
        const double rhrDiff = *night.restingHr - medianOf(lows);

        // HRV balance and sleep balance.
        std::vector<double> hrvRecent, hrvBase, sleepRecent, midpoints;
        for (int k = 0; k < 67; ++k) {
            const auto it = nightOf.find(shifted(day.date, k));
            if (it == nightOf.end())
                continue;
            const Night &n = *it->second;
            if (k < 7) {
                if (n.avgHrv)
                    hrvRecent.push_back(*n.avgHrv);
                sleepRecent.push_back(n.asleepMin / 60.0);
            } else if (n.avgHrv) {
                hrvBase.push_back(*n.avgHrv);
            }
            if (k < 14) {
                const int s = tod(n.startUnix), e = tod(n.endUnix);
                midpoints.push_back((s + e + (e < s ? 1440 : 0)) / 2.0);
            }
        }
        if (hrvRecent.size() < 2 || hrvBase.size() < 5 || sleepRecent.size() < 3 || midpoints.size() < 3)
            continue;

        // Recovery index: how long before waking the heart rate was lowest.
        double recoveryMinutes = 0;
        {
            const TimedValue *lowest = nullptr;
            for (const TimedValue &p : night.hr)
                if (!lowest || p.value < lowest->value)
                    lowest = &p;
            if (!lowest)
                continue;
            recoveryMinutes = std::max(0.0, static_cast<double>(night.endUnix - lowest->utc) / 60.0);
        }

        // Activity balance: the last week's active calories against the 60 days before.
        std::vector<double> kcalRecent, kcalBase;
        for (int k = 1; k <= 67; ++k) {
            const auto it = dayOf.find(shifted(day.date, k));
            if (it == dayOf.end())
                continue;
            (k <= 7 ? kcalRecent : kcalBase).push_back(it->second->activeKcal);
        }
        double activityBalance = 0.9;   // the middle of the fitted range when there is no history yet
        if (kcalRecent.size() >= 3 && kcalBase.size() >= 7 && meanOf(kcalBase) > 0)
            activityBalance = meanOf(kcalRecent) / meanOf(kcalBase);

        double c[9];
        c[0] = interpolate(kActivityBalance, activityBalance);
        c[1] = interpolate(kBodyTemperature, -deviation);
        c[2] = interpolate(kHrvBalance, meanOf(hrvRecent) / meanOf(hrvBase));
        c[3] = kPreviousDayActivity;
        c[4] = interpolate(kPreviousNight, score);
        c[5] = interpolate(kRecoveryIndex, recoveryMinutes);
        c[6] = interpolate(kRestingHr, -rhrDiff);
        c[7] = interpolate(kSleepBalance, meanOf(sleepRecent));
        c[8] = interpolate(kRegularity, -spreadOf(midpoints));

        double total = 0;
        for (size_t i = 0; i < 9; ++i)
            total += kWeights[i] * c[i];
        total -= kTemperaturePenalty * std::max(0.0, 50 - c[1]);
        total -= kRestingHrPenalty * std::max(0.0, 50 - c[6]);

        day.estimatedReadinessScore = static_cast<int>(std::lround(std::max(1.0, std::min(100.0, total))));
        day.estimatedReadinessContributors.clear();
        for (double v : c)
            day.estimatedReadinessContributors.push_back(static_cast<int>(std::lround(v)));
    }
}

} // namespace Oura

#include "activity_score.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace Oura {

#ifndef ACT_SED_BIAS
#define ACT_SED_BIAS 0.0
#endif
#ifndef ACT_TRAIN_OFFSET
#define ACT_TRAIN_OFFSET 0.6
#endif

namespace {

const std::array<int, 6> kWeights = { 25, 10, 25, 15, 10, 15 };

// Days of the last seven with the goal reached -> meet daily targets.
const int kMeetTargets[8] = { 1, 25, 43, 60, 78, 95, 100, 100 };
// Days of the last seven with 150 or more medium and high MET-minutes -> training frequency.
const int kTrainingFrequency[8] = { 1, 40, 71, 96, 100, 100, 100, 100 };
// Inactivity alerts of the day -> move every hour.
const int kMoveEveryHour[6] = { 100, 95, 78, 60, 43, 25 };

constexpr double kTrainingDayMetMinutes = 150;

using Curve = std::vector<std::pair<double, double>>;

// Median score per bin of sedentary hours, from the fit.
const Curve kStayActive = {
    { 4.0, 100 }, { 5.25, 99 }, { 5.75, 98 }, { 6.25, 96 }, { 6.75, 94 }, { 7.25, 91 }, { 7.75, 86 },
    { 8.25, 83 }, { 8.75, 79 }, { 9.25, 75 }, { 9.75, 71 }, { 10.25, 67 }, { 10.75, 64 }, { 11.25, 60 },
    { 11.75, 56 }, { 12.25, 53 }, { 12.75, 49 }, { 13.25, 45 }, { 13.75, 41 }, { 14.25, 38.5 },
};

// Median score per bin of the week's medium and high MET-minutes, from the fit.
const Curve kTrainingVolume = {
    { 0, 1 },     { 200, 33 },  { 280, 40 },  { 360, 47 },  { 440, 51 },  { 520, 65 },  { 600, 69.5 },
    { 680, 76 },  { 760, 83 },  { 840, 95 },  { 1000, 96 }, { 1240, 97 }, { 1480, 98 }, { 1720, 99 },
    { 1960, 100 },
};

double interpolate(const Curve &c, double x)
{
    if (x <= c.front().first)
        return c.front().second;
    if (x >= c.back().first)
        return c.back().second;
    for (size_t i = 1; i < c.size(); ++i)
        if (x <= c[i].first) {
            const double t = (x - c[i - 1].first) / (c[i].first - c[i - 1].first);
            return c[i - 1].second + t * (c[i].second - c[i - 1].second);
        }
    return c.back().second;
}

} // namespace

ActivityDay activityDayFromMinutes(const std::vector<double> &met, double activeKcal, double targetKcal)
{
    static const double kThreshold[4] = { 1.1, 1.7, 3.7, 7.7 };
    ActivityDay day;
    day.activeKcal = activeKcal;
    day.targetKcal = targetKcal;
    int run = 0, sedentaryWindows = 0;
    for (size_t w = 0; w + 5 <= met.size(); w += 5) {
        double sum = 0;
        int n = 0;
        for (size_t i = w; i < w + 5; ++i)
            if (!std::isnan(met[i])) {
                sum += met[i];
                ++n;
            }
        // A window the ring recorded too little of is no class at all.
        int cls = 0;
        if (n >= 3) {
            const double mean = sum / n;
            cls = 1;
            for (double t : kThreshold)
                if (mean >= t)
                    ++cls;
        }
        if (cls == 2) {
            ++sedentaryWindows;
            if (++run == 20)
                ++day.inactivityAlerts;   // a hundred minutes unbroken
        } else {
            run = 0;
        }
        if (cls >= 4)
            for (size_t i = w; i < w + 5; ++i)
                if (!std::isnan(met[i]))
                    day.trainingMetMinutes += std::max(0.0, met[i] - ACT_TRAIN_OFFSET);
    }
    day.sedentaryHours = std::max(0.0, sedentaryWindows * 5 / 60.0 - ACT_SED_BIAS);
    return day;
}

const std::array<int, 6> &activityWeights()
{
    return kWeights;
}

std::optional<ActivityScore> activityScore(const std::vector<ActivityDay> &week)
{
    if (week.size() < 7)
        return std::nullopt;
    const std::vector<ActivityDay> last(week.end() - 7, week.end());
    const ActivityDay &today = last.back();

    int metGoal = 0, trained = 0;
    double volume = 0;
    for (const ActivityDay &d : last) {
        if (d.targetKcal > 0 && d.activeKcal >= d.targetKcal)
            ++metGoal;
        if (d.trainingMetMinutes >= kTrainingDayMetMinutes)
            ++trained;
        volume += d.trainingMetMinutes;
    }

    ActivityScore s;
    s.contributors[0] = kMeetTargets[metGoal];
    s.contributors[1] = kMoveEveryHour[std::min(today.inactivityAlerts, 5)];
    s.contributors[2] = 100;   // what lowers it is not known
    s.contributors[3] = static_cast<int>(std::lround(interpolate(kStayActive, today.sedentaryHours)));
    s.contributors[4] = kTrainingFrequency[trained];
    s.contributors[5] = static_cast<int>(std::lround(interpolate(kTrainingVolume, volume)));
    int sum = 0;
    for (size_t i = 0; i < 6; ++i)
        sum += kWeights[i] * s.contributors[i];
    s.score = (sum + 50) / 100;
    return s;
}

} // namespace Oura

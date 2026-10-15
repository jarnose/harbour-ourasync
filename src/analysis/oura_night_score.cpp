#include "oura_night_score.h"

#include <algorithm>
#include <cmath>

namespace Oura {

namespace {
constexpr double kNapMinutes = 15;
}

std::optional<OuraSleepScore> ouraScoreForNight(const Night &n, int ageYears, const std::vector<Night> &all)
{
    if (!n.staged || n.asleepMin <= 0 || !n.oura)
        return std::nullopt;
    const OuraNightSummary &s = *n.oura;
    int deepEpochs = s.deepEpochs, lightEpochs = s.lightEpochs, remEpochs = s.remEpochs;
    for (const Night &other : all) {
        if (&other == &n || other.main || other.date != n.date || !other.oura || other.asleepMin < kNapMinutes)
            continue;
        deepEpochs += other.oura->deepEpochs;
        lightEpochs += other.oura->lightEpochs;
        remEpochs += other.oura->remEpochs;
    }
    const int asleepEpochs = deepEpochs + lightEpochs + remEpochs;

    OuraAppInput in;
    in.ageYears = ageYears;
    in.totalMinutes = (asleepEpochs + 1) >> 1;
    in.deepMinutes = (deepEpochs + 1) >> 1;
    in.remMinutes = (remEpochs + 1) >> 1;
    // Efficiency is asleep over time in bed, rounded, over the same periods as the sleep.
    double inBedSeconds = n.inBedMin * 60.0;
    for (const Night &other : all)
        if (&other != &n && !other.main && other.date == n.date && other.oura && other.asleepMin >= kNapMinutes)
            inBedSeconds += other.inBedMin * 60.0;
    in.efficiencyPercent = inBedSeconds > 0
            ? static_cast<int>(std::lround(100.0 * asleepEpochs * 30.0 / inBedSeconds)) : 0;
    in.efficiencyPercent = std::max(0, std::min(100, in.efficiencyPercent));
    in.latencyEpochs = s.latencyEpochs;
    in.midpointMinutes = s.midpointMinutes;
    in.wakeUps = s.wakeUps;
    if (n.ouraMotionKnown)
        in.restlessEpochs = s.restlessEpochs;
    return ouraSleepScoreApp(in);
}

} // namespace Oura

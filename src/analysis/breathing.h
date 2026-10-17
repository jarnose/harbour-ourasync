#pragma once

#include <optional>
#include <vector>

// A night's breathing rate from the ring's beat-to-beat intervals, for rings that do not send the
// breathing rate themselves (Ring 4 and 5 compute it in Oura's app).
//
// Not a port of Oura's `breathing_rate_calculate_averages`, which was run in an emulator and read
// but whose resampling, filter and peak logic are tangled in fixed-point state. This takes the same
// route in floating point: the intervals are cleaned, resampled at 4 Hz, band-passed to the
// breathing band (0.15-0.45 Hz) and the peaks of each two-minute stretch counted, per 30-second
// epoch; the night's rate is the mean of the epochs of its middle (the first and last hour are left
// out of nights over five hours, as Oura does). On eight stretches of one real night it lands within
// 0.11 breaths a minute of Oura's library on average and 0.33 at worst
// (tools/cloudcheck/breathing_check.py, docs/oura-score-extraction.md).
namespace Oura {

struct BreathBeat
{
    double utcSeconds = 0;   // when the beat ended
    int ibiMs = 0;           // its length
};

// `startUtc`..`endUtc` is the night's sleep window. Needs at least three hours of it and returns
// nothing when too few usable beats remain.
std::optional<double> breathingRate(const std::vector<BreathBeat> &beats, double startUtc, double endUtc);

} // namespace Oura

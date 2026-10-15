#pragma once

#include <array>
#include <optional>

// Oura's own sleep-score mapper, as found in the native `ecore` library of the
// Android app 7.27.0 (libecore.so, `ecore_sleep_score_calculate` and the
// function it calls to map nine inputs to nine contributors). See
// docs/oura-score-extraction.md for what was read, where, and how it was
// checked.
//
// This is a *reading of a proprietary binary*, done to reproduce the arithmetic
// and nothing else: no code or table is copied from the library into this
// repository, and the numbers below are constants of a formula. Treat the result
// as an estimate that follows Oura's own method, not as Oura's score.
//
// What is exact: the weights, the response curve, the age-dependent limits and
// the combination (sum of weight x contributor, plus 50, divided by 100). What is
// not here: the three "restfulness" inputs and the timing centre, which the
// native code derives from a per-epoch motion state machine. Where the caller
// cannot supply them, those contributors drop out and the rest renormalise, and
// the result says so.
namespace Oura {

struct OuraSleepInput
{
    int ageYears = 40;

    // Counts of 30-second epochs, as the native code counts them.
    int inBedEpochs = 0;
    int deepEpochs = 0;
    int lightEpochs = 0;
    int remEpochs = 0;
    // Epochs from the start of the window to the first sustained sleep.
    int latencyEpochs = 0;

    // Midpoint of the sleep, minutes after midnight (negative before it).
    int midpointMinutes = 0;
    // The chronotype byte the engine is given, and its centre in minutes. The
    // native rule: 255 means "none": a fixed curve on the distance from 01:30; 1-6
    // are real chronotypes and centre a narrower plateau on `chronotypeCenterMinutes`;
    // anything else (0 included) is the default plateau around 03:00. Oura's
    // own app sends the last of these when the user has no chronotype yet.
    int chronotypeType = 0;
    int chronotypeCenterMinutes = 0;

    // The native restfulness inputs, which need the motion state machine.
    std::optional<int> restfulA;
    std::optional<int> restfulB;
    std::optional<int> restfulC;
};

struct OuraSleepScore
{
    // 0-100 per contributor, in native order: total sleep, deep, REM,
    // efficiency, latency, restfulness A/B/C, timing. -1 where not computed.
    std::array<int, 9> contributor{};
    int score = 0;
    // False when restfulness was left out, so the score is over fewer than the
    // full 100 weight points.
    bool complete = false;
    int weightUsed = 0;
};

// The static contributor weights, summing to 100.
const std::array<int, 9> &ouraSleepWeights();

// One contributor's response: a piecewise-linear curve through
// (limits[0..3]) -> (0, 25, 95, 100), ascending or descending, with the native
// integer rounding.
int ouraCurve(int value, const std::array<int, 4> &limits);

// The four breakpoints of a contributor for an age.
std::array<int, 4> ouraSleepLimits(int contributorIndex, int ageYears);

OuraSleepScore ouraSleepScore(const OuraSleepInput &in);

} // namespace Oura

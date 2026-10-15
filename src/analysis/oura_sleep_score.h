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

// The library has two sets of breakpoints for the deep and REM contributors. The
// numerical analysis path (what open_oura's notes and the first port covered)
// initialises the second; the scores in Oura's cloud and in its app are computed
// with the first, to the point: tools/cloudcheck/compare_sleep.py reproduces total
// sleep, deep, REM, efficiency and latency of 644 of 644 real nights exactly.
enum class OuraLimitSet { V1, V2 };

struct OuraSleepInput
{
    int ageYears = 40;
    OuraLimitSet limits = OuraLimitSet::V2;

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
    // The seven contributors as Oura's app shows them, each 1-100: total sleep,
    // deep, REM, efficiency, latency, restfulness (the three parts above, rescaled
    // and added) and timing. 0 where not computed.
    std::array<int, 7> displayed{};
    // False when restfulness was left out, so the score is over fewer than the
    // full 100 weight points.
    bool complete = false;
    int weightUsed = 0;
    // True when restfulness rests on the typical value for its third input.
    bool restfulnessEstimated = false;
};

// The static contributor weights, summing to 100.
const std::array<int, 9> &ouraSleepWeights();

// The input of the path the app takes to a night's score: minutes, as
// `ecore_sleep_score_calculate_minutes` is called, with the first set of limits.
struct OuraAppInput
{
    int ageYears = 40;

    int totalMinutes = 0;      // asleep: deep + light + REM, half-up from 30 s epochs
    int deepMinutes = 0;
    int remMinutes = 0;
    int efficiencyPercent = 0; // asleep over time in bed, rounded
    int latencyEpochs = 0;     // 30 s epochs to the first sustained sleep

    int midpointMinutes = 0;   // local, minutes after midnight, -720..719
    int chronotypeType = 255;  // 255: none known (what Oura sends until it has learnt one)
    int chronotypeCenterMinutes = 0;

    // Restfulness. Wake-ups and restless epochs come from the night's state machine; the
    // third input, "motion events", is a count the cloud data cannot reveal. When it is
    // not given a typical value stands in and the result says so.
    int wakeUps = 0;
    std::optional<int> restlessEpochs;   // unknown: restfulness is left out
    std::optional<int> motionEvents;
};

// The value used for the third restfulness input when the real one is not known: the
// median over three years of one person's nights, found by inverting the official
// restfulness (see docs/oura-score-extraction.md). An estimate, not a measurement.
constexpr int kTypicalMotionEvents = 40;

// One contributor's response: a piecewise-linear curve through
// (limits[0..3]) -> (0, 25, 95, 100), ascending or descending, with the native
// integer rounding.
int ouraCurve(int value, const std::array<int, 4> &limits);

// The four breakpoints of a contributor for an age.
std::array<int, 4> ouraSleepLimits(int contributorIndex, int ageYears, OuraLimitSet limits = OuraLimitSet::V2);

OuraSleepScore ouraSleepScoreApp(const OuraAppInput &in);

OuraSleepScore ouraSleepScore(const OuraSleepInput &in);

} // namespace Oura

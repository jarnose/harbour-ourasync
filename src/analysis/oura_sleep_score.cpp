#include "oura_sleep_score.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace Oura {

namespace {

// Response of every contributor: input at the four breakpoints scores 0, 25, 95
// and 100.
constexpr int kCurveScore[4] = { 0, 25, 95, 100 };

// Contributor weights in native order. They sum to 100.
const std::array<int, 9> kWeights = { 35, 10, 10, 10, 10, 5, 5, 5, 10 };

// Breakpoints of the contributors that do not depend on age.
const std::array<std::array<int, 4>, 6> kStaticLimits = {{
    { 30, 60, 90, 95 },     // 3: efficiency, percent
    { 60, 28, 2, 0 },       // 4: distance of the latency from 15 minutes
    { 240, 128, 16, 0 },    // 5: restfulness A
    { 80, 50, 25, 15 },     // 6: restfulness B
    { 96, 48, 16, 0 },      // 7: restfulness C
    { 390, 240, 100, 90 },  // 8: timing, when no chronotype is known
}};

int roundHalfAway(float x)
{
    return static_cast<int>(std::lround(x));
}

int clampScore(int v)
{
    return std::max(0, std::min(100, v));
}

} // namespace

const std::array<int, 9> &ouraSleepWeights()
{
    return kWeights;
}

std::array<int, 4> ouraSleepLimits(int index, int ageYears)
{
    // The native function takes the age as an unsigned byte.
    const int age = std::max(0, std::min(255, ageYears));

    if (index >= 3 && index <= 8)
        return kStaticLimits[static_cast<size_t>(index - 3)];

    // 486 - 453 * age / 256, the sleep need falling with age.
    const int need = 486 - ((453 * age) >> 8);

    if (index == 0) {
        // Total sleep in minutes: 0 -> 0, about half the need -> 25, the need ->
        // 95, and 100 at nine hours (younger than 31: a little more).
        const int full = age < 31 ? (5 * need + 2) >> 2 : 540;
        return { 0, ((need << 7) + 128) >> 8, (302 * need + 128) >> 8, full };
    }
    if (index == 1) {
        // Deep sleep target in minutes, from a single-precision fused
        // multiply-add exactly as the native code does it.
        const float line = std::fma(static_cast<float>(age), -1.1334f, 115.64f);
        const int target = std::max(roundHalfAway(line), 20);
        return { 0, roundHalfAway(static_cast<float>(target) / 4.0f), target,
                 roundHalfAway(static_cast<float>(target) * 1.25f) };
    }
    // index == 2: REM target in minutes.
    const int target = std::max(114 - ((341 * age) >> 9), 60);
    return { 0, roundHalfAway(static_cast<float>(target) / 4.0f), target,
             roundHalfAway(static_cast<float>(target) * 1.25f) };
}

int ouraCurve(int v, const std::array<int, 4> &r)
{
    int k;
    if (r[1] > r[0]) {
        // Ascending breakpoints: more is better.
        if (v <= r[0])
            return 0;
        k = v <= r[1] ? 1 : (v <= r[2] ? 2 : 3);
    } else {
        // Descending breakpoints: less is better.
        if (v >= r[0])
            return 0;
        k = v >= r[1] ? 1 : (v >= r[2] ? 2 : 3);
    }
    const int d = r[static_cast<size_t>(k)] - r[static_cast<size_t>(k - 1)];
    if (d == 0)
        return 0;
    const int num = (v - r[static_cast<size_t>(k - 1)]) * (kCurveScore[k] - kCurveScore[k - 1]);
    // The native rounding: add half the span, in the direction of the span, then
    // a truncating signed division.
    const int half = d > 0 ? (d >> 1) : (((-d) & 0xfffe) >> 1);
    return clampScore((num + half) / d + kCurveScore[k - 1]);
}

namespace {

// Sleep timing: a trapezoid around the ideal midpoint. 100 inside +-45 minutes
// of it (+-30 when the chronotype is known), 25 at 150 minutes beyond that and
// 0 at 280. Read from sleep_timing_score_calculate; see the doc for its status.
int timingScore(int midpoint, std::optional<int> chronotypeCenter)
{
    const bool known = chronotypeCenter.has_value();
    const float c = known ? static_cast<float>(*chronotypeCenter) : 180.0f;
    const float lo = known ? -30.0f : -45.0f;
    const float hi = known ? 30.0f : 45.0f;
    const float x[6] = { c + lo - 280, c + lo - 150, c + lo, c + hi, c + hi + 150, c + hi + 280 };
    const float y[6] = { 0, 25, 100, 100, 25, 0 };
    const float m = static_cast<float>(midpoint);
    if (m <= x[0] || m >= x[5])
        return 0;
    for (int i = 1; i < 6; ++i) {
        if (m <= x[i]) {
            const float t = (m - x[i - 1]) / (x[i] - x[i - 1]);
            return clampScore(roundHalfAway(y[i - 1] + t * (y[i] - y[i - 1])));
        }
    }
    return 0;
}

} // namespace

OuraSleepScore ouraSleepScore(const OuraSleepInput &in)
{
    // The native code counts epochs of 30 seconds and halves them to minutes
    // with its own +1 rounding; the same expressions are used here.
    const int w1 = std::min(in.inBedEpochs, 1799) + 1;
    const int w2 = in.deepEpochs;
    const int w3 = in.lightEpochs;
    const int w4 = in.remEpochs;

    const int total = ((w2 + 1) + w3 + w4) >> 1;
    const int denominator = w1 >> 1;

    std::array<int, 9> input{};
    input[0] = total;
    input[1] = (w2 + 1) >> 1;
    input[2] = (w4 + 1) >> 1;
    input[3] = denominator > 0 ? (total * 100 + (w1 >> 2)) / denominator : 0;
    input[4] = std::abs(((in.latencyEpochs + 1) >> 1) - 15);
    const int t = std::max(total, 60);
    const int h = t >> 1;
    if (in.restfulA && in.restfulB && in.restfulC) {
        input[5] = (*in.restfulA * 6720 + h) / t;
        input[6] = (*in.restfulB * 50 + h) / t;
        input[7] = (*in.restfulC * 6720 + h) / t;
    }

    OuraSleepScore out;
    out.complete = in.restfulA && in.restfulB && in.restfulC;
    long weighted = 0;
    int weights = 0;
    for (int i = 0; i < 9; ++i) {
        if (i >= 5 && i <= 7 && !out.complete) {
            out.contributor[static_cast<size_t>(i)] = -1;
            continue;
        }
        const int c = i == 8 ? timingScore(in.midpointMinutes, in.chronotypeCenterMinutes)
                             : ouraCurve(input[static_cast<size_t>(i)], ouraSleepLimits(i, in.ageYears));
        out.contributor[static_cast<size_t>(i)] = c;
        weighted += static_cast<long>(kWeights[static_cast<size_t>(i)]) * c;
        weights += kWeights[static_cast<size_t>(i)];
    }
    out.weightUsed = weights;
    // The native combination is (sum + 50) / 100 over weights that add to 100;
    // over fewer weights the same rounding is taken against their total.
    out.score = weights > 0 ? static_cast<int>((weighted + weights / 2) / weights) : 0;
    return out;
}

} // namespace Oura

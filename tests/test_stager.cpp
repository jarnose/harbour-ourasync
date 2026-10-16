// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/{stager,daysummary,ringclock,sleepscore,oura_sleep_state}.cpp ../src/ring/{protocol,events,aes128}.cpp test_stager.cpp -o /tmp/st && /tmp/st
#include "hex.h"
#include "analysis/stager.h"

#include <cmath>

using namespace Oura;

int main()
{
    // A synthetic seven-hour night: restless for the first ten minutes, then still; heart rate
    // dips in the first hours and rises towards morning; HRV rises late; stirring at the end.
    const double start = 1760000000, end = start + 7 * 3600;
    std::vector<TimedValue> hr, hrv, motion;
    for (int t = 0; t < 7 * 3600; t += 300) {
        const double f = t / (7.0 * 3600);
        hr.push_back({ static_cast<int64_t>(start + t), 56 - 6 * std::sin(f * 3.14159) + (f > 0.9 ? 6 : 0) });
        hrv.push_back({ static_cast<int64_t>(start + t), 35 + 25 * f });
    }
    for (int t = 0; t < 7 * 3600; t += 30) {
        double m = 0;
        if (t < 600) m = 12;                     // settling in
        if (t > 7 * 3600 - 900) m = 10;          // getting up
        motion.push_back({ static_cast<int64_t>(start + t), m });
    }
    const std::vector<uint8_t> s = stageFromSignals(start, end, hr, hrv, motion);
    CHECK_EQ(s.size(), size_t(840));
    if (s.size() != 840)
        return finish("stager");
    // Awake while settling in and at the end, asleep in between.
    CHECK_EQ(int(s[2]), 4);
    CHECK_EQ(int(s[838]), 4);
    int deep = 0, light = 0, rem = 0, wake = 0;
    for (uint8_t c : s) {
        CHECK(c >= 1 && c <= 4);
        (c == 1 ? deep : c == 2 ? light : c == 3 ? rem : wake)++;
    }
    CHECK(deep > 0);
    CHECK(rem > 0);
    CHECK(light > deep);
    CHECK(wake < 120);

    // Too little to go on: under an hour, or no heart rate.
    CHECK(stageFromSignals(start, start + 1800, hr, hrv, motion).empty());
    CHECK(stageFromSignals(start, end, {}, hrv, motion).empty());
    // Deterministic.
    CHECK(stageFromSignals(start, end, hr, hrv, motion) == s);
    return finish("stager");
}

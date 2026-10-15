// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/{oura_night_score,oura_sleep_score,oura_sleep_state,daysummary,ringclock,sleepscore}.cpp ../src/ring/{protocol,events,aes128}.cpp test_oura_night_score.cpp -o /tmp/ons && /tmp/ons
#include "hex.h"
#include "analysis/oura_night_score.h"

#include <cmath>

using namespace Oura;

static Night night(const char *date, bool main, int deep, int light, int rem, int latency, double inBedMin)
{
    Night n;
    n.date = date;
    n.main = main;
    n.staged = true;
    n.inBedMin = inBedMin;
    OuraNightSummary s;
    s.deepEpochs = deep;
    s.lightEpochs = light;
    s.remEpochs = rem;
    s.latencyEpochs = latency;
    s.midpointMinutes = 120;
    s.restlessEpochs = 250;
    s.wakeUps = 5;
    n.oura = s;
    n.asleepMin = (deep + light + rem) / 2.0;
    n.ouraMotionKnown = true;
    return n;
}

int main()
{
    const Night main = night("2026-10-01", true, 140, 520, 200, 30, 480);
    const auto alone = ouraScoreForNight(main, 44);
    CHECK(alone.has_value());
    if (!alone)
        return finish("oura_night_score");

    // By hand: 860 epochs asleep = 430 minutes, deep 70, REM 100, efficiency 430 / 480 = 90 %.
    OuraAppInput in;
    in.ageYears = 44;
    in.totalMinutes = 430;
    in.deepMinutes = 70;
    in.remMinutes = 100;
    in.efficiencyPercent = 90;
    in.latencyEpochs = 30;
    in.midpointMinutes = 120;
    in.wakeUps = 5;
    in.restlessEpochs = 250;
    const OuraSleepScore expect = ouraSleepScoreApp(in);
    for (size_t i = 0; i < 7; ++i)
        CHECK_EQ(alone->displayed[i], expect.displayed[i]);
    CHECK_EQ(alone->score, expect.score);
    CHECK(alone->restfulnessEstimated);

    // A nap of 15 minutes or more on the same day adds to the sleep; shorter, on another
    // day, or the night itself does not.
    const Night nap = night("2026-10-01", false, 10, 30, 10, 20, 70);          // 25 minutes asleep
    const Night shortNap = night("2026-10-01", false, 2, 10, 4, 5, 20);         // 8 minutes
    const Night otherDay = night("2026-10-02", false, 20, 40, 20, 5, 80);
    const std::vector<Night> all = { main, nap, shortNap, otherDay };
    const auto withNap = ouraScoreForNight(all[0], 44, all);
    CHECK(withNap.has_value());
    in.totalMinutes = 430 + 25;
    in.deepMinutes = 70 + 5;
    in.remMinutes = 100 + 5;
    in.efficiencyPercent = static_cast<int>(std::lround(100.0 * (430 + 25) / (480 + 70)));
    const OuraSleepScore expectNap = ouraSleepScoreApp(in);
    if (withNap)
        for (size_t i = 0; i < 7; ++i)
            CHECK_EQ(withNap->displayed[i], expectNap.displayed[i]);

    // No stages, no score.
    Night bare;
    bare.date = "2026-10-01";
    CHECK(!ouraScoreForNight(bare, 44).has_value());

    // Without motion the restfulness is left out.
    Night noMotion = main;
    noMotion.ouraMotionKnown = false;
    const auto partial = ouraScoreForNight(noMotion, 44);
    CHECK(partial.has_value() && !partial->complete);
    return finish("oura_night_score");
}

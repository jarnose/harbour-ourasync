// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/oura_sleep_score.cpp test_oura_sleep_score.cpp -o /tmp/oss && /tmp/oss
//
// Checks the arithmetic read out of Oura's native library against the numbers
// open_oura published for one night of its author (docs/algorithms/score-weights.md):
// 471 minutes asleep at 91 % efficiency gave Total 83 and Efficiency 96.
#include "hex.h"
#include "analysis/oura_sleep_score.h"

using namespace Oura;

static void weights()
{
    int sum = 0;
    for (int w : ouraSleepWeights())
        sum += w;
    CHECK_EQ(sum, 100);
    // Total sleep 35, the three restfulness inputs 5 each (open_oura's 15 %).
    CHECK_EQ(ouraSleepWeights()[0], 35);
    CHECK_EQ(ouraSleepWeights()[5] + ouraSleepWeights()[6] + ouraSleepWeights()[7], 15);
}

static void limits()
{
    // Total sleep for a 40-year-old: need 416, 208 / 491 / 540.
    auto a = ouraSleepLimits(0, 40);
    CHECK_EQ(a[0], 0);
    CHECK_EQ(a[1], 208);
    CHECK_EQ(a[2], 491);
    CHECK_EQ(a[3], 540);
    // Under 31 the full-score point is higher.
    CHECK(ouraSleepLimits(0, 20)[3] > 540);
    // Deep and REM targets fall with age.
    CHECK(ouraSleepLimits(1, 25)[2] > ouraSleepLimits(1, 60)[2]);
    CHECK(ouraSleepLimits(2, 25)[2] > ouraSleepLimits(2, 60)[2]);
    // Floors.
    CHECK_EQ(ouraSleepLimits(1, 120)[2], 20);
    CHECK_EQ(ouraSleepLimits(2, 120)[2], 60);
}

static void curve()
{
    const std::array<int, 4> eff = { 30, 60, 90, 95 };
    CHECK_EQ(ouraCurve(91, eff), 96);   // open_oura: efficiency 91 % -> 96
    CHECK_EQ(ouraCurve(30, eff), 0);
    CHECK_EQ(ouraCurve(60, eff), 25);
    CHECK_EQ(ouraCurve(90, eff), 95);
    CHECK_EQ(ouraCurve(95, eff), 100);
    CHECK_EQ(ouraCurve(100, eff), 100); // beyond the last point: clamped
    // Descending: latency distance, 0 minutes away from the ideal is best. The
    // native rounding adds half the span toward zero, so on a descending curve
    // the breakpoints themselves come out one point low (99, 94, 24): that is
    // what the code does, and the test keeps it that way on purpose.
    const std::array<int, 4> lat = { 60, 28, 2, 0 };
    CHECK_EQ(ouraCurve(0, lat), 99);
    CHECK_EQ(ouraCurve(2, lat), 94);
    CHECK_EQ(ouraCurve(28, lat), 24);
    CHECK_EQ(ouraCurve(60, lat), 0);
    CHECK_EQ(ouraCurve(75, lat), 0);
    CHECK(ouraCurve(1, lat) > ouraCurve(2, lat));   // monotone
}

static void openOuraNight()
{
    // 471 minutes asleep. The author's age is not published; the Total 83 of
    // open_oura is reproduced for ages 24-25, and Efficiency 96 for every age.
    OuraSleepInput in;
    in.ageYears = 24;
    in.inBedEpochs = 2 * 518;   // 91 % of 518 minutes in bed is 471 asleep
    in.deepEpochs = 2 * 80;
    in.lightEpochs = 2 * 270;
    in.remEpochs = 2 * 121 - 1;
    in.latencyEpochs = 2 * 12;
    in.midpointMinutes = 150;
    OuraSleepScore s = ouraSleepScore(in);
    CHECK_EQ(s.contributor[0], 83);
    CHECK_EQ(s.contributor[3], 96);
    CHECK_EQ(s.contributor[8], 100);   // midpoint 02:30 is inside the timing plateau
    CHECK(!s.complete);                // restfulness inputs were not supplied
    CHECK_EQ(s.contributor[5], -1);
    CHECK_EQ(s.weightUsed, 85);
    CHECK(s.score > 0 && s.score <= 100);

    // With restfulness supplied the weights add to 100.
    in.restfulA = 2;
    in.restfulB = 3;
    in.restfulC = 1;
    s = ouraSleepScore(in);
    CHECK(s.complete);
    CHECK_EQ(s.weightUsed, 100);
}

static void timing()
{
    OuraSleepInput in;
    in.inBedEpochs = 800;
    in.deepEpochs = in.lightEpochs = in.remEpochs = 200;
    in.ageYears = 40;
    auto at = [&](int midpoint) {
        in.midpointMinutes = midpoint;
        return ouraSleepScore(in).contributor[8];
    };
    CHECK_EQ(at(180), 100);        // 03:00, the default centre
    CHECK_EQ(at(180 + 45), 100);   // the edge of the plateau
    CHECK_EQ(at(180 - 45), 100);
    CHECK_EQ(at(180 + 45 + 150), 25);
    CHECK_EQ(at(180 + 45 + 280), 0);
    CHECK_EQ(at(-720), 0);         // noon: nowhere near
    // A known chronotype narrows the plateau to +-30 around its own centre.
    in.chronotypeType = 3;
    in.chronotypeCenterMinutes = 120;
    in.midpointMinutes = 120 + 30;
    CHECK_EQ(ouraSleepScore(in).contributor[8], 100);
    in.midpointMinutes = 120 + 40;
    CHECK(ouraSleepScore(in).contributor[8] < 100);
}

// The path the app takes to a night's score, against vectors read from Oura's library
// (init_limits + calculate_minutes in the emulator, tools/apk/diff_sleep_app.py); the
// fuzz comparison there covers 12,000 more.
static void appPath()
{
    struct Case
    {
        int age, total, deep, rem, eff, lat, wake, restless, events, mid, type, centre;
        int expect[7];
    };
    const Case cases[] = {
        { 44, 409, 68, 112, 77, 45, 6, 279, 40, 0, 255, 0, { 77, 88, 96, 65, 78, 69, 99 } },
        { 44, 444, 75, 120, 95, 30, 5, 200, 30, -60, 255, 0, { 85, 95, 97, 100, 99, 85, 69 } },
        { 43, 300, 40, 60, 88, 2, 9, 300, 40, 200, 3, 164, { 49, 50, 55, 90, 62, 54, 97 } },
        { 30, 520, 100, 150, 99, 28, 1, 50, 10, 100, 255, 0, { 97, 96, 100, 100, 97, 98, 100 } },
        { 44, 190, 10, 20, 60, 255, 12, 400, 60, 300, 255, 0, { 23, 13, 18, 25, 1, 38, 39 } },
    };
    for (const Case &c : cases) {
        OuraAppInput in;
        in.ageYears = c.age;
        in.totalMinutes = c.total;
        in.deepMinutes = c.deep;
        in.remMinutes = c.rem;
        in.efficiencyPercent = c.eff;
        in.latencyEpochs = c.lat;
        in.wakeUps = c.wake;
        in.restlessEpochs = c.restless;
        in.motionEvents = c.events;
        in.midpointMinutes = c.mid;
        in.chronotypeType = c.type;
        in.chronotypeCenterMinutes = c.centre;
        const OuraSleepScore s = ouraSleepScoreApp(in);
        for (size_t i = 0; i < 7; ++i)
            CHECK_EQ(s.displayed[i], c.expect[i]);
        CHECK(!s.restfulnessEstimated);
    }
    // Without the third restfulness input a typical value stands in, and says so.
    OuraAppInput in;
    in.ageYears = 44;
    in.totalMinutes = 409;
    in.restlessEpochs = 279;
    in.wakeUps = 6;
    const OuraSleepScore est = ouraSleepScoreApp(in);
    CHECK(est.restfulnessEstimated);
    in.motionEvents = kTypicalMotionEvents;
    CHECK_EQ(ouraSleepScoreApp(in).displayed[5], est.displayed[5]);
    // Without the restless count there is no restfulness at all.
    in.restlessEpochs.reset();
    CHECK(!ouraSleepScoreApp(in).complete);
    // The first set of limits for deep and REM: 150 minutes at the top.
    CHECK_EQ(ouraSleepLimits(1, 44, OuraLimitSet::V1)[3], 150);
    CHECK_EQ(ouraSleepLimits(1, 44, OuraLimitSet::V1)[2], 73);
    CHECK_EQ(ouraSleepLimits(2, 44, OuraLimitSet::V1)[2], 101);
    CHECK_EQ(ouraSleepLimits(1, 44, OuraLimitSet::V2)[3], 83);
}

int main()
{
    appPath();
    weights();
    limits();
    curve();
    openOuraNight();
    timing();
    return finish("oura_sleep_score");
}

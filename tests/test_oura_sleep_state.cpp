// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/oura_sleep_state.cpp test_oura_sleep_state.cpp -o /tmp/oss2 && /tmp/oss2
//
// Only quietNight's numbers were read from Oura's library in the emulator
// (tools/apk/ecore_sleep.py). The other cases pin the port's behaviour on small
// hand-made nights; the port agrees with the library on 750 random nights in
// tools/apk/diff_sleep_state.py, which is the real evidence.
#include "hex.h"
#include "analysis/oura_sleep_state.h"

using namespace Oura;

static std::vector<uint8_t> runs(std::initializer_list<std::pair<int, int>> list)
{
    std::vector<uint8_t> out;
    for (const auto &p : list)
        out.insert(out.end(), static_cast<size_t>(p.second), static_cast<uint8_t>(p.first));
    return out;
}

static void quietNight()
{
    // 20 awake, 200 light, 120 deep, 100 REM, 100 light, 10 awake; no motion.
    OuraNightEpochs n;
    n.stages = runs({ { 4, 20 }, { 2, 200 }, { 1, 120 }, { 3, 100 }, { 2, 100 }, { 4, 10 } });
    n.epochs = static_cast<int>(n.stages.size());
    n.motion.assign(n.stages.size(), 0);
    n.startMinutesOfDay = 533; // 08:53, the library's own sample start
    const OuraNightSummary s = summariseNight(n);
    CHECK_EQ(s.inBedEpochs, 550);
    CHECK_EQ(s.deepEpochs, 120);
    CHECK_EQ(s.lightEpochs, 300);
    CHECK_EQ(s.remEpochs, 100);
    CHECK_EQ(s.latencyEpochs, 20);   // the first ten-epoch run of sleep starts at epoch 20
    CHECK_EQ(s.wakeUps, 0);
    CHECK_EQ(s.restlessEpochs, 0);
    CHECK_EQ(s.motionEvents, 0);
    CHECK_EQ(s.midpointMinutes, 674);
}

static void motionCounts()
{
    // A single motion value anywhere makes four windowed epochs restless.
    OuraNightEpochs n;
    n.stages = runs({ { 4, 20 }, { 2, 200 }, { 1, 120 }, { 3, 100 }, { 2, 100 }, { 4, 10 } });
    n.epochs = static_cast<int>(n.stages.size());
    n.motion.assign(n.stages.size(), 0);
    n.startMinutesOfDay = 533;
    n.motion[300] = 12;
    CHECK_EQ(summariseNight(n).restlessEpochs, 4);
    n.motion[300] = 0;
    for (int i = 300; i < 306; ++i)
        n.motion[static_cast<size_t>(i)] = 10;
    CHECK_EQ(summariseNight(n).restlessEpochs, 9);       // six epochs and the three after
    // Motion while awake does not count, but its window spills into sleep.
    n.motion.assign(n.stages.size(), 0);
    for (int i = 0; i < 20; ++i)
        n.motion[static_cast<size_t>(i)] = 25;
    CHECK_EQ(summariseNight(n).restlessEpochs, 3);
    // Too few motion values: the library ignores motion altogether.
    n.motion.assign(n.stages.size() - 1, 9);
    CHECK_EQ(summariseNight(n).restlessEpochs, 0);
    CHECK_EQ(summariseNight(n).motionEvents, 0);
}

static void wakeUps()
{
    // A 400-epoch light night with short awakenings.
    OuraNightEpochs n;
    n.stages.assign(400, 2);
    n.epochs = 400;
    n.startMinutesOfDay = 0;
    n.stages[200] = n.stages[201] = n.stages[202] = 4;           // three awake epochs: one wake-up
    CHECK_EQ(summariseNight(n).wakeUps, 1);
    n.stages[100] = n.stages[101] = n.stages[102] = n.stages[103] = 4;
    n.stages[300] = n.stages[301] = n.stages[302] = n.stages[303] = 4;
    CHECK_EQ(summariseNight(n).wakeUps, 3);
    // A single awake epoch is not a wake-up.
    OuraNightEpochs m;
    m.stages.assign(400, 2);
    m.epochs = 400;
    m.stages[150] = 4;
    CHECK_EQ(summariseNight(m).wakeUps, 0);
    // Waking at the very end is the end of the night, not a wake-up in it.
    OuraNightEpochs e;
    e.stages.assign(400, 2);
    e.epochs = 400;
    for (size_t i = 380; i < 400; ++i)
        e.stages[i] = 4;
    CHECK_EQ(summariseNight(e).wakeUps, 0);
}

static void latency()
{
    OuraNightEpochs n;
    n.epochs = 400;
    n.stages = runs({ { 4, 40 }, { 2, 360 } });
    CHECK_EQ(summariseNight(n).latencyEpochs, 40);
    // Never settling within 132 minutes (264 epochs): the sentinel.
    n.stages = runs({ { 4, 300 }, { 2, 100 } });
    CHECK_EQ(summariseNight(n).latencyEpochs, 255);
    // Asleep from the first epoch: 1, not 0, since 0 means "not found".
    n.stages.assign(400, 2);
    CHECK_EQ(summariseNight(n).latencyEpochs, 1);
}

static void limits()
{
    // The engine reads one epoch past the end (the record that was never written
    // reads as awake) and caps a night at 1799 epochs.
    OuraNightEpochs n;
    n.epochs = 2000;
    n.stages.assign(2000, 2);
    const OuraNightSummary s = summariseNight(n);
    CHECK_EQ(s.inBedEpochs, 1799);
    CHECK_EQ(s.lightEpochs, 1800);
    // Fewer stages than epochs: the rest are awake.
    OuraNightEpochs m;
    m.epochs = 100;
    m.stages.assign(60, 2);
    CHECK_EQ(summariseNight(m).lightEpochs, 60);
    // Nothing at all.
    OuraNightEpochs z;
    const OuraNightSummary zs = summariseNight(z);
    CHECK_EQ(zs.deepEpochs + zs.lightEpochs + zs.remEpochs, 0);
}

int main()
{
    quietNight();
    motionCounts();
    wakeUps();
    latency();
    limits();
    return finish("oura_sleep_state");
}

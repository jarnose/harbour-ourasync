#include "oura_sleep_state.h"

#include <algorithm>

namespace Oura {

namespace {

constexpr int kMaxEpochs = 0x708;      // the engine's record array
constexpr int kMaxCount = 0x707;

int signedByte(int v)
{
    v &= 0xff;
    return v >= 128 ? v - 256 : v;
}

} // namespace

OuraNightSummary summariseNight(const OuraNightEpochs &night)
{
    const int epochs = std::min(std::max(night.epochs, 0), kMaxEpochs);
    const int n = std::min(epochs, kMaxCount);

    // Motion is windowed: each epoch carries the sum of itself and the three
    // before it (a byte, so it wraps at 256).
    std::vector<int> windowed(static_cast<size_t>(n) + 1, 0);
    if (static_cast<int>(night.motion.size()) >= epochs) {
        const size_t have = std::min<size_t>(night.motion.size(), kMaxEpochs);
        for (size_t k = 0; k < windowed.size() && k < have; ++k) {
            int sum = 0;
            for (size_t j = k >= 3 ? k - 3 : 0; j <= k; ++j)
                sum += night.motion[j];
            windowed[k] = sum & 0xff;
        }
    }

    int light = 0, deep = 0, rem = 0;
    int sleepStreak = 0;          // signed: negative while awake
    bool asleep = false;          // sleeping for at least ~10 minutes (or since the start)
    int quiet = 0;                // consecutive non-awake epochs, saturating at 0xff
    int restless = 0;
    int events = 0;
    bool armed = true;            // motion detector ready for a new jump
    int calm = 0;                 // consecutive epochs without a jump
    long indexSum = 0;
    int wakes = 0;
    bool sleptSinceWake = false;
    int latency = 0;
    int motionNow = 0, motionPrev1 = 0, motionPrev2 = 0;  // the windowed value at i, i-1, i-2

    for (int idx = 0; idx <= n; ++idx) {
        const int code = idx < static_cast<int>(night.stages.size()) ? night.stages[static_cast<size_t>(idx)] : 0;
        const int stage = code == 0 ? 4 : code;
        // Motion history: i-3 and i-2 decide whether a jump is a jump.
        const int motionPrev3 = motionPrev2;
        motionPrev2 = motionPrev1;
        motionPrev1 = motionNow;
        motionNow = windowed[static_cast<size_t>(idx)];
        if (stage < 1 || stage > 4)
            continue;

        int nextQuiet;
        const bool awake = stage == 4;
        if (awake) {
            const int p = signedByte(sleepStreak);
            sleepStreak = std::min(p, 0) - (p > -127 ? 1 : 0);
            if (asleep && sleepStreak <= -2) {
                ++wakes;
                asleep = false;
                sleptSinceWake = false;
            }
            nextQuiet = 0;
        } else {
            if (stage == 2) ++light;
            else if (stage == 1) ++deep;
            else ++rem;
            const int p = signedByte(sleepStreak);
            const int low = sleepStreak & 0xff;
            sleepStreak = low == 0x7f ? low : std::max(p, 0) + 1;
            if (!asleep) {
                if (std::min(idx + 1, 20) <= (signedByte(sleepStreak) & 0xffff)) {
                    asleep = true;
                    sleptSinceWake = true;
                }
            }
            if (quiet == 0xff) {
                nextQuiet = 0xff;
            } else {
                nextQuiet = quiet + 1;
                // Ten sleeping epochs in a row: the run began at idx - 9.
                if (quiet >= 9 && (latency & 0xff) == 0) {
                    if (idx < 10) latency = 1;
                    else if (idx <= 0x108) latency = idx + 1 - 10;
                    else latency = 0xff;
                }
            }
        }

        // Jumps in motion: the current windowed value against the larger of the
        // values two and three epochs back.
        const int reference = std::max(motionPrev3 & 0xff, motionPrev2 & 0xff);
        const int rise = std::max(motionNow - reference, 0);
        if (armed && reference >= 24 && rise >= 24) {
            ++events;
            armed = false;
            calm = 0;
        } else {
            if (rise >= 24)
                calm = 0;
            else if (calm != 0xff)
                ++calm;
            if (!armed)
                armed = std::min(idx + 1, 20) <= (calm & 0xff);
        }
        if (motionNow != 0 && stage != 4)
            ++restless;
        if (!awake)
            indexSum += idx + 1;
        quiet = nextQuiet;
    }

    // The last awakening is the end of the night, not a wake-up in it.
    if (wakes && !sleptSinceWake)
        --wakes;

    const int total = deep + light + rem;
    const long mean = total ? (indexSum + total / 2) / total : 0;

    OuraNightSummary s;
    s.inBedEpochs = n;
    s.deepEpochs = deep;
    s.lightEpochs = light;
    s.remEpochs = rem;
    s.latencyEpochs = latency;
    s.wakeUps = wakes & 0xff;
    s.restlessEpochs = restless & 0xffff;
    s.motionEvents = events & 0xff;
    s.midpointMinutes = night.startMinutesOfDay + static_cast<int>((mean + 1) >> 1);
    return s;
}

} // namespace Oura

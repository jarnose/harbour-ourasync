#pragma once

#include <cstdint>
#include <vector>

// The part of Oura's sleep analysis that turns a night's per-epoch data into the
// counts the sleep score needs: how long falling asleep took, how many times the
// sleeper woke, how restless the sleep was, and where its middle lies.
//
// It is the state machine inside the `ecore` library of the Android app 7.27.0,
// read from the library and then checked against it: tools/apk/diff_sleep_state.py
// runs the library itself in an emulator on random nights and compares. See
// docs/oura-score-extraction.md. What it consumes is what the app gives the
// engine: stage codes and motion seconds for every 30-second epoch.
namespace Oura {

struct OuraNightEpochs
{
    // One code per 30-second epoch from the start of the window: 1 deep, 2 light,
    // 3 REM, 4 awake. 0 reads as awake. Epochs beyond the end read as awake.
    std::vector<uint8_t> stages;
    // Seconds of motion in each epoch (0-30, from the ring's motion events). Empty
    // when unknown. The library ignores motion entirely when there are fewer
    // values than epochs, and so does this.
    std::vector<uint8_t> motion;
    // Number of epochs in the window: end minus start, in 30-second units.
    int epochs = 0;
    // Start of the window as minutes after local midnight, wrapped to -720..+719.
    int startMinutesOfDay = 0;
};

struct OuraNightSummary
{
    int inBedEpochs = 0;     // epochs the engine counts (at most 1799)
    int deepEpochs = 0;
    int lightEpochs = 0;
    int remEpochs = 0;
    int latencyEpochs = 0;   // start of the first run of ten sleeping epochs; 255 = none within 132 minutes
    int wakeUps = 0;         // awake runs of a minute or more during sleep, not counting the last
    int restlessEpochs = 0;  // sleeping epochs with motion in them or in the three before
    int motionEvents = 0;    // sudden jumps in motion
    int midpointMinutes = 0; // middle of the sleep, minutes after local midnight
};

OuraNightSummary summariseNight(const OuraNightEpochs &night);

} // namespace Oura

#pragma once

#include "daysummary.h"

#include <vector>

// Sleep staging from the ring's own night-time streams, for rings that do not stage sleep
// themselves (Ring 4 and 5 send no hypnogram; Oura's app runs a neural network instead).
//
// A port of open_health's model-free stager (stage_night_from_signals, refine_deep_stages_with_hrv
// and smooth_stages; MIT), by way of OpenRing's Java port. It is a heuristic: wake and sleep
// follow the movement, deep sleep is the quietest low-heart-rate stretches early in the night,
// REM the later light stretches with higher HRV. The result is an estimate and is shown as one.
namespace Oura {

// One code per 30 s epoch from `start`: 1 deep, 2 light, 3 REM, 4 awake, 0 unknown. Empty when
// there is too little to go on (under an hour, or fewer than 12 heart-rate samples).
// `hr` and `hrv` are five-minute means, `motion` the seconds of movement per ring window.
std::vector<uint8_t> stageFromSignals(double start, double end, std::vector<TimedValue> hr,
                                      std::vector<TimedValue> hrv, std::vector<TimedValue> motion);

} // namespace Oura

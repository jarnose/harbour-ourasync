#pragma once

#include "daysummary.h"
#include "oura_sleep_score.h"

#include <optional>
#include <vector>

namespace Oura {

// Oura's sleep score for a night, computed from what the app holds about it by the path
// Oura's own app takes (minutes in, the first set of limits). Empty for a night without
// stages or without the engine's counts.
//
// Total sleep, deep, REM, efficiency and latency reproduce the cloud's numbers for every
// one of 644 real nights; timing for about 97 %; restfulness lacks one input that only
// the ring's own motion data can give (docs/oura-score-extraction.md).
//
// The score of a day counts the sleep of its other periods too: a nap of at least 15 minutes
// of sleep that the night's day holds adds to total sleep, deep and REM (found by comparing
// with 871 real days: 863 match exactly with this rule, 732 without the threshold). `all` is
// the list the night belongs to; pass an empty one to score the night alone.
std::optional<OuraSleepScore> ouraScoreForNight(const Night &n, int ageYears, const std::vector<Night> &all = {});

} // namespace Oura

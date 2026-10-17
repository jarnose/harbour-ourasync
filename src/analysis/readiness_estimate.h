#pragma once

#include "daysummary.h"

// A readiness score estimated for days Oura's cloud has none for, from the nights and days the app
// holds. Nothing here is Oura's own arithmetic: each contributor is a curve fitted to 859 real days
// from one person's cloud import (tools/cloudcheck/readiness_fit.py, docs/oura-score-extraction.md)
// on the quantity that fit best, and the score a weighted sum of them with a penalty for a low body
// temperature or resting heart rate. It is within 2 points of Oura's on about 6 days in 10 and within
// 5 on 9 in 10, for the person it was fitted to; for anyone else it is a rougher guide. Shown as an
// estimate.
namespace Oura {

// Sets Day::estimatedReadinessScore (and the contributors, in Oura's order: activity balance, body
// temperature, HRV balance, previous day activity, previous night, recovery index, resting heart
// rate, sleep balance, sleep regularity) on every day that has a night of its own and no readiness
// score from Oura (or on all of them with `alsoWhereOuraHasOne`, to check the estimate against it).
// Call after the cloud has been merged in, so that its history counts.
void estimateReadiness(Summary &summary, const AnalysisOptions &options, bool alsoWhereOuraHasOne = false);

} // namespace Oura

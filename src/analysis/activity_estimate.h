#pragma once

#include "daysummary.h"

// Oura's daily activity goal, and the activity score estimated with it.
//
// Oura does not keep a fixed goal. The person picks a basic one (450 kcal of active calories here),
// and each day's is that plus a step that follows the day's readiness score. Over 872 real days
// (tools/cloudcheck/target_fit.py, docs/oura-score-extraction.md) the goal Oura set was the basic one
// plus these steps on 92.7 % of the days, and the rest were a step lower (a higher readiness did not
// always raise it; why is not found):
//
//   readiness   94 and over  88-93   85-87   70-84   62-69   48-61   40-47   under 40
//   step (kcal)    +200      +150    +100       0     -150    -200    -250     -300
//
// A low readiness is how sickness shows, so the goal falling to 150 on those days is the same rule.
namespace Oura {

// The goal for a day with the given readiness score, from the basic goal (never under 100).
int activityTargetKcal(int baseKcal, int readinessScore);

// Sets Day::estimatedActivityScore (and its contributors) on each day with enough movement and the six
// days before it. Each day is measured against its own goal: Oura's from the cloud where there is one,
// else the basic goal stepped by the day's readiness (Oura's or the estimate), else the basic goal.
// Call after estimateReadiness.
void estimateActivity(Summary &summary, const AnalysisOptions &options);

} // namespace Oura

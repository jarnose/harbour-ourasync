#pragma once

#include "daysummary.h"

#include <optional>
#include <vector>

// Sleep debt: how much sleep the last two weeks fell short of the person's need, newest nights
// counting most.
//
// The arithmetic is Oura's, from the app's library (`sleep_debt_calculate`, as ported by open_oura):
// the shortfall of each of the last 14 days (need minus the day's total sleep, naps included), weighted
// linearly from 1.0 for the newest day to 0.25 for the oldest, summed, held between 0 and 10 hours and
// rounded to 45 minutes; it needs five days with sleep. What the library is given as the need is
// worked out elsewhere in Oura's app from "typical sleep over the past 90 days, with unusually short or
// long nights filtered out", and how is not known: here it is the mean of the 90 days before, after
// dropping the days beyond 1.5 interquartile ranges from the middle half. The debt is therefore an
// estimate, and is labelled as one.
namespace Oura {

struct SleepDebt
{
    int debtMin = 0;             // 0 to 585, a multiple of 45 (10 hours held, then rounded)
    int recentShortfallMin = 0;  // the newest day's shortfall (negative when it slept more than the need)
};

// `sleepMin` and `needMin` hold the last 14 days, index 0 the newest; a day without sleep is 0. Nothing
// unless the newest day has sleep and at least five of the days do.
std::optional<SleepDebt> sleepDebt(const std::vector<double> &sleepMin, const std::vector<double> &needMin);

// The need from the daily totals of up to the 90 days before (minutes, days without sleep left out):
// nothing under 14 days.
std::optional<double> sleepNeedMinutes(const std::vector<double> &previousDaysMin);

// Sets Day::estimatedSleepDebtMin and estimatedSleepNeedMin on each day that has sleep. Call after
// the cloud has been merged in, so that its history counts.
void estimateSleepDebt(Summary &summary);

} // namespace Oura

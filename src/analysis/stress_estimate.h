#pragma once

#include <optional>
#include <vector>

// An estimate of Oura's "high stress" time for a day, from the day's heart rate.
//
// Oura counts five-minute slots, awake and still, in which it finds the person stressed (its own
// model works on the HRV of those slots). The ring does not send that HRV for the day, so this is
// built from what can be had: the slots' mean heart rate against the person's own median over the
// 14 days before, counted at several margins, and the night's HRV against its last 30, combined
// linearly. Fitted to 868 real days (tools/cloudcheck/stress_fit.py, docs/oura-score-extraction.md):
// out of sample the mean error is 6.2 slots (31 minutes) and the correlation 0.79. It is an estimate
// for one person and is labelled as one.
namespace Oura {

struct StressInput
{
    std::vector<double> today;      // mean bpm of each awake, still five-minute slot of the day
    std::vector<double> history;    // the same for the 14 days before, pooled
    std::optional<double> nightHrv; // this morning's night, ms
    std::vector<double> priorHrv;   // the nights of the 30 days before
};

// Minutes of high stress (a multiple of five), or nothing when there is too little to go on: the
// 14 days before must hold 200 slots and the day one.
std::optional<int> estimateStressMinutes(const StressInput &in);

} // namespace Oura

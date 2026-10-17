#pragma once

#include <array>
#include <optional>
#include <vector>

// An activity score in the manner of Oura's, from a day and the six before it.
//
// Not a reading of Oura's code (the library keeps a history in global state) but rules found by
// fitting 878 real days from Oura's cloud (tools/cloudcheck/activity_fit.py, and
// docs/oura-score-extraction.md): three contributors are tables of day counts and match exactly,
// two are curves with some noise, and recovery time is taken as full. The result is an estimate
// and is labelled as one.
namespace Oura {

struct ActivityDay
{
    double activeKcal = 0;
    double targetKcal = 0;          // the person's daily goal
    double sedentaryHours = 0;
    int inactivityAlerts = 0;
    double trainingMetMinutes = 0;  // medium and high activity together
};

struct ActivityScore
{
    int score = 0;
    // In Oura's order: meet daily targets, move every hour, recovery time, stay active,
    // training frequency, training volume.
    std::array<int, 6> contributors{};
};

// The day's inputs from a ring's MET series: one value per minute of the local day (1440, NaN
// where the ring recorded none). The minutes are taken in windows of five and each window given the
// class Oura's own activity classes follow (found by fitting them to the cloud's MET series: rest,
// inactive, low, medium and high at window means of 1.1, 1.7, 3.7 and 7.7), so
// the sedentary time, the training MET-minutes and the inactivity alerts come out as the cloud
// reports them to within the noise of that fit. `activeKcal` and `targetKcal` are the caller's.
ActivityDay activityDayFromMinutes(const std::vector<double> &metPerMinute, double activeKcal, double targetKcal);

// `week` holds the day and the six before it, oldest first; fewer than seven days give nothing.
std::optional<ActivityScore> activityScore(const std::vector<ActivityDay> &week);

// The contributors' weights, in the same order; they add to 100.
const std::array<int, 6> &activityWeights();

} // namespace Oura

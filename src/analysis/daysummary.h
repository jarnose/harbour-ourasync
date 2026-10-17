#pragma once

#include "activity_score.h"
#include "oura_sleep_state.h"
#include "ringclock.h"
#include "sleepscore.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Turns the stored history events into the things a person wants to look at:
// nights of sleep, heart rate through the day, activity.
//
// What the ring does and does not compute matters for what can be shown here.
// The ring itself stages sleep, finds the sleep window, averages HRV and heart
// rate, reads skin temperature and bins movement into MET levels. It does NOT
// produce the 0-100 scores, step counts or readiness: Oura computes those on the
// phone, in closed code and encrypted models that are not reproduced here. So
// every number below is either a ring measurement or an estimate derived from
// one, and the estimates are labelled as such in the interface.
namespace Oura {

struct TimedValue
{
    int64_t utc = 0;
    double value = 0;
};

struct AnalysisOptions
{
    // Seconds east of UTC at a given instant. Local midnight splits the days.
    std::function<int(int64_t)> utcOffset = [](int64_t) { return 0; };
    double ageYears = 40;
    double weightKg = 75;
    // The person's daily goal for active calories; Oura adapts it, the app takes it from Settings.
    double activeKcalGoal = 550;
};

enum StageCode : uint8_t { StageUnknown = 0, StageDeep = 1, StageLight = 2, StageRem = 3, StageAwake = 4 };

struct Night
{
    int64_t startUnix = 0;
    int64_t endUnix = 0;
    std::string date;          // local calendar date of waking, YYYY-MM-DD
    bool main = true;          // the night's longest sleep; others are naps
    bool approximateClock = false;

    double inBedMin = 0;
    double asleepMin = 0;
    bool asleepEstimated = false; // not staged: derived from the per-window records
    bool staged = false;
    // The stages were estimated from heart rate, HRV and movement (the ring did not stage the
    // night itself), not read from the ring.
    bool stagesEstimated = false;

    std::optional<double> efficiencyPct;
    std::optional<double> latencyMin;
    std::optional<double> wasoMin;
    std::optional<int> awakenings;
    double deepMin = 0, lightMin = 0, remMin = 0, awakeMin = 0;

    // Hypnogram: one stage code per epoch, starting at hypnogramStart.
    int64_t hypnogramStart = 0;
    int epochSeconds = 30;
    std::vector<uint8_t> hypnogram;

    std::vector<TimedValue> hr;      // bpm
    std::vector<TimedValue> hrv;     // RMSSD ms
    std::vector<TimedValue> breath;  // breaths per minute
    std::vector<TimedValue> temp;    // deg C
    std::vector<TimedValue> spo2;    // percent

    std::optional<double> restingHr;
    std::optional<double> avgHrv;
    std::optional<double> avgBreath;
    std::optional<double> avgSpo2;
    std::optional<double> skinTemp;
    std::optional<double> tempDeviation;

    SleepScore score;

    // Oura's own counts for the night (how long falling asleep took, wake-ups,
    // restlessness, midpoint), from the engine's state machine. The restful
    // counts mean something only when the night's motion is known.
    std::optional<OuraNightSummary> oura;
    bool ouraMotionKnown = false;

    // From Oura's cloud, when an export or API import is present. The official
    // score is Oura's own; `score` above is this app's estimate.
    bool fromCloud = false;
    std::optional<int> ouraScore;
    std::vector<std::pair<std::string, int>> ouraContributors;
};

struct HourStat
{
    int count = 0;
    double min = 0, max = 0, mean = 0;
};

struct Day
{
    std::string date;
    std::vector<HourStat> hr;          // 24 local hours, count 0 where nothing was measured
    std::vector<double> metByHour;     // mean MET per local hour, 0 where nothing was measured
    double activeKcal = 0;             // estimate: sum of (MET - 1) * kg / 60 per minute
    int lightMin = 0;                  // 1.5 <= MET < 3
    int moderateMin = 0;               // 3 <= MET < 6
    int vigorousMin = 0;               // MET >= 6
    int sedentaryMin = 0;              // MET < 1.5, awake-ish minutes the ring recorded
    int coveredMin = 0;                // minutes the ring has any MET for
    std::optional<double> avgMet;

    // An activity score worked out from the ring's MET series (an estimate: see activity_score.h).
    // Needs the day and the six before it, each with a day's worth of data.
    std::optional<int> estimatedActivityScore;
    std::vector<int> estimatedActivityContributors;   // Oura's order

    // From Oura's cloud.
    bool fromCloud = false;
    std::optional<int> steps;
    std::optional<int> totalKcal;
    std::optional<int> ouraActivityScore;
    std::vector<std::pair<std::string, int>> activityContributors;
    std::optional<int> readinessScore;
    std::vector<std::pair<std::string, int>> readinessContributors;
    std::optional<double> temperatureDeviation;
    std::optional<int> stressHighMin;      // minutes of high stress
    std::optional<int> recoveryHighMin;    // minutes of high recovery
    std::string stressSummary;             // restful / normal / stressful ...
};

struct Summary
{
    std::vector<Night> nights;         // oldest first
    std::vector<Day> days;             // oldest first
    std::optional<double> latestHr;
    int64_t latestHrUnix = 0;
    std::optional<int> ringBatteryPercent;
    int64_t ringBatteryUnix = 0;
    int unplacedEvents = 0;            // events whose epoch had no usable clock
};

Summary summarize(const std::vector<StoredEvent> &events, const AnalysisOptions &options);

// The 5th-95th percentile band and mean of one hour's samples (sorted in place).
HourStat hourStatOf(std::vector<double> &values);

// "YYYY-MM-DD" for a unix time at a fixed offset.
std::string localDate(int64_t utc, int offsetSeconds);
int64_t daysFromCivil(int64_t y, unsigned m, unsigned d);

// ecore's nightly_temperature: a 7-sample running median, then 30-sample
// windows each contributing its maximum when its range is under 2.5 deg C; the
// night's value is the lowest of those maxima, and needs at least four windows.
std::optional<double> nightlyTemperature(const std::vector<double> &celsius);

} // namespace Oura

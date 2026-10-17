// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/activity_score.cpp test_activity_score.cpp -o /tmp/act && /tmp/act
#include "hex.h"
#include "analysis/activity_score.h"

#include <cmath>

using namespace Oura;

static ActivityDay day(double kcal, double goal, double sedentaryHours, int alerts, double training)
{
    ActivityDay d;
    d.activeKcal = kcal;
    d.targetKcal = goal;
    d.sedentaryHours = sedentaryHours;
    d.inactivityAlerts = alerts;
    d.trainingMetMinutes = training;
    return d;
}

int main()
{
    // The weights add to 100.
    int sum = 0;
    for (int w : activityWeights())
        sum += w;
    CHECK_EQ(sum, 100);

    // A week of reaching the goal and training: everything full but a little sitting.
    std::vector<ActivityDay> good(7, day(600, 550, 6.25, 0, 250));
    auto s = activityScore(good);
    CHECK(s.has_value());
    if (s) {
        CHECK_EQ(s->contributors[0], 100);   // goal met all seven days
        CHECK_EQ(s->contributors[1], 100);   // no inactivity alerts
        CHECK_EQ(s->contributors[2], 100);
        CHECK_EQ(s->contributors[3], 96);    // 6.25 h sitting
        CHECK_EQ(s->contributors[4], 100);   // at least four training days
        CHECK_EQ(s->contributors[5], 99);    // a lot of training: 1750 MET-minutes in the week
        CHECK_EQ(s->score, 99);
    }

    // A still week: no goal, no training, long sitting, many alerts.
    std::vector<ActivityDay> poor(7, day(100, 550, 12.25, 5, 10));
    s = activityScore(poor);
    CHECK(s.has_value());
    if (s) {
        CHECK_EQ(s->contributors[0], 1);
        CHECK_EQ(s->contributors[1], 25);
        CHECK_EQ(s->contributors[4], 1);
        CHECK_EQ(s->contributors[3], 53);
        CHECK(s->contributors[5] < 20);
        CHECK_EQ(s->score, 38);   // recovery time, taken as full, still carries a quarter
    }

    // The counts are of the last seven days: two training days of seven give 71, three give 96.
    std::vector<ActivityDay> mixed(7, day(300, 550, 8, 1, 50));
    mixed[2].trainingMetMinutes = 200;
    mixed[5].trainingMetMinutes = 150;
    CHECK_EQ(activityScore(mixed)->contributors[4], 71);
    mixed[6].trainingMetMinutes = 160;
    CHECK_EQ(activityScore(mixed)->contributors[4], 96);
    // Goals reached on two days: 43.
    mixed[0].activeKcal = 560;
    mixed[3].activeKcal = 550;
    CHECK_EQ(activityScore(mixed)->contributors[0], 43);

    // Fewer than seven days give nothing; more only use the last seven.
    CHECK(!activityScore(std::vector<ActivityDay>(6, day(600, 550, 6, 0, 250))).has_value());
    std::vector<ActivityDay> longer(10, day(0, 550, 6, 0, 0));
    for (size_t i = 3; i < 10; ++i)
        longer[i] = day(600, 550, 6.25, 0, 250);
    CHECK_EQ(activityScore(longer)->score, 99);

    // A day from minutes: 24 h at 1.3 MET is all inactive, one long run, no training.
    std::vector<double> still(1440, 1.3);
    ActivityDay a = activityDayFromMinutes(still, 0, 550);
    CHECK(std::fabs(a.sedentaryHours - 24.0) < 1e-9);
    CHECK_EQ(a.inactivityAlerts, 1);
    CHECK(a.trainingMetMinutes == 0);
    // Resting at 0.9 MET is rest, not sitting.
    CHECK(activityDayFromMinutes(std::vector<double>(1440, 0.9), 0, 550).sedentaryHours == 0);
    // An hour at 4 MET is medium effort: 60 x (4 - 0.6) MET-minutes.
    std::vector<double> walk(1440, 0.9);
    for (size_t i = 600; i < 660; ++i)
        walk[i] = 4.0;
    a = activityDayFromMinutes(walk, 200, 550);
    CHECK(std::fabs(a.trainingMetMinutes - 60 * 3.4) < 1e-6);
    CHECK_EQ(a.inactivityAlerts, 0);
    // Windows with fewer than three recorded minutes count as nothing.
    std::vector<double> gaps(1440, std::nan(""));
    gaps[0] = 5;
    CHECK(activityDayFromMinutes(gaps, 0, 550).sedentaryHours == 0);
    return finish("activity_score");
}

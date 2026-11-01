// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/{activity_estimate,activity_score,daysummary,ringclock,sleepscore,oura_sleep_state,stager,breathing,stress_estimate}.cpp ../src/ring/{protocol,events,aes128}.cpp test_activity_estimate.cpp -o /tmp/ae && /tmp/ae
#include "hex.h"
#include "analysis/activity_estimate.h"

using namespace Oura;

namespace {

std::string dateBack(const std::string &date, int back)
{
    const int y = std::atoi(date.substr(0, 4).c_str());
    const unsigned m = static_cast<unsigned>(std::atoi(date.substr(5, 2).c_str()));
    const unsigned d = static_cast<unsigned>(std::atoi(date.substr(8, 2).c_str()));
    return localDate((daysFromCivil(y, m, d) - back) * 86400, 0);
}

// Seven days of 500 kcal active, with the given readiness score on each.
Summary week(int readiness, bool official, std::optional<int> cloudTarget = std::nullopt)
{
    Summary s;
    for (int back = 6; back >= 0; --back) {
        Day d;
        d.date = dateBack("2026-06-30", back);
        d.coveredMin = 1400;
        ActivityDay a;
        a.activeKcal = 500;
        a.sedentaryHours = 8;
        a.trainingMetMinutes = 200;
        d.activityBasis = a;
        (official ? d.readinessScore : d.estimatedReadinessScore) = readiness;
        d.targetKcal = cloudTarget;
        s.days.push_back(d);
    }
    return s;
}

} // namespace

int main()
{
    // The steps, by readiness, from a basic goal of 450.
    const int expected[][2] = { { 38, 150 }, { 43, 200 }, { 50, 250 }, { 62, 300 }, { 69, 300 }, { 70, 450 }, { 84, 450 },
                                { 85, 550 }, { 87, 550 }, { 88, 600 }, { 93, 600 }, { 94, 650 }, { 100, 650 } };
    for (const auto &e : expected)
        CHECK_EQ(activityTargetKcal(450, e[0]), e[1]);
    CHECK_EQ(activityTargetKcal(300, 20), 100);   // never under 100

    AnalysisOptions options;
    options.activeKcalGoal = 450;

    // 500 kcal meets the goal of a readiness of 75 (450) and not that of 90 (600): the contributor
    // for meeting the goal is full on all seven days at the first, and 1 at the second.
    Summary calm = week(75, true);
    estimateActivity(calm, options);
    Summary pushed = week(90, true);
    estimateActivity(pushed, options);
    CHECK(calm.days.back().estimatedActivityScore.has_value());
    CHECK(pushed.days.back().estimatedActivityScore.has_value());
    if (calm.days.back().estimatedActivityScore && pushed.days.back().estimatedActivityScore) {
        CHECK_EQ(calm.days.back().estimatedActivityContributors[0], 100);
        CHECK_EQ(pushed.days.back().estimatedActivityContributors[0], 1);
        CHECK(*calm.days.back().estimatedActivityScore > *pushed.days.back().estimatedActivityScore);
    }

    // The estimate of readiness stands in where Oura's is missing; Oura's goal from the cloud beats both.
    Summary estimated = week(90, false);
    estimateActivity(estimated, options);
    if (estimated.days.back().estimatedActivityScore)
        CHECK_EQ(estimated.days.back().estimatedActivityContributors[0], 1);
    Summary cloud = week(90, true, 400);
    estimateActivity(cloud, options);
    if (cloud.days.back().estimatedActivityScore)
        CHECK_EQ(cloud.days.back().estimatedActivityContributors[0], 100);

    // No readiness at all: the basic goal.
    Summary none = week(0, true);
    for (Day &d : none.days)
        d.readinessScore.reset();
    estimateActivity(none, options);
    if (none.days.back().estimatedActivityScore)
        CHECK_EQ(none.days.back().estimatedActivityContributors[0], 100);

    // Fewer than seven days, or a day with too little movement, gets no score.
    Summary short6 = week(75, true);
    short6.days.erase(short6.days.begin());
    estimateActivity(short6, options);
    CHECK(!short6.days.back().estimatedActivityScore.has_value());
    Summary thin = week(75, true);
    thin.days.back().coveredMin = 300;
    estimateActivity(thin, options);
    CHECK(!thin.days.back().estimatedActivityScore.has_value());
    return finish("activity_estimate");
}

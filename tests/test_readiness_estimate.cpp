// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/{readiness_estimate,oura_night_score,oura_sleep_score,oura_sleep_state,activity_score,daysummary,ringclock,sleepscore,stager}.cpp ../src/ring/{protocol,events,aes128}.cpp test_readiness_estimate.cpp -o /tmp/re && /tmp/re
#include "hex.h"
#include "analysis/readiness_estimate.h"

using namespace Oura;

static std::string dateOf(int dayIndex)
{
    return localDate((daysFromCivil(2026, 1, 1) + dayIndex) * 86400, 0);
}

static Summary history(int days)
{
    Summary s;
    for (int i = 0; i < days; ++i) {
        Night n;
        n.date = dateOf(i);
        n.main = true;
        n.startUnix = (daysFromCivil(2026, 1, 1) + i) * 86400 - 3600;       // 23:00 the evening before
        n.endUnix = n.startUnix + 8 * 3600;
        n.asleepMin = 450;
        n.inBedMin = 480;
        n.restingHr = 50;
        n.avgHrv = 40;
        n.ouraScore = 84;
        for (int k = 0; k < 96; ++k)
            n.hr.push_back({ n.startUnix + k * 300, k == 40 ? 48.0 : 55.0 });   // lowest at 3:20 in
        s.nights.push_back(n);
        Day d;
        d.date = n.date;
        d.activeKcal = 500;
        d.estimatedActivityScore = 80;
        s.days.push_back(d);
    }
    return s;
}

int main()
{
    AnalysisOptions options;
    Summary s = history(75);
    estimateReadiness(s, options);

    // The first days have no history to compare with; later ones are estimated.
    CHECK(!s.days[3].estimatedReadinessScore.has_value());
    const Day &last = s.days.back();
    CHECK(last.estimatedReadinessScore.has_value());
    if (last.estimatedReadinessScore) {
        CHECK(*last.estimatedReadinessScore >= 70 && *last.estimatedReadinessScore <= 100);
        CHECK_EQ(last.estimatedReadinessContributors.size(), size_t(9));
        for (int c : last.estimatedReadinessContributors)
            CHECK(c >= 1 && c <= 100);
    }

    // A steady person with a good night scores well, and a worse night scores less.
    Summary worse = history(75);
    worse.nights.back().ouraScore = 66;
    worse.nights.back().restingHr = 56;       // six beats over the median
    worse.nights.back().tempDeviation = 0.5;
    estimateReadiness(worse, options);
    CHECK(worse.days.back().estimatedReadinessScore.has_value());
    if (worse.days.back().estimatedReadinessScore && last.estimatedReadinessScore)
        CHECK(*worse.days.back().estimatedReadinessScore < *last.estimatedReadinessScore - 10);

    // A day Oura has a readiness score for is left alone, unless asked.
    Summary official = history(75);
    official.days.back().readinessScore = 77;
    estimateReadiness(official, options);
    CHECK(!official.days.back().estimatedReadinessScore.has_value());
    estimateReadiness(official, options, true);
    CHECK(official.days.back().estimatedReadinessScore.has_value());

    // No night, no estimate.
    Summary empty;
    Day d;
    d.date = dateOf(0);
    empty.days.push_back(d);
    estimateReadiness(empty, options);
    CHECK(!empty.days[0].estimatedReadinessScore.has_value());
    return finish("readiness_estimate");
}

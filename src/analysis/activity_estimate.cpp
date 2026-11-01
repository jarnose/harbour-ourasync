#include "activity_estimate.h"

#include <algorithm>
#include <cstdlib>
#include <map>

namespace Oura {

namespace {

std::string shifted(const std::string &date, int back)
{
    const int y = std::atoi(date.substr(0, 4).c_str());
    const unsigned m = static_cast<unsigned>(std::atoi(date.substr(5, 2).c_str()));
    const unsigned d = static_cast<unsigned>(std::atoi(date.substr(8, 2).c_str()));
    return localDate((daysFromCivil(y, m, d) - back) * 86400, 0);
}

} // namespace

int activityTargetKcal(int baseKcal, int readinessScore)
{
    int step;
    if (readinessScore >= 94)
        step = 200;
    else if (readinessScore >= 88)
        step = 150;
    else if (readinessScore >= 85)
        step = 100;
    else if (readinessScore >= 70)
        step = 0;
    else if (readinessScore >= 62)
        step = -150;
    else if (readinessScore >= 48)
        step = -200;
    else if (readinessScore >= 40)
        step = -250;
    else
        step = -300;
    return std::max(100, baseKcal + step);
}

void estimateActivity(Summary &summary, const AnalysisOptions &options)
{
    std::map<std::string, const Day *> dayOf;
    for (const Day &d : summary.days)
        dayOf[d.date] = &d;
    const int base = static_cast<int>(options.activeKcalGoal);

    const auto goalOf = [&](const Day &d) {
        if (d.targetKcal)
            return *d.targetKcal;
        if (d.readinessScore)
            return activityTargetKcal(base, *d.readinessScore);
        if (d.estimatedReadinessScore)
            return activityTargetKcal(base, *d.estimatedReadinessScore);
        return base;
    };

    for (Day &day : summary.days) {
        day.estimatedActivityScore.reset();
        day.estimatedActivityContributors.clear();
        if (!day.activityBasis || day.coveredMin < 720)
            continue;
        std::vector<ActivityDay> week;
        for (int back = 6; back >= 0; --back) {
            const auto it = dayOf.find(shifted(day.date, back));
            if (it == dayOf.end() || !it->second->activityBasis || it->second->coveredMin < 300)
                break;
            ActivityDay a = *it->second->activityBasis;
            a.targetKcal = goalOf(*it->second);
            week.push_back(a);
        }
        if (const auto score = activityScore(week)) {
            day.estimatedActivityScore = score->score;
            day.estimatedActivityContributors.assign(score->contributors.begin(), score->contributors.end());
        }
    }
}

} // namespace Oura

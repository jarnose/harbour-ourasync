#include "sleep_debt.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>

namespace Oura {

namespace {

constexpr int kDays = 14;
constexpr int kMinValidDays = 5;
constexpr double kMaxDebtMin = 600;      // 10 hours
constexpr double kRoundingMin = 45;
constexpr double kOldestWeight = 0.25;
constexpr int kNeedDays = 90;
constexpr size_t kMinNeedDays = 14;

double quantile(const std::vector<double> &sorted, double q)
{
    const double pos = q * static_cast<double>(sorted.size() - 1);
    const size_t lo = static_cast<size_t>(std::floor(pos));
    const size_t hi = std::min(sorted.size() - 1, lo + 1);
    return sorted[lo] + (pos - static_cast<double>(lo)) * (sorted[hi] - sorted[lo]);
}

std::string shifted(const std::string &date, int back)
{
    const int y = std::atoi(date.substr(0, 4).c_str());
    const unsigned m = static_cast<unsigned>(std::atoi(date.substr(5, 2).c_str()));
    const unsigned d = static_cast<unsigned>(std::atoi(date.substr(8, 2).c_str()));
    return localDate((daysFromCivil(y, m, d) - back) * 86400, 0);
}

} // namespace

std::optional<SleepDebt> sleepDebt(const std::vector<double> &sleepMin, const std::vector<double> &needMin)
{
    const size_t n = std::min(sleepMin.size(), needMin.size());
    if (n == 0 || sleepMin[0] <= 0 || needMin[0] <= 0)
        return std::nullopt;
    const double decay = n == 1 ? 0.0 : (1.0 - kOldestWeight) / static_cast<double>(n - 1);
    double debt = 0;
    int valid = 0;
    for (size_t d = 0; d < n; ++d) {
        if (sleepMin[d] <= 0 || needMin[d] <= 0)
            continue;
        ++valid;
        debt += (1.0 - decay * static_cast<double>(d)) * (needMin[d] - sleepMin[d]);
    }
    if (valid < kMinValidDays)
        return std::nullopt;
    debt = std::max(0.0, std::min(debt, kMaxDebtMin));
    SleepDebt out;
    out.debtMin = static_cast<int>(std::lround(std::round(debt / kRoundingMin) * kRoundingMin));
    out.recentShortfallMin = static_cast<int>(std::lround(needMin[0] - sleepMin[0]));
    return out;
}

std::optional<double> sleepNeedMinutes(const std::vector<double> &previousDaysMin)
{
    std::vector<double> v;
    for (double m : previousDaysMin)
        if (m > 0)
            v.push_back(m);
    if (v.size() < kMinNeedDays)
        return std::nullopt;
    std::sort(v.begin(), v.end());
    const double q1 = quantile(v, 0.25), q3 = quantile(v, 0.75);
    const double lo = q1 - 1.5 * (q3 - q1), hi = q3 + 1.5 * (q3 - q1);
    double sum = 0;
    size_t kept = 0;
    for (double m : v)
        if (m >= lo && m <= hi) {
            sum += m;
            ++kept;
        }
    return kept ? std::optional<double>(sum / static_cast<double>(kept)) : std::nullopt;
}

void estimateSleepDebt(Summary &summary)
{
    // Each date's total sleep, naps included.
    std::map<std::string, double> total;
    for (const Night &n : summary.nights)
        if (!n.date.empty() && n.asleepMin > 0)
            total[n.date] += n.asleepMin;

    const auto totalOf = [&](const std::string &date) {
        const auto it = total.find(date);
        return it == total.end() ? 0.0 : it->second;
    };
    const auto needOn = [&](const std::string &date) -> std::optional<double> {
        std::vector<double> prior;
        for (int k = 1; k <= kNeedDays; ++k)
            prior.push_back(totalOf(shifted(date, k)));
        return sleepNeedMinutes(prior);
    };

    for (Day &day : summary.days) {
        if (totalOf(day.date) <= 0)
            continue;
        std::vector<double> sleep, need;
        for (int k = 0; k < kDays; ++k) {
            const std::string date = shifted(day.date, k);
            sleep.push_back(totalOf(date));
            need.push_back(sleep.back() > 0 ? needOn(date).value_or(0.0) : 0.0);
        }
        if (const auto debt = sleepDebt(sleep, need)) {
            day.estimatedSleepDebtMin = debt->debtMin;
            day.estimatedSleepNeedMin = static_cast<int>(std::lround(need[0]));
        }
    }
}

} // namespace Oura

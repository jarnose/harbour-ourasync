// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/{sleep_debt,daysummary,ringclock,sleepscore,oura_sleep_state,stager,activity_score,breathing,stress_estimate}.cpp ../src/ring/{protocol,events,aes128}.cpp test_sleep_debt.cpp -o /tmp/sd && /tmp/sd
#include "hex.h"
#include "analysis/sleep_debt.h"

#include <cmath>

using namespace Oura;

namespace {

Night night(const std::string &date, double asleepMin)
{
    Night n;
    n.date = date;
    n.asleepMin = asleepMin;
    n.main = true;
    return n;
}

std::string dateBack(const std::string &date, int back)
{
    const int y = std::atoi(date.substr(0, 4).c_str());
    const unsigned m = static_cast<unsigned>(std::atoi(date.substr(5, 2).c_str()));
    const unsigned d = static_cast<unsigned>(std::atoi(date.substr(8, 2).c_str()));
    return localDate((daysFromCivil(y, m, d) - back) * 86400, 0);
}

} // namespace

int main()
{
    // The library's own example (open_oura's test): needing 8 hours and sleeping 7, five nights:
    // weights 1, 0.8125, 0.625, 0.4375, 0.25 add to 3.125 hours of shortfall, rounded to 45 minutes: 3 hours.
    auto d = sleepDebt(std::vector<double>(5, 420), std::vector<double>(5, 480));
    CHECK(d.has_value());
    if (d) {
        CHECK_EQ(d->debtMin, 180);
        CHECK_EQ(d->recentShortfallMin, 60);
    }

    // Over fourteen days the weights run 1 to 0.25 in steps of 0.75/13.
    d = sleepDebt(std::vector<double>(14, 420), std::vector<double>(14, 480));
    CHECK(d.has_value());
    if (d)
        CHECK_EQ(d->debtMin, 540);   // 60 * 14 * 0.625 = 525 -> 540

    // Held between none and ten hours (and then rounded); sleeping more than the need pays nothing back past zero.
    d = sleepDebt(std::vector<double>(14, 200), std::vector<double>(14, 480));
    CHECK(d.has_value());
    if (d)
        CHECK_EQ(d->debtMin, 585);   // held at 10 h, then rounded down to 13 steps of 45 minutes
    d = sleepDebt(std::vector<double>(14, 600), std::vector<double>(14, 480));
    CHECK(d.has_value());
    if (d) {
        CHECK_EQ(d->debtMin, 0);
        CHECK_EQ(d->recentShortfallMin, -120);
    }

    // Four days with sleep are too few; so is a newest day without any.
    std::vector<double> sleep(14, 0), need(14, 480);
    for (int i = 0; i < 4; ++i)
        sleep[static_cast<size_t>(i)] = 400;
    CHECK(!sleepDebt(sleep, need).has_value());
    sleep[4] = 400;
    CHECK(sleepDebt(sleep, need).has_value());
    sleep[0] = 0;
    CHECK(!sleepDebt(sleep, need).has_value());

    // The need: the mean of the days, without the ones far from the middle half.
    std::vector<double> days(30, 480);
    days[3] = 120;    // a night of three hours
    days[9] = 900;    // and one of fifteen
    days[12] = 0;     // a day without sleep is not a short night
    auto need90 = sleepNeedMinutes(days);
    CHECK(need90.has_value());
    if (need90)
        CHECK(std::fabs(*need90 - 480) < 0.01);
    CHECK(!sleepNeedMinutes(std::vector<double>(13, 480)).has_value());

    // Over a summary: forty days of 8 hours, then the last week of 6.
    Summary summary;
    const std::string last = "2026-06-30";
    for (int back = 59; back >= 0; --back) {
        const std::string date = dateBack(last, back);
        summary.nights.push_back(night(date, back < 7 ? 360 : 480));
        Day day;
        day.date = date;
        summary.days.push_back(day);
    }
    // A nap counts toward its day.
    summary.nights.push_back(night(last, 30));
    summary.nights.back().main = false;
    estimateSleepDebt(summary);
    const Day &today = summary.days.back();
    CHECK(today.estimatedSleepDebtMin.has_value());
    CHECK(today.estimatedSleepNeedMin.has_value());
    if (today.estimatedSleepNeedMin)
        CHECK(*today.estimatedSleepNeedMin > 440 && *today.estimatedSleepNeedMin <= 480);
    if (today.estimatedSleepDebtMin) {
        CHECK(*today.estimatedSleepDebtMin >= 270);   // a week of two hours' shortfall, weighted
        CHECK_EQ(*today.estimatedSleepDebtMin % 45, 0);
    }
    // An early day has no 90-day history to take a need from.
    CHECK(!summary.days.front().estimatedSleepDebtMin.has_value());
    return finish("sleep_debt");
}

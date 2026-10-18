// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/stress_estimate.cpp test_stress_estimate.cpp -o /tmp/se && /tmp/se
#include "hex.h"
#include "analysis/stress_estimate.h"

using namespace Oura;

int main()
{
    StressInput in;
    for (int i = 0; i < 300; ++i)
        in.history.push_back(60 + i % 5);   // median 62
    for (int i = 0; i < 30; ++i) in.today.push_back(62);
    for (int i = 0; i < 30; ++i) in.today.push_back(72);
    for (int i = 0; i < 10; ++i) in.today.push_back(80);
    for (int i = 0; i < 10; ++i) in.today.push_back(50);

    // Worked out by hand from the weights (21 slots of five minutes).
    auto r = estimateStressMinutes(in);
    CHECK(r.has_value());
    if (r) CHECK_EQ(*r, 105);

    // Night HRV above the recent mean lowers it, below raises it; with fewer than five prior nights it is ignored.
    in.priorHrv.assign(10, 40.0);
    in.nightHrv = 60.0;
    r = estimateStressMinutes(in);
    CHECK(r.has_value());
    if (r) CHECK_EQ(*r, 85);
    in.nightHrv = 30.0;
    r = estimateStressMinutes(in);
    CHECK(r.has_value());
    if (r) CHECK_EQ(*r, 110);
    in.priorHrv.resize(4);
    r = estimateStressMinutes(in);
    CHECK(r.has_value());
    if (r) CHECK_EQ(*r, 105);

    // More slots well over the usual means more minutes.
    StressInput calm = in;
    calm.today.assign(80, 62);
    const auto c = estimateStressMinutes(calm);
    CHECK(c.has_value());
    if (c && r) CHECK(*c < *r);

    // Never more slots than the day had, and always a multiple of five.
    StressInput few = in;
    few.today = { 90, 91, 92 };
    const auto f = estimateStressMinutes(few);
    CHECK(f.has_value());
    if (f) {
        CHECK(*f <= 15);
        CHECK_EQ(*f % 5, 0);
    }

    // Too little to say anything.
    StressInput young = in;
    young.history.resize(199);
    CHECK(!estimateStressMinutes(young).has_value());
    StressInput empty = in;
    empty.today.clear();
    CHECK(!estimateStressMinutes(empty).has_value());
    return finish("stress_estimate");
}

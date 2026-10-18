#include "stress_estimate.h"

#include <algorithm>
#include <cmath>

namespace Oura {

namespace {

constexpr size_t kMinHistory = 200;
constexpr size_t kMinPriorHrv = 5;
constexpr int kUp[6] = { 4, 8, 12, 16, 20, 26 };
constexpr int kDown[3] = { -4, -8, -12 };

// Fitted by ridge regression on slots of high stress per day (stress_fit.py): intercept, slots
// today, slots at 4/8/12/16/20/26 bpm or more over the median, at 4/8/12 or more under it, the
// median's distance over the baseline, the night's HRV against its recent mean.
constexpr double kWeights[13] = { 7.35820, 0.02203, 2.31616, -0.25607, 1.64458, -1.05477, 3.28400,
                                  4.64422, 0.47477, -0.02837, 0.32369, 3.63298, -1.56947 };

// The median; for an even count the mean of the middle two, which is what numpy gives.
double median(std::vector<double> v)
{
    std::sort(v.begin(), v.end());
    const size_t n = v.size();
    return n % 2 ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) / 2.0;
}

} // namespace

std::optional<int> estimateStressMinutes(const StressInput &in)
{
    if (in.history.size() < kMinHistory || in.today.empty())
        return std::nullopt;

    const double base = median(in.history);
    std::vector<double> delta;
    delta.reserve(in.today.size());
    for (double h : in.today)
        delta.push_back(h - base);

    double f[13];
    int k = 0;
    f[k++] = 1.0;
    f[k++] = static_cast<double>(delta.size()) / 100.0;
    for (int t : kUp)
        f[k++] = static_cast<double>(std::count_if(delta.begin(), delta.end(), [t](double d) { return d >= t; })) / 10.0;
    for (int t : kDown)
        f[k++] = static_cast<double>(std::count_if(delta.begin(), delta.end(), [t](double d) { return d <= t; })) / 10.0;
    f[k++] = median(delta) / 5.0;
    double hrv = 0;
    if (in.nightHrv && in.priorHrv.size() >= kMinPriorHrv) {
        double sum = 0;
        for (double v : in.priorHrv)
            sum += v;
        hrv = (*in.nightHrv - sum / static_cast<double>(in.priorHrv.size())) / 10.0;
    }
    f[k++] = hrv;

    double slots = 0;
    for (int i = 0; i < 13; ++i)
        slots += kWeights[i] * f[i];
    // Never more slots than there were.
    slots = std::max(0.0, std::min(slots, static_cast<double>(delta.size())));
    return static_cast<int>(std::lround(slots)) * 5;
}

} // namespace Oura

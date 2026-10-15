#include "sleepscore.h"

#include <cmath>

namespace Oura {

double curve(double x, const std::vector<std::pair<double, double>> &anchors)
{
    if (anchors.empty())
        return 0;
    if (x <= anchors.front().first)
        return anchors.front().second;
    for (size_t i = 1; i < anchors.size(); ++i) {
        const auto &a = anchors[i - 1];
        const auto &b = anchors[i];
        if (x <= b.first) {
            const double span = b.first - a.first;
            const double t = std::fabs(span) < 1e-12 ? 0.0 : (x - a.first) / span;
            return a.second + t * (b.second - a.second);
        }
    }
    return anchors.back().second;
}

std::optional<std::pair<double, double>> causalBaseline(const std::vector<double> &prior)
{
    if (prior.size() < 3)
        return std::nullopt;
    double sum = 0;
    for (double v : prior)
        sum += v;
    const double mean = sum / static_cast<double>(prior.size());
    double var = 0;
    for (double v : prior)
        var += (v - mean) * (v - mean);
    var /= static_cast<double>(prior.size());
    const double sd = std::sqrt(var);
    return std::make_pair(mean, sd > 1e-6 ? sd : 1.0);
}

namespace {

struct Band
{
    double mayLow, recLow, recHigh, mayHigh;
};

// Hirshkowitz 2015, table 1: hours of sleep by age band.
Band durationBand(double age)
{
    if (age < 14)
        return { 7, 9, 11, 12 };
    if (age < 18)
        return { 7, 8, 10, 11 };
    if (age < 26)
        return { 6, 7, 9, 11 };
    if (age < 65)
        return { 6, 7, 9, 10 };
    return { 5, 7, 8, 9 };
}

} // namespace

SleepScore scoreNight(const NightInput &in)
{
    std::vector<ScoreComponent> parts;
    const auto add = [&parts](const char *key, double score, double weight) {
        parts.push_back({ key, score, weight });
    };

    // Duration is weighted highest: it is the best-measured thing a ring reports
    // and has the strongest outcome literature behind it.
    if (in.asleepMin) {
        const double hours = *in.asleepMin / 60.0;
        const Band b = durationBand(in.age);
        add("duration",
            curve(hours, { { b.mayLow - 2, 0 },
                           { b.mayLow, 50 },
                           { b.recLow, 90 },
                           { b.recLow + 0.5, 100 },
                           { b.recHigh, 100 },
                           { b.mayHigh, 75 },
                           { b.mayHigh + 2, 40 } }),
            0.30);
    }
    // At or above 85 % is appropriate for every adult band; under 75 % is not.
    if (in.efficiencyPct)
        add("efficiency",
            curve(*in.efficiencyPct, { { 60, 0 }, { 75, 40 }, { 85, 85 }, { 92, 100 }, { 100, 100 } }),
            0.20);
    // Up to 15 minutes is appropriate for adults; over 60 is not.
    if (in.onsetLatencyMin)
        add("onset_latency",
            curve(*in.onsetLatencyMin,
                  { { 0, 100 }, { 15, 100 }, { 30, 70 }, { 45, 40 }, { 60, 20 }, { 120, 0 } }),
            0.10);
    if (in.wasoMin) {
        const double ok = in.age >= 65 ? 30.0 : 20.0;
        add("waso",
            curve(*in.wasoMin,
                  { { 0, 100 }, { ok, 95 }, { ok * 2, 65 }, { ok * 3, 35 }, { ok * 5, 0 } }),
            0.15);
    }
    // The paper counts awakenings longer than five minutes. A count taken from a
    // hypnogram includes brief arousals the paper would not, so this is scored
    // leniently and weighted low.
    if (in.awakenings)
        add("awakenings",
            curve(*in.awakenings, { { 0, 100 }, { 2, 95 }, { 5, 75 }, { 10, 45 }, { 20, 10 } }), 0.05);

    // Architecture gets the smallest weight on purpose: consumer staging is
    // weakest at deep sleep, so scoring it heavily would mostly score the error.
    {
        std::vector<double> arch;
        if (in.deepPct)
            arch.push_back(curve(*in.deepPct,
                                 { { 0, 0 }, { 8, 50 }, { 13, 95 }, { 16, 100 }, { 23, 100 }, { 35, 85 } }));
        if (in.remPct)
            arch.push_back(curve(*in.remPct,
                                 { { 0, 0 }, { 10, 45 }, { 20, 95 }, { 22, 100 }, { 25, 100 }, { 35, 80 } }));
        if (!arch.empty()) {
            double sum = 0;
            for (double v : arch)
                sum += v;
            add("architecture", sum / static_cast<double>(arch.size()), 0.10);
        }
    }

    // Restorative physiology is scored against the wearer's own baseline: the
    // population spread of resting HR and HRV dwarfs the night-to-night change
    // that means something.
    {
        std::vector<double> phys;
        if (in.restingHr && in.restingHrBaseline && in.restingHrBaseline->second > 0) {
            const double z = (*in.restingHr - in.restingHrBaseline->first) / in.restingHrBaseline->second;
            phys.push_back(curve(z, { { -2, 100 }, { 0, 90 }, { 1, 70 }, { 2, 40 }, { 3.5, 10 } }));
        }
        if (in.hrvMs && in.hrvBaseline && in.hrvBaseline->second > 0) {
            const double z = (*in.hrvMs - in.hrvBaseline->first) / in.hrvBaseline->second;
            phys.push_back(curve(z, { { -3, 10 }, { -2, 35 }, { -1, 70 }, { 0, 90 }, { 1.5, 100 } }));
        }
        if (!phys.empty()) {
            double sum = 0;
            for (double v : phys)
                sum += v;
            add("physiology", sum / static_cast<double>(phys.size()), 0.10);
        }
    }

    SleepScore out;
    if (parts.empty())
        return out;
    double totalWeight = 0;
    for (const ScoreComponent &c : parts)
        totalWeight += c.weight;
    double total = 0;
    for (ScoreComponent &c : parts) {
        total += c.score * c.weight;
        c.weight /= totalWeight;
    }
    out.valid = true;
    out.score = static_cast<int>(std::lround(total / totalWeight));
    out.components = std::move(parts);
    return out;
}

} // namespace Oura

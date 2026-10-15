#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

// A sleep score you can argue with.
//
// Oura's own 0-100 score is computed on the phone by its closed `ecore` engine;
// the ring never sends it. This is the other thing: a score built from
// published sleep science, where every threshold is traceable to a paper rather
// than to a fit against a vendor's black box. The two will disagree, and that is
// the point of labelling it "estimate" everywhere it is shown.
//
// Ported from open_health's sleep_score.rs (MIT, Thomas Marchand), whose
// thresholds are transcribed from:
//  * Ohayon et al., Sleep Health 2017;3(1):6-19, NSF sleep quality: efficiency,
//    onset latency, wake after sleep onset, awakenings.
//  * Hirshkowitz et al., Sleep Health 2015;1(1):40-43, NSF duration by age.
//  * Boulos et al., Lancet Respir Med 2019;7(6):533-543, stage shares.
//  * de Zambotti et al., Behav Sleep Med 2019;17(2):124-136, why architecture is
//    weighted low: ring staging agrees with PSG far better on asleep-versus-awake
//    than on which stage.
//
// Each component scores 0-100 through a piecewise-linear curve pinned at the
// papers' band edges and the components combine as a weighted mean. A component
// with no data drops out and the rest renormalise, so a night the ring did not
// stage is scored on what it has instead of being marked down for what it lacks.
namespace Oura {

struct NightInput
{
    std::optional<double> asleepMin;
    std::optional<double> efficiencyPct;
    std::optional<double> onsetLatencyMin;
    std::optional<double> wasoMin;
    std::optional<double> awakenings;
    std::optional<double> deepPct;
    std::optional<double> remPct;
    std::optional<double> restingHr;
    std::optional<std::pair<double, double>> restingHrBaseline; // mean, sd
    std::optional<double> hrvMs;
    std::optional<std::pair<double, double>> hrvBaseline;
    double age = 40;
};

struct ScoreComponent
{
    std::string key;
    double score = 0;   // 0-100
    double weight = 0;  // after renormalising for what the night lacked
};

struct SleepScore
{
    bool valid = false;
    int score = 0;
    std::vector<ScoreComponent> components;
};

SleepScore scoreNight(const NightInput &input);

// Piecewise-linear interpolation through (input, score) anchors sorted by
// input; clamps outside the ends so an absurd input cannot give an absurd score.
double curve(double x, const std::vector<std::pair<double, double>> &anchors);

// Mean and standard deviation of strictly preceding nights, or nothing when
// there are fewer than three: a baseline from two nights is noise.
std::optional<std::pair<double, double>> causalBaseline(const std::vector<double> &prior);

} // namespace Oura

#include "stager.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Oura {

namespace {

enum : int { Deep = 1, Light = 2, Rem = 3, Wake = 4 };

using Series = std::vector<TimedValue>;

Series filtered(const Series &in, double lo, double hi)
{
    Series out;
    for (const TimedValue &p : in)
        if (p.value >= lo && p.value <= hi)
            out.push_back(p);
    std::stable_sort(out.begin(), out.end(), [](const TimedValue &a, const TimedValue &b) { return a.utc < b.utc; });
    return out;
}

// Linear interpolation, clamped at the ends. The series is sorted.
double interp(const Series &s, double x)
{
    if (s.empty())
        return 0;
    if (x <= static_cast<double>(s.front().utc))
        return s.front().value;
    if (x >= static_cast<double>(s.back().utc))
        return s.back().value;
    size_t lo = 0, hi = s.size() - 1;
    while (lo + 1 < hi) {
        const size_t mid = (lo + hi) / 2;
        if (static_cast<double>(s[mid].utc) <= x)
            lo = mid;
        else
            hi = mid;
    }
    const double tl = static_cast<double>(s[lo].utc), th = static_cast<double>(s[hi].utc);
    if (th <= tl)
        return s[lo].value;
    const double f = (x - tl) / (th - tl);
    return s[lo].value * (1 - f) + s[hi].value * f;
}

double meanOf(const std::vector<double> &a, size_t from, size_t to)
{
    if (to <= from)
        return 0;
    double s = 0;
    for (size_t i = from; i < to; ++i)
        s += a[i];
    return s / static_cast<double>(to - from);
}

int clampI(int x, int lo, int hi)
{
    return std::max(lo, std::min(hi, x));
}

double clampD(double x, double lo, double hi)
{
    return std::max(lo, std::min(hi, x));
}

std::vector<double> epochMotion(const Series &motion, double start, double span, size_t n)
{
    std::vector<double> mo(n, 0.0);
    for (const TimedValue &m : motion) {
        const double f = (static_cast<double>(m.utc) - start) / span;
        if (f >= -1e-6 && f <= 1 + 1e-6) {
            const double idx = std::max(0.0, std::min(static_cast<double>(n) - 1, std::floor(f * static_cast<double>(n))));
            const size_t i = static_cast<size_t>(idx);
            mo[i] = std::max(mo[i], m.value);
        }
    }
    return mo;
}

// Mode filter over a centred odd window; unknown epochs stay unknown.
std::vector<int> smooth(const std::vector<int> &vals, size_t win)
{
    if (vals.size() < win || win < 3)
        return vals;
    const size_t half = win / 2;
    std::vector<int> out(vals.size());
    for (size_t i = 0; i < vals.size(); ++i) {
        if (vals[i] == 0) {
            out[i] = 0;
            continue;
        }
        const size_t a = i >= half ? i - half : 0, b = std::min(vals.size(), i + half + 1);
        int counts[5] = { 0, 0, 0, 0, 0 };
        for (size_t k = a; k < b; ++k)
            if (vals[k] >= 1 && vals[k] <= 4)
                ++counts[vals[k]];
        int best = vals[i], bestCount = -1;
        // On a tie the last of the four wins, as in the original.
        for (int s = 1; s <= 4; ++s)
            if (counts[s] >= bestCount) {
                bestCount = counts[s];
                best = s;
            }
        out[i] = best;
    }
    return out;
}

// Consolidated deep bouts inside light sleep: heart-rate dips, stillness, early in the night.
std::vector<int> refineDeep(const std::vector<int> &stages, const Series &hrIn, const Series &hrvIn,
                            const Series &motion, double start, double end)
{
    const size_t n = stages.size();
    if (n < 40 || std::find(stages.begin(), stages.end(), static_cast<int>(Deep)) != stages.end() || end <= start)
        return stages;
    size_t lightCount = 0;
    for (int s : stages)
        if (s == Light)
            ++lightCount;
    if (lightCount < 20)
        return stages;
    const Series hrPts = filtered(hrIn, 30.0001, 139.9999);
    if (hrPts.size() < 6)
        return stages;
    const Series hrvPts = filtered(hrvIn, 1e-9, std::numeric_limits<double>::max());

    const double span = std::max(1.0, end - start);
    const double epochS = span / static_cast<double>(n);
    std::vector<double> hrEp(n), hrvEp(n, 0.0);
    for (size_t i = 0; i < n; ++i)
        hrEp[i] = interp(hrPts, start + (static_cast<double>(i) + 0.5) * epochS);
    if (!hrvPts.empty())
        for (size_t i = 0; i < n; ++i)
            hrvEp[i] = interp(hrvPts, start + (static_cast<double>(i) + 0.5) * epochS);
    const std::vector<double> mo = epochMotion(motion, start, span, n);

    double hrMin = std::numeric_limits<double>::infinity(), hrMax = -std::numeric_limits<double>::infinity();
    for (double h : hrEp) {
        hrMin = std::min(hrMin, h);
        hrMax = std::max(hrMax, h);
    }
    if (hrMax - hrMin < 2.0)
        return stages;

    std::vector<double> localHr(n);
    for (size_t i = 0; i < n; ++i) {
        const size_t lo = i >= 90 ? i - 90 : 0, hi = std::min(n, i + 91);
        localHr[i] = meanOf(hrEp, lo, hi);
    }
    std::vector<double> sortedHrv;
    for (const TimedValue &p : hrvPts)
        sortedHrv.push_back(p.value);
    std::sort(sortedHrv.begin(), sortedHrv.end());
    const double hrvMed = sortedHrv.empty() ? 0 : sortedHrv[sortedHrv.size() / 2];

    std::vector<size_t> sleepIdx;
    for (size_t i = 0; i < n; ++i)
        if (stages[i] >= 1 && stages[i] <= 3)
            sleepIdx.push_back(i);
    if (sleepIdx.size() < 20)
        return stages;
    const double onset = static_cast<double>(sleepIdx.front());
    const double finalSleep = static_cast<double>(sleepIdx.back());
    const double sleepSpan = std::max(1.0, finalSleep - onset);

    struct Range { size_t s, e; };
    std::vector<Range> runs;
    for (size_t i = 0; i < n;) {
        if (stages[i] == Light && mo[i] <= 1.0) {
            size_t j = i;
            while (j < n && stages[j] == Light && mo[j] <= 1.0)
                ++j;
            if (j - i >= 12)
                runs.push_back({ i, j });
            i = j;
        } else {
            ++i;
        }
    }
    if (runs.empty())
        return stages;

    std::vector<Range> segmented;
    for (const Range &r : runs) {
        if (r.e - r.s <= 180) {
            segmented.push_back(r);
        } else {
            size_t cur = r.s;
            while (cur + 12 <= r.e) {
                size_t segEnd = std::min(cur + 90, r.e);
                if (r.e - segEnd < 24)
                    segEnd = r.e;
                segmented.push_back({ cur, segEnd });
                cur = segEnd;
            }
        }
    }

    size_t firstCycleMaxLen = 0;
    for (const Range &r : segmented) {
        const double frac = ((static_cast<double>(r.s + r.e) / 2.0) - onset) / sleepSpan;
        if (frac < 0.16)
            firstCycleMaxLen = std::max(firstCycleMaxLen, r.e - r.s);
    }

    struct Scored { double score; size_t s, e; double frac, dip; };
    std::vector<Scored> scored;
    for (const Range &r : segmented) {
        const size_t runLen = r.e - r.s;
        const double mid = static_cast<double>(r.s + r.e) / 2.0;
        const double sleepFrac = std::max(0.0, (mid - onset) / sleepSpan);
        if (sleepFrac > 0.56)
            continue;
        if (sleepFrac < 0.12 && firstCycleMaxLen >= 40 && static_cast<double>(runLen) < static_cast<double>(firstCycleMaxLen) * 0.65)
            continue;
        const double meanHr = meanOf(hrEp, r.s, r.e);
        const double meanLoc = meanOf(localHr, r.s, r.e);
        const size_t prevEnd = std::max<size_t>(r.s, 1);
        const double prevHr = meanOf(hrEp, 0, prevEnd);
        const double meanHrv = hrvPts.empty() ? 0 : meanOf(hrvEp, r.s, r.e);
        const double hrDip = meanLoc - meanHr;
        const double prevDip = prevHr - meanHr;
        if (prevDip < -0.25 || (prevDip < 0.10 && hrDip < 0.35))
            continue;
        if (!sortedHrv.empty() && sleepFrac > 0.12 && meanHrv > std::max(hrvMed * 1.15, 38.0) && hrDip < 2.0)
            continue;
        const double homeo = std::exp(-2.2 * sleepFrac);
        const double durBonus = clampD(std::max(0.0, static_cast<double>(runLen) - 12) / 48.0, 0, 1);
        const double dipBonus = clampD(hrDip / 3.0, -0.5, 0.6);
        const double score = homeo * 0.55 + durBonus * 0.35 + dipBonus * 0.25;
        scored.push_back({ score, r.s, r.e, sleepFrac, hrDip });
    }
    std::stable_sort(scored.begin(), scored.end(), [](const Scored &a, const Scored &b) { return a.score > b.score; });

    const size_t asleep = sleepIdx.size();
    const int targetDeep = std::max(static_cast<int>(std::lround(static_cast<double>(asleep) * 0.105)), 12);
    std::vector<int> out = stages;
    std::vector<int> centers;
    int deepCount = 0;
    for (const Scored &sr : scored) {
        if (deepCount >= targetDeep)
            break;
        const int rs = static_cast<int>(sr.s), re = static_cast<int>(sr.e);
        const double sleepFrac = sr.frac, hrDip = sr.dip;
        const int runLen = re - rs;
        const int mid = (rs + re) / 2;
        bool near = false;
        for (int c : centers)
            if (std::abs(mid - c) < 110)
                near = true;
        if (near)
            continue;
        const int remaining = targetDeep - deepCount;
        if (remaining < 8)
            break;
        int sIdx, eIdx;
        if (sleepFrac < 0.16) {
            const int lead = rs <= static_cast<int>(onset) + 12 ? 2 : clampI(std::max(0, runLen - 12) / 5, 2, 10);
            const int tail = clampI(std::max(0, runLen - (12 + lead)) / 6, 1, 4);
            const int maxBout = std::min(std::min(56, std::max(0, runLen - (lead + tail))), remaining);
            if (maxBout < 8)
                continue;
            const int searchEnd = std::max(std::max(0, re - (tail + maxBout)), rs + lead);
            int bestS = rs + lead;
            double bestHr = std::numeric_limits<double>::infinity();
            for (int cand = rs + lead; cand <= searchEnd; ++cand) {
                if (cand + maxBout > static_cast<int>(n))
                    break;
                const double m = meanOf(hrEp, static_cast<size_t>(cand), static_cast<size_t>(cand + maxBout));
                if (m < bestHr) {
                    bestHr = m;
                    bestS = cand;
                }
            }
            sIdx = bestS;
            eIdx = bestS + maxBout;
        } else if (sleepFrac < 0.36) {
            const int cap = hrDip >= 2.0 ? clampI((runLen * 9) / 20, 10, 42) : clampI((runLen * 2) / 5, 10, 26);
            const int maxBout = std::min(std::min(cap, remaining), runLen);
            const int tail = clampI(std::max(0, runLen - maxBout) / 4, 1, 4);
            const int e = std::max(0, re - tail);
            const int s = std::max(std::max(0, e - maxBout), rs + std::min(4, runLen));
            sIdx = s;
            eIdx = e;
        } else {
            const int maxBout = std::min(std::min(clampI((runLen * 2) / 5, 8, 22), remaining), runLen);
            const int s = rs + std::max(0, runLen - maxBout) / 2;
            sIdx = s;
            eIdx = s + maxBout;
        }
        if (eIdx <= sIdx || eIdx - sIdx < 8)
            continue;
        eIdx = std::min(eIdx, static_cast<int>(n));
        for (int k = sIdx; k < eIdx; ++k)
            out[static_cast<size_t>(k)] = Deep;
        deepCount += eIdx - sIdx;
        centers.push_back((sIdx + eIdx) / 2);
    }
    return out;
}

} // namespace

std::vector<uint8_t> stageFromSignals(double start, double end, std::vector<TimedValue> hr,
                                      std::vector<TimedValue> hrv, std::vector<TimedValue> motion)
{
    if (end <= start)
        return {};
    const size_t epochs = static_cast<size_t>(std::max(1.0, std::floor((end - start) / 30.0)));
    if (epochs < 120)
        return {};
    const Series hrPts = filtered(hr, 30.0001, 139.9999);
    if (hrPts.size() < 12)
        return {};
    const Series hrvPts = filtered(hrv, 1e-9, std::numeric_limits<double>::max());
    std::stable_sort(motion.begin(), motion.end(), [](const TimedValue &a, const TimedValue &b) { return a.utc < b.utc; });

    const double span = std::max(1.0, end - start);
    const double epochS = span / static_cast<double>(epochs);
    std::vector<double> hrEp(epochs), hrvRaw(epochs, 0.0), hrvEp(epochs);
    for (size_t i = 0; i < epochs; ++i)
        hrEp[i] = interp(hrPts, start + (static_cast<double>(i) + 0.5) * epochS);
    if (!hrvPts.empty())
        for (size_t i = 0; i < epochs; ++i)
            hrvRaw[i] = interp(hrvPts, start + (static_cast<double>(i) + 0.5) * epochS);
    for (size_t i = 0; i < epochs; ++i) {
        const size_t lo = i >= 12 ? i - 12 : 0, hi = std::min(epochs, i + 13);
        double s = 0;
        for (size_t k = lo; k < hi; ++k)
            s += hrvRaw[k];
        hrvEp[i] = s / static_cast<double>(std::max<size_t>(1, hi - lo));
    }
    const std::vector<double> mo = epochMotion(motion, start, span, epochs);

    std::vector<double> sortedHr = hrEp;
    std::sort(sortedHr.begin(), sortedHr.end());
    const double hrMed = sortedHr[sortedHr.size() / 2];
    double hrvMed = 0, hrvP62 = 0, hrvP80 = 0;
    if (!hrvPts.empty()) {
        std::vector<double> sh = hrvEp;
        std::sort(sh.begin(), sh.end());
        const size_t nh = sh.size() - 1;
        hrvMed = sh[nh / 2];
        hrvP62 = sh[(nh * 62) / 100];
        hrvP80 = sh[(nh * 80) / 100];
    }

    std::vector<int> stages(epochs, Light);
    size_t onset = 0;
    const size_t maxOnset = std::min<size_t>(epochs / 4, 120);
    for (size_t i = 0; i < maxOnset; ++i) {
        const size_t endChk = std::min(i + 10, epochs);
        bool quiet = true;
        for (size_t k = i; k < endChk; ++k)
            if (mo[k] > 4.0) {
                quiet = false;
                break;
            }
        if (quiet) {
            onset = i;
            break;
        }
    }
    for (size_t i = 0; i < onset; ++i)
        stages[i] = Wake;

    size_t wakeEnd = epochs;
    const size_t minWakeEnd = std::max(onset, epochs >= 90 ? epochs - 90 : 0);
    bool foundQuietEnd = false;
    for (size_t i = epochs; i-- > minWakeEnd;) {
        const size_t loChk = std::max(i >= 20 ? i - 20 : 0, onset);
        bool quiet = true;
        for (size_t k = loChk; k <= i; ++k)
            if (mo[k] > 2.0) {
                quiet = false;
                break;
            }
        if (quiet) {
            wakeEnd = i + 1;
            foundQuietEnd = true;
            break;
        }
    }
    if (!foundQuietEnd)
        while (wakeEnd > minWakeEnd && mo[wakeEnd - 1] >= 4.0)
            --wakeEnd;
    for (size_t i = wakeEnd; i < epochs; ++i)
        stages[i] = Wake;

    // Brief arousals around substantial movement.
    for (size_t i = onset; i < wakeEnd; ++i) {
        const size_t loN = std::max(i >= 4 ? i - 4 : 0, onset), hiN = std::min(i + 5, wakeEnd);
        double near = 0;
        for (size_t k = loN; k < hiN; ++k)
            near += mo[k];
        if (mo[i] >= 8.0 || (mo[i] >= 5.0 && near >= 10.0)) {
            const size_t lo = std::max(i >= 1 ? i - 1 : 0, onset), hi = std::min(i + 2, wakeEnd);
            for (size_t k = lo; k < hi; ++k)
                stages[k] = Wake;
        }
    }

    std::vector<int> refined = refineDeep(stages, hrPts, hrvPts, motion, start, end);

    const size_t remStart = std::min(onset + 90, wakeEnd);
    const size_t remEnd = std::max(wakeEnd >= 2 ? wakeEnd - 2 : 0, remStart);
    const double sleepSpan = std::max(1.0, static_cast<double>(wakeEnd) - static_cast<double>(onset));
    for (size_t i = remStart; i < remEnd; ++i) {
        if (refined[i] != Light || mo[i] > 2.0)
            continue;
        const double frac = (static_cast<double>(i) - static_cast<double>(onset)) / sleepSpan;
        const double cyclePos = static_cast<double>((i - onset) % 176) / 176.0;
        const bool inRemPhase = cyclePos >= 0.56 && cyclePos <= 0.90;
        bool isRem;
        if (!hrvPts.empty() && hrvP80 > hrvMed) {
            const double h = hrvEp[i];
            isRem = (inRemPhase && h >= hrvP62) || (frac >= 0.30 && h >= hrvP80) ||
                    (frac >= 0.80 && inRemPhase && h >= hrvMed * 0.90);
        } else {
            isRem = inRemPhase && (frac >= 0.35 || hrEp[i] >= hrMed - 0.5);
        }
        if (isRem)
            refined[i] = Rem;
    }
    const std::vector<int> result = smooth(refined, 5);
    return std::vector<uint8_t>(result.begin(), result.end());
}

} // namespace Oura

#include "breathing.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numeric>

namespace Oura {

namespace {

constexpr double kFs = 4.0;               // resampled rate, Hz
constexpr double kBandLow = 0.15;         // Hz
constexpr double kBandHigh = 0.45;
constexpr int kOrder = 3;
constexpr double kWindowSeconds = 120;
constexpr double kPromisingFraction = 0.3;   // peak prominence, as a share of the window's spread
constexpr double kMinPeakSeconds = 2.0;

struct Biquad
{
    double b[3];
    double a[3];   // a[0] == 1
};

// scipy.signal.butter(3, [lo, hi], 'band', fs=4, output='sos'), written out.
std::vector<Biquad> bandpass()
{
    using C = std::complex<double>;
    const double pi = 3.14159265358979323846;
    const double lo = 2 * kFs * std::tan(pi * kBandLow / kFs);
    const double hi = 2 * kFs * std::tan(pi * kBandHigh / kFs);
    const double bw = hi - lo, wo = std::sqrt(lo * hi);

    std::vector<C> poles;   // analogue, band-pass
    for (int k = 0; k < kOrder; ++k) {
        const double angle = pi * (2 * k + kOrder + 1) / (2.0 * kOrder);
        const C p = std::polar(1.0, angle);
        const C half = p * bw / 2.0;
        const C root = std::sqrt(half * half - wo * wo);
        poles.push_back(half + root);
        poles.push_back(half - root);
    }
    // Bilinear transform.
    std::vector<C> z;
    for (const C &p : poles)
        z.push_back((2 * kFs + p) / (2 * kFs - p));

    // Conjugate pairs, each with the numerator (z - 1)(z + 1) = z^2 - 1.
    std::vector<Biquad> sections;
    std::vector<bool> used(z.size(), false);
    for (size_t i = 0; i < z.size(); ++i) {
        if (used[i])
            continue;
        size_t best = i;
        double bestDistance = 1e9;
        for (size_t j = i + 1; j < z.size(); ++j)
            if (!used[j] && std::abs(z[j] - std::conj(z[i])) < bestDistance) {
                bestDistance = std::abs(z[j] - std::conj(z[i]));
                best = j;
            }
        used[i] = used[best] = true;
        Biquad s;
        s.b[0] = 1;
        s.b[1] = 0;
        s.b[2] = -1;
        s.a[0] = 1;
        s.a[1] = -(z[i] + z[best]).real();
        s.a[2] = (z[i] * z[best]).real();
        sections.push_back(s);
    }
    // Unit gain at the centre of the band.
    const double centre = std::sqrt(kBandLow * kBandHigh);
    const C e = std::polar(1.0, 2 * pi * centre / kFs);
    C response = 1;
    for (const Biquad &s : sections) {
        const C num = s.b[0] + s.b[1] / e + s.b[2] / (e * e);
        const C den = s.a[0] + s.a[1] / e + s.a[2] / (e * e);
        response *= num / den;
    }
    const double gain = 1.0 / std::abs(response);
    sections[0].b[0] *= gain;
    sections[0].b[1] *= gain;
    sections[0].b[2] *= gain;
    return sections;
}

void filterOnce(const std::vector<Biquad> &sections, std::vector<double> &x)
{
    for (const Biquad &s : sections) {
        double z1 = 0, z2 = 0;
        for (double &v : x) {
            const double y = s.b[0] * v + z1;
            z1 = s.b[1] * v - s.a[1] * y + z2;
            z2 = s.b[2] * v - s.a[2] * y;
            v = y;
        }
    }
}

// Forward then backward: no phase shift.
void zeroPhase(const std::vector<Biquad> &sections, std::vector<double> &x)
{
    filterOnce(sections, x);
    std::reverse(x.begin(), x.end());
    filterOnce(sections, x);
    std::reverse(x.begin(), x.end());
}

// scipy.signal.medfilt for an odd window, the edges padded with zeros.
std::vector<double> medianFilter(const std::vector<double> &x, size_t window)
{
    const size_t half = window / 2;
    std::vector<double> out(x.size());
    std::vector<double> w(window);
    for (size_t i = 0; i < x.size(); ++i) {
        for (size_t k = 0; k < window; ++k) {
            const long j = static_cast<long>(i) + static_cast<long>(k) - static_cast<long>(half);
            w[k] = (j < 0 || j >= static_cast<long>(x.size())) ? 0.0 : x[static_cast<size_t>(j)];
        }
        std::nth_element(w.begin(), w.begin() + static_cast<long>(half), w.end());
        out[i] = w[half];
    }
    return out;
}

// scipy.signal.find_peaks(x, distance=minDistance, prominence=minProminence), positions only.
std::vector<size_t> findPeaks(const double *x, size_t n, double minDistance, double minProminence)
{
    std::vector<size_t> peaks;
    for (size_t i = 1; i + 1 < n; ++i) {
        if (x[i - 1] < x[i]) {
            size_t ahead = i + 1;
            while (ahead < n - 1 && x[ahead] == x[i])
                ++ahead;
            if (x[ahead] < x[i]) {
                peaks.push_back((i + ahead - 1) / 2);
                i = ahead;
            }
        }
    }
    // Distance: the highest peaks win.
    std::vector<size_t> order(peaks.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) { return x[peaks[a]] < x[peaks[b]]; });
    std::vector<char> keep(peaks.size(), 1);
    for (size_t r = order.size(); r-- > 0;) {
        const size_t j = order[r];
        if (!keep[j])
            continue;
        for (long k = static_cast<long>(j) - 1; k >= 0 && static_cast<double>(peaks[j] - peaks[static_cast<size_t>(k)]) < minDistance; --k)
            keep[static_cast<size_t>(k)] = 0;
        for (size_t k = j + 1; k < peaks.size() && static_cast<double>(peaks[k] - peaks[j]) < minDistance; ++k)
            keep[k] = 0;
    }
    std::vector<size_t> out;
    for (size_t i = 0; i < peaks.size(); ++i) {
        if (!keep[i])
            continue;
        const size_t p = peaks[i];
        // Prominence: the peak's height over the higher of the two lowest points between it and
        // the next higher ground on either side.
        double leftMin = x[p], rightMin = x[p];
        for (size_t j = p; j-- > 0;) {
            if (x[j] > x[p])
                break;
            leftMin = std::min(leftMin, x[j]);
        }
        for (size_t j = p + 1; j < n; ++j) {
            if (x[j] > x[p])
                break;
            rightMin = std::min(rightMin, x[j]);
        }
        if (x[p] - std::max(leftMin, rightMin) >= minProminence)
            out.push_back(p);
    }
    return out;
}

} // namespace

std::optional<double> breathingRate(const std::vector<BreathBeat> &all, double startUtc, double endUtc)
{
    const long epochs = static_cast<long>(std::floor((endUtc - startUtc) / 30.0));
    if (epochs < 361)
        return std::nullopt;

    // Beats of the window, lengths a person can have, in time order.
    std::vector<BreathBeat> beats;
    for (const BreathBeat &b : all)
        if (b.utcSeconds >= startUtc && b.utcSeconds <= endUtc && b.ibiMs >= 300 && b.ibiMs <= 2000)
            beats.push_back(b);
    std::sort(beats.begin(), beats.end(), [](const BreathBeat &a, const BreathBeat &b) { return a.utcSeconds < b.utcSeconds; });
    if (beats.size() < 600)
        return std::nullopt;

    // Out go beats more than a quarter away from the median of the nine around them.
    std::vector<double> ibi(beats.size());
    for (size_t i = 0; i < beats.size(); ++i)
        ibi[i] = beats[i].ibiMs;
    const std::vector<double> med = medianFilter(ibi, 9);
    std::vector<double> t, v;
    for (size_t i = 0; i < beats.size(); ++i)
        if (std::fabs(ibi[i] - med[i]) < 0.25 * med[i]) {
            t.push_back(beats[i].utcSeconds);
            v.push_back(ibi[i]);
        }
    if (t.size() < 600)
        return std::nullopt;

    // Resampled at 4 Hz by linear interpolation.
    const double t0 = t.front();
    const size_t count = static_cast<size_t>(std::ceil((t.back() - t0) * kFs));
    std::vector<double> x(count);
    size_t j = 0;
    for (size_t i = 0; i < count; ++i) {
        const double at = t0 + static_cast<double>(i) / kFs;
        while (j + 1 < t.size() && t[j + 1] <= at)
            ++j;
        if (j + 1 >= t.size())
            x[i] = v.back();
        else if (at <= t[j])
            x[i] = v[j];
        else {
            const double f = (at - t[j]) / (t[j + 1] - t[j]);
            x[i] = v[j] * (1 - f) + v[j + 1] * f;
        }
    }
    const double mean = std::accumulate(x.begin(), x.end(), 0.0) / static_cast<double>(x.size());
    for (double &s : x)
        s -= mean;
    zeroPhase(bandpass(), x);

    // The epochs of the middle of the night.
    long first, last;
    if (epochs >= 601) {
        first = 120;
        last = epochs - 120;
    } else {
        first = epochs / 2 - 180;
        last = epochs / 2 + 180;
    }
    const long half = static_cast<long>(kWindowSeconds * kFs / 2);
    double sum = 0;
    int used = 0;
    for (long k = first; k < last; ++k) {
        const long centre = static_cast<long>((startUtc + 30.0 * static_cast<double>(k) + 15.0 - t0) * kFs);
        if (centre - half < 0 || centre + half > static_cast<long>(x.size()))
            continue;
        const double *seg = x.data() + (centre - half);
        const size_t n = static_cast<size_t>(2 * half);
        double m = 0;
        for (size_t i = 0; i < n; ++i)
            m += seg[i];
        m /= static_cast<double>(n);
        double var = 0;
        for (size_t i = 0; i < n; ++i)
            var += (seg[i] - m) * (seg[i] - m);
        const double spread = std::sqrt(var / static_cast<double>(n));
        const std::vector<size_t> peaks = findPeaks(seg, n, kMinPeakSeconds * kFs, kPromisingFraction * spread);
        if (peaks.size() < 3)
            continue;
        sum += 60.0 * kFs * static_cast<double>(peaks.size() - 1) / static_cast<double>(peaks.back() - peaks.front());
        ++used;
    }
    if (used < 60)
        return std::nullopt;
    return sum / used;
}

} // namespace Oura

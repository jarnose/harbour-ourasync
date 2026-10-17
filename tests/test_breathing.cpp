// g++ -std=c++17 -Wall -Wextra -I ../src ../src/analysis/breathing.cpp ../src/ring/{protocol,events,aes128}.cpp test_breathing.cpp -o /tmp/br && /tmp/br
#include "hex.h"
#include "analysis/breathing.h"
#include "ring/events.h"

#include <cmath>

using namespace Oura;

namespace {

// The inverse of decodeIbi, to build bodies from known intervals.
Bytes encodeIbi(const int (&ibi)[6])
{
    Bytes p(14, 0);
    for (int i = 0; i < 6; ++i) {
        p[i] = static_cast<uint8_t>(ibi[i] >> 3);
        p[6 + i] = static_cast<uint8_t>(ibi[i] & 1);
    }
    const int mid[6] = { ibi[0] & 6, ibi[1] & 6, ibi[2] & 6, ibi[3] & 6, ibi[4] & 6, ibi[5] & 6 };
    p[12] = static_cast<uint8_t>((mid[0] << 5) | (mid[1] << 3) | (mid[2] << 1) | (mid[3] >> 1));
    p[13] = static_cast<uint8_t>((mid[4] << 5) | (mid[5] << 3));
    return p;
}

// Beats of an eight-hour night whose intervals sway with the breath (respiratory sinus
// arrhythmia) and jitter a little.
std::vector<BreathBeat> night(double start, double breathsPerMinute, double hours)
{
    std::vector<BreathBeat> beats;
    const double f = breathsPerMinute / 60.0;
    double t = start;
    unsigned seed = 12345;
    while (t < start + hours * 3600) {
        seed = seed * 1103515245u + 12345u;
        const double jitter = (static_cast<int>((seed >> 16) & 0x1f) - 16) * 0.8;   // +-13 ms
        const int ibi = static_cast<int>(std::lround(950 + 45 * std::sin(2 * 3.14159265358979 * f * (t - start)) + jitter));
        t += ibi / 1000.0;
        beats.push_back({ t, ibi });
    }
    return beats;
}

} // namespace

int main()
{
    // Layout: round trips through the bit shuffle for the whole plausible range.
    for (int base = 300; base <= 2000; base += 7) {
        const int ibi[6] = { base, base + 1, base + 2, base + 3, base + 4, base + 5 };
        const std::vector<int> back = decodeIbi(encodeIbi(ibi));
        CHECK_EQ(back.size(), size_t(6));
        for (size_t i = 0; i < back.size() && i < 6; ++i)
            CHECK_EQ(back[i], ibi[i]);
    }
    CHECK(decodeIbi(Bytes(13, 0)).empty());
    CHECK(decodeIbi(Bytes(15, 0)).empty());

    // The rate comes back for slow, ordinary and fast breathing.
    const double start = 1760000000;
    for (double rate : { 10.0, 12.0, 15.0, 18.0, 22.0 }) {
        const auto beats = night(start, rate, 8);
        const auto r = breathingRate(beats, start, start + 8 * 3600);
        CHECK(r.has_value());
        if (r)
            CHECK(std::fabs(*r - rate) < 0.6);
    }

    // The same night with its window a different length, and the first and last hour left empty.
    {
        auto beats = night(start, 14, 8);
        const auto r = breathingRate(beats, start, start + 8 * 3600);
        CHECK(r.has_value());
        std::vector<BreathBeat> middle;
        for (const BreathBeat &b : beats)
            if (b.utcSeconds > start + 3600 && b.utcSeconds < start + 7 * 3600)
                middle.push_back(b);
        const auto m = breathingRate(middle, start + 3600, start + 7 * 3600);
        CHECK(m.has_value());
        if (r && m)
            CHECK(std::fabs(*r - *m) < 0.6);
    }

    // Too little to say anything: a nap, a night with holes, no beats.
    CHECK(!breathingRate(night(start, 15, 1), start, start + 3600).has_value());
    CHECK(!breathingRate({}, start, start + 8 * 3600).has_value());
    {
        std::vector<BreathBeat> sparse;
        const auto all = night(start, 15, 8);
        for (size_t i = 0; i < all.size(); i += 100)  // under the 600 beats a night needs
            sparse.push_back(all[i]);
        CHECK(!breathingRate(sparse, start, start + 8 * 3600).has_value());
    }
    return finish("breathing");
}

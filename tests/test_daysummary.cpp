// g++ -std=c++17 -Wall -Wextra -I ../src ../src/ring/aes128.cpp ../src/ring/protocol.cpp ../src/ring/events.cpp ../src/analysis/ringclock.cpp ../src/analysis/sleepscore.cpp ../src/analysis/daysummary.cpp test_daysummary.cpp -o /tmp/daysummary && /tmp/daysummary
//
// A synthetic night, built byte by byte in the layouts open_oura documents, so
// the test carries no real person's sleep.
#include "hex.h"
#include "analysis/daysummary.h"
#include "ring/events.h"

#include <algorithm>
#include <cmath>

using namespace Oura;

namespace {

int64_t g_id = 0;

StoredEvent ev(uint8_t tag, int64_t ds, Bytes body, int64_t captured)
{
    StoredEvent e;
    e.id = ++g_id;
    e.tag = tag;
    e.ds = ds;
    e.body = std::move(body);
    e.capturedUnix = captured;
    return e;
}

Bytes le32(uint32_t v)
{
    return { uint8_t(v), uint8_t(v >> 8), uint8_t(v >> 16), uint8_t(v >> 24) };
}

Bytes cat(Bytes a, const Bytes &b)
{
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

// 2015-01-01 is far enough from 1970 to be a believable clock; use a round one.
const int64_t kT0 = 1760000000; // 2025-10-09 08:53:20 UTC
}

static void clockAndNight()
{
    // Ring counter starts at 100000 ds when the phone tells it kT0.
    const int64_t base = 100000;
    auto at = [&](int64_t utc) { return base + (utc - kT0) * 10; };

    std::vector<StoredEvent> events;
    const int64_t downloaded = kT0 + 86400;

    events.push_back(ev(Tag::TimeSync, base, le32(uint32_t(kT0)), kT0));

    // Sleep from kT0+3600 to kT0+3600+7h.
    const int64_t sleepStart = kT0 + 3600;
    const int64_t sleepEnd = sleepStart + 7 * 3600;
    events.push_back(ev(Tag::BedtimePeriod, at(sleepEnd) + 600, cat(le32(uint32_t(at(sleepStart))), le32(uint32_t(at(sleepEnd)))), downloaded));

    // 5-minute HRV records every 30 minutes: 6 pairs each (hr 52..58, rmssd 40..50).
    for (int64_t t = sleepStart; t < sleepEnd; t += 1800) {
        Bytes body;
        for (int k = 0; k < 6; ++k) {
            body.push_back(uint8_t(52 + (k % 4)));
            body.push_back(uint8_t(40 + k * 2));
        }
        events.push_back(ev(Tag::Hrv, at(t), body, downloaded));
    }

    // Hypnogram: 7 h = 840 epochs of 30 s -> 17 pages of 52 epochs (884 epochs).
    // Pattern per page: first epoch awake, then light/deep/rem blocks.
    const int pages = 17;
    for (int p = 0; p < pages; ++p) {
        Bytes body { uint8_t(p) };
        for (int b = 0; b < 13; ++b) {
            // four epochs per byte, 2 bits each, MSB first
            uint8_t byte = 0;
            for (int k = 0; k < 4; ++k) {
                const int epoch = p * 52 + b * 4 + k;
                uint8_t phase = PhaseLight;
                if (epoch < 30) phase = PhaseAwake;             // 15 min to fall asleep
                else if (epoch < 200) phase = PhaseDeep;
                else if (epoch % 100 < 20) phase = PhaseRem;
                byte = uint8_t((byte << 2) | phase);
            }
            body.push_back(byte);
        }
        // The burst is written when the ring finishes analysing: at sleep end.
        events.push_back(ev(Tag::SleepPhaseData, at(sleepEnd) + 700 + p, body, downloaded));
    }

    // Skin temperature while asleep: 200 samples near 35.4 C.
    for (int i = 0; i < 20; ++i) {
        Bytes body;
        for (int k = 0; k < 10; ++k) {
            const int centi = 3540 + (k % 3) * 5;
            body.push_back(uint8_t(centi & 0xff));
            body.push_back(uint8_t(centi >> 8));
        }
        events.push_back(ev(Tag::SleepTemp, at(sleepStart + 600 * i), body, downloaded));
    }

    // Activity: one hour of walking at 4 MET then an hour of rest, minute bins.
    {
        Bytes walk { 0x01 };
        for (int i = 0; i < 12; ++i)
            walk.push_back(40); // 4.0 MET
        for (int m = 0; m < 5; ++m)
            events.push_back(ev(Tag::ActivityInfo, at(kT0 + 60 * 12 * m), walk, downloaded));
    }

    // Movement: ten consecutive 30-second windows of 10 s of motion, mid-night. The
    // low five bits are the seconds; the top three are orientation.
    for (int k = 0; k < 10; ++k)
        events.push_back(ev(Tag::Motion, at(sleepStart + 4 * 3600 + 30 * k), { uint8_t(0x40 | 10), 0, 0, 0, 0, 0 }, downloaded));

    // Six epochs of heavy movement after a calm stretch: the windowed total doubles past
    // 24 s, which the engine counts as one motion event. The gentler build-up above
    // never rises by 24 once past 24, so it is restlessness but not an event.
    for (int k = 0; k < 6; ++k)
        events.push_back(ev(Tag::Motion, at(sleepStart + 2 * 3600 + 30 * k), { uint8_t(0x40 | 28), 0, 0, 0, 0, 0 }, downloaded));

    // A ring serves its history in time order, and the clock relies on that: a
    // counter that steps back by hours is how it recognises a reset.
    std::stable_sort(events.begin(), events.end(),
                     [](const StoredEvent &a, const StoredEvent &b) { return a.ds < b.ds; });
    int64_t id = 0;
    for (StoredEvent &e : events)
        e.id = ++id;

    AnalysisOptions opt;
    opt.utcOffset = [](int64_t) { return 2 * 3600; };
    Summary s = summarize(events, opt);

    CHECK_EQ(s.nights.size(), size_t(1));
    if (s.nights.empty())
        return;
    const Night &n = s.nights[0];
    CHECK(!n.approximateClock);
    CHECK(std::llabs(n.startUnix - sleepStart) <= 1);
    CHECK(std::llabs(n.endUnix - sleepEnd) <= 1);
    CHECK(std::fabs(n.inBedMin - 420.0) < 0.1);
    CHECK(n.main);

    // The anchor is the time_sync event, so the date is the local day of waking:
    // sleepEnd = kT0 + 3600 + 25200 = 2025-10-09 16:53:20 + 7h... compute below.
    CHECK_EQ(n.date, localDate(sleepEnd, 2 * 3600));

    // Hypnogram painted and consistent.
    CHECK(n.staged);
    CHECK(n.deepMin > 0 && n.remMin > 0 && n.lightMin > 0 && n.awakeMin > 0);
    CHECK(n.latencyMin.has_value());
    CHECK(std::fabs(*n.latencyMin - 15.0) < 0.6);
    CHECK(n.efficiencyPct.has_value());
    CHECK(*n.efficiencyPct > 80 && *n.efficiencyPct <= 100);
    CHECK(std::fabs(n.asleepMin + n.awakeMin - 420.0) < 40.0);

    // The engine's own counts: 30 awake epochs, then asleep to the end.
    CHECK(n.oura.has_value());
    if (n.oura) {
        CHECK(n.ouraMotionKnown);
        CHECK(std::abs(n.oura->latencyEpochs - 30) <= 1);
        CHECK(n.oura->deepEpochs > 0 && n.oura->remEpochs > 0 && n.oura->lightEpochs > 0);
        // Ten epochs of motion and six more, each windowed over four.
        CHECK(n.oura->restlessEpochs >= 20 && n.oura->restlessEpochs <= 25);
        CHECK_EQ(n.oura->motionEvents, 1);
        // 02:53 local + 2 h offset + 1 h: the night starts at 11:53 UTC+2 -> wrapped time of day.
        const int64_t startLocal = ((sleepStart + 2 * 3600) % 86400) / 60;
        CHECK(n.oura->midpointMinutes > startLocal - 1440 && n.oura->midpointMinutes < startLocal + 1440);
    }

    // Physiology.
    CHECK(n.restingHr.has_value());
    CHECK(std::fabs(*n.restingHr - 52.0) < 0.1);
    CHECK(n.avgHrv.has_value());
    CHECK(*n.avgHrv > 40 && *n.avgHrv < 50);
    CHECK(n.skinTemp.has_value());
    CHECK(*n.skinTemp > 35.3 && *n.skinTemp < 35.6);
    CHECK(!n.tempDeviation.has_value()); // no baseline from a single night

    // A score exists and sits in a believable range for a 7 h, efficient night.
    CHECK(n.score.valid);
    CHECK(n.score.score >= 60 && n.score.score <= 100);
    double weights = 0;
    for (const auto &c : n.score.components)
        weights += c.weight;
    CHECK(std::fabs(weights - 1.0) < 1e-9);

    // Activity: 5 events x 12 minutes at 4 MET = 60 moderate minutes, each
    // (4-1)*75/60 = 3.75 kcal -> 225.
    bool foundDay = false;
    for (const Day &d : s.days) {
        if (d.moderateMin == 60) {
            foundDay = true;
            CHECK_EQ(d.activeKcal, 225.0);
            CHECK_EQ(d.vigorousMin, 0);
            CHECK(d.avgMet.has_value());
            CHECK(std::fabs(*d.avgMet - 4.0) < 1e-9);
        }
    }
    CHECK(foundDay);
}

static void ringWithoutHypnogram()
{
    // A Ring 4 sends the night's window and its heart rate, HRV and movement but no stages:
    // they are estimated, and said to be.
    const int64_t base = 100000;
    auto at = [&](int64_t utc) { return base + (utc - kT0) * 10; };
    std::vector<StoredEvent> events;
    const int64_t downloaded = kT0 + 86400;
    events.push_back(ev(Tag::TimeSync, base, le32(uint32_t(kT0)), kT0));
    const int64_t sleepStart = kT0 + 3600, sleepEnd = sleepStart + 7 * 3600;
    events.push_back(ev(Tag::BedtimePeriod, at(sleepEnd) + 600,
                        cat(le32(uint32_t(at(sleepStart))), le32(uint32_t(at(sleepEnd)))), downloaded));
    for (int64_t t = sleepStart; t < sleepEnd; t += 1800) {
        Bytes body;
        for (int k = 0; k < 6; ++k) {
            body.push_back(uint8_t(52 + (k % 4) + (t > sleepEnd - 3600 ? 6 : 0)));
            body.push_back(uint8_t(30 + int((t - sleepStart) / 900)));
        }
        events.push_back(ev(Tag::Hrv, at(t), body, downloaded));
    }
    for (int64_t t = sleepStart; t < sleepStart + 600; t += 30)
        events.push_back(ev(Tag::Motion, at(t), { uint8_t(0x40 | 12), 0, 0, 0 }, downloaded));
    std::stable_sort(events.begin(), events.end(), [](const StoredEvent &a, const StoredEvent &b) { return a.ds < b.ds; });
    int64_t id = 0;
    for (StoredEvent &e : events)
        e.id = ++id;

    AnalysisOptions opt;
    opt.utcOffset = [](int64_t) { return 0; };
    const Summary s = summarize(events, opt);
    CHECK_EQ(s.nights.size(), size_t(1));
    if (s.nights.empty())
        return;
    const Night &n = s.nights[0];
    CHECK(n.staged);
    CHECK(n.stagesEstimated);
    CHECK(n.asleepMin > 300 && n.asleepMin < 430);
    CHECK(n.oura.has_value());
}

static void unanchoredFallsBackToDownloadTime()
{
    // No time_sync at all: the epoch is placed from the download time and the
    // result is flagged approximate.
    const int64_t downloaded = kT0;
    std::vector<StoredEvent> events;
    const int64_t end = 500000;
    events.push_back(ev(Tag::BedtimePeriod, end + 100, cat(le32(uint32_t(end - 72000)), le32(uint32_t(end))), downloaded));
    AnalysisOptions opt;
    Summary s = summarize(events, opt);
    CHECK_EQ(s.nights.size(), size_t(1));
    if (!s.nights.empty()) {
        CHECK(s.nights[0].approximateClock);
        CHECK(std::fabs(s.nights[0].inBedMin - 120.0) < 0.1);
        CHECK(!s.nights[0].staged);
        CHECK(s.nights[0].asleepEstimated);
    }
}

static void counterResetStartsANewEpoch()
{
    std::vector<StoredEvent> events;
    events.push_back(ev(Tag::TimeSync, 5000000, le32(uint32_t(kT0)), kT0));
    events.push_back(ev(Tag::DebugEvent, 5001000, { 1 }, kT0));
    // The counter falls back to near zero: a reset. Its own time record follows.
    events.push_back(ev(Tag::TimeSync, 200, le32(uint32_t(kT0 + 86400)), kT0 + 86400));
    events.push_back(ev(Tag::DebugEvent, 1200, { 1 }, kT0 + 86400));
    RingClock clock(events);
    CHECK_EQ(clock.epochCount(), 2);
    CHECK(clock.anchored(0) && clock.anchored(1));
    CHECK(std::fabs(clock.utc(clock.epochOf(1), 5001000) - (kT0 + 100.0)) < 1e-6);
    CHECK(std::fabs(clock.utc(clock.epochOf(3), 1200) - (kT0 + 86400 + 100.0)) < 1e-6);
}

static void temperature()
{
    // 150 steady samples -> five windows, all 35.00.
    std::vector<double> steady(150, 35.0);
    auto t = nightlyTemperature(steady);
    CHECK(t.has_value());
    CHECK(std::fabs(*t - 35.0) < 1e-9);
    // Too few windows.
    CHECK(!nightlyTemperature(std::vector<double>(60, 35.0)).has_value());
}

static void dates()
{
    CHECK_EQ(localDate(0, 0), std::string("1970-01-01"));
    CHECK_EQ(localDate(86399, 0), std::string("1970-01-01"));
    CHECK_EQ(localDate(86400, 0), std::string("1970-01-02"));
    // Local midnight moves with the offset, and negative offsets cross back.
    CHECK_EQ(localDate(0, 2 * 3600), std::string("1970-01-01"));
    CHECK_EQ(localDate(-1, 0), std::string("1969-12-31"));
    CHECK_EQ(localDate(1760000000, 0), std::string("2025-10-09"));
    CHECK_EQ(daysFromCivil(1970, 1, 1), int64_t(0));
    CHECK_EQ(daysFromCivil(2000, 3, 1), int64_t(11017));
}

static void scoring()
{
    NightInput textbook;
    textbook.asleepMin = 8 * 60;
    textbook.efficiencyPct = 92;
    textbook.onsetLatencyMin = 12;
    textbook.wasoMin = 15;
    textbook.awakenings = 1;
    textbook.deepPct = 18;
    textbook.remPct = 22;
    CHECK(scoreNight(textbook).score >= 95);

    NightInput poor;
    poor.asleepMin = 4 * 60;
    poor.efficiencyPct = 66;
    poor.onsetLatencyMin = 55;
    poor.wasoMin = 90;
    poor.awakenings = 12;
    poor.deepPct = 5;
    poor.remPct = 8;
    CHECK(scoreNight(poor).score <= 40);

    // A night without a hypnogram is not marked down for lacking one.
    NightInput full;
    full.asleepMin = 8 * 60;
    full.efficiencyPct = 90;
    full.deepPct = 18;
    full.remPct = 22;
    NightInput partial;
    partial.asleepMin = 8 * 60;
    partial.efficiencyPct = 90;
    CHECK(std::abs(scoreNight(full).score - scoreNight(partial).score) < 6);

    // Duration bands follow age: 9.5 h is fine at 40 and long at 70.
    NightInput a;
    a.asleepMin = 9.5 * 60;
    a.age = 40;
    NightInput b = a;
    b.age = 70;
    CHECK(scoreNight(a).score > scoreNight(b).score);

    CHECK(!scoreNight(NightInput()).valid);
    CHECK(!causalBaseline({ 1, 2 }).has_value());
    CHECK(causalBaseline({ 50, 52, 54 }).has_value());
}

static void motionPeriodDecoding()
{
    // Header: type 0, three codes in the last byte, counter 5; codes 0 1 2 3 | 2 1 0 (+ padding).
    auto m = decodeMotionPeriod({ 0x35, 0x1b, 0x90 });
    CHECK(m.has_value());
    if (m) {
        CHECK_EQ(m->periodType, 0);
        CHECK_EQ(m->sequence, 5);
        CHECK_EQ(m->states.size(), size_t(7));
        const std::vector<uint8_t> expect = { 0, 1, 2, 3, 2, 1, 0 };
        CHECK(m->states == expect);
    }
    // A count of 0 means the last byte is full.
    auto full = decodeMotionPeriod({ 0x05, 0xe4 });
    CHECK(full.has_value() && full->states.size() == size_t(4) && full->states[0] == 3);
    CHECK(!decodeMotionPeriod({ 0x35 }));
}

static void motionDecoding()
{
    CHECK_EQ(*decodeMotionSeconds({ 0x4a, 0, 0, 0, 0, 0 }), 10);   // orientation bits dropped
    CHECK_EQ(*decodeMotionSeconds({ 0x1f, 0, 0, 0 }), 31);
    CHECK(!decodeMotionSeconds({ 0x05, 0, 0 }));                    // too short
    CHECK(!decodeMotionSeconds({ 0x05, 0, 0, 0, 0x40, 0 }));        // the library rejects bit 0x40 here
    CHECK(!decodeMotionSeconds({ 0x05, 0, 0, 0, 0, 0x40 }));
}

int main()
{
    motionDecoding();
    motionPeriodDecoding();
    clockAndNight();
    ringWithoutHypnogram();
    unanchoredFallsBackToDownloadTime();
    counterResetStartsANewEpoch();
    temperature();
    dates();
    scoring();
    return finish("daysummary");
}

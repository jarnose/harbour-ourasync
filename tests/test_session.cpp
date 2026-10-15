// g++ -std=c++17 -Wall -Wextra -I ../src ../src/ring/aes128.cpp ../src/ring/protocol.cpp ../src/ring/events.cpp ../src/ring/session.cpp test_session.cpp -o /tmp/session && /tmp/session
//
// The conversation logic against a scripted ring: no Bluetooth, no Qt.
#include "hex.h"
#include "ring/session.h"

#include <algorithm>
#include <functional>
#include <map>

using namespace Oura;

// Maps a request (hex) to the notifications the ring sends back, or, for
// requests that need state, to a generator.
class MockRing : public Transport
{
public:
    std::map<std::string, std::vector<std::string>> script;
    std::function<std::vector<std::string>(const Bytes &)> fallback;
    std::vector<std::string> writes;
    bool linkUp = true;

    bool transact(const Bytes &request, const std::function<bool(const Packet &)> &terminal, int,
                  std::vector<Packet> &out, std::string &error) override
    {
        if (!linkUp) {
            error = "link down";
            return false;
        }
        writes.push_back(toHex(request));
        std::vector<std::string> frames;
        auto it = script.find(toHex(request));
        if (it != script.end())
            frames = it->second;
        else if (fallback)
            frames = fallback(request);
        for (const std::string &f : frames) {
            bool done = false;
            for (Packet &p : parsePackets(fromHex(f))) {
                done = done || terminal(p);
                out.push_back(std::move(p));
            }
            if (done)
                break;
        }
        return true;
    }
};

static AesKey keyOf(const std::string &hex)
{
    AesKey k;
    auto v = fromHex(hex);
    for (size_t i = 0; i < 16; ++i)
        k[i] = v[i];
    return k;
}

static void authentication()
{
    // The open_oura known-answer nonce/ciphertext pair.
    const AesKey key = keyOf("4431967d8bacc2659743142b68391d9a");
    MockRing ring;
    ring.script["2f012b"] = { "2f102c0e2d6a0a08c99b4365f458e6e97382" };
    ring.script["2f112da38a8772d3acb6db5c2b516dd56987c8"] = { "2f022e00" };

    RingSession session(ring);
    AuthResult result = AuthResult::Unknown;
    Outcome o = session.authenticate(key, &result);
    CHECK(o.ok);
    CHECK(result == AuthResult::Success);

    // Wrong key: the ring answers 2f022e01 to whatever block it is sent.
    MockRing wrong;
    wrong.script["2f012b"] = { "2f102c0e2d6a0a08c99b4365f458e6e97382" };
    wrong.fallback = [](const Bytes &r) -> std::vector<std::string> {
        return r.size() > 2 && r[2] == 0x2d ? std::vector<std::string> { "2f022e01" }
                                            : std::vector<std::string>();
    };
    RingSession s2(wrong);
    o = s2.authenticate(keyOf("00000000000000000000000000000000"), &result);
    CHECK(!o.ok);
    CHECK(result == AuthResult::AuthenticationError);

    // No nonce at all: the error names the likely cause.
    MockRing silent;
    RingSession s3(silent);
    o = s3.authenticate(key);
    CHECK(!o.ok);
    CHECK(o.error.find("paired") != std::string::npos);

    // A dead link surfaces as a failure rather than a hang.
    MockRing dead;
    dead.linkUp = false;
    RingSession s4(dead);
    CHECK(!s4.authenticate(key).ok);
}

static void keyInstall()
{
    const AesKey key = keyOf("000102030405060708090a0b0c0d0e0f");
    MockRing ring;
    ring.script["2410000102030405060708090a0b0c0d0e0f"] = { "250100" };
    RingSession ok(ring);
    CHECK(ok.setAuthKey(key).ok);

    MockRing refuse;
    refuse.script["2410000102030405060708090a0b0c0d0e0f"] = { "250105" };
    RingSession no(refuse);
    Outcome o = no.setAuthKey(key);
    CHECK(!o.ok);
    CHECK(o.error.find("factory reset") != std::string::npos);
}

static void factoryReset()
{
    // The ring drops the link before it answers: no answer is a success.
    MockRing quiet;
    quiet.script["1a00"] = {};
    RingSession a(quiet);
    CHECK(a.factoryReset().ok);

    MockRing answered;
    answered.script["1a00"] = { "1b0200" };
    CHECK(RingSession(answered).factoryReset().ok);

    MockRing refused;
    refused.script["1a00"] = { "1b0201" };
    CHECK(!RingSession(refused).factoryReset().ok);

    MockRing dead;
    dead.linkUp = false;
    CHECK(!RingSession(dead).factoryReset().ok);
}

static void reads()
{
    MockRing ring;
    ring.script["0c00"] = { "0d0659000001f00f" };
    ring.script["1803080010"] = { "19110058585858585858585858585858580000" };
    RingSession s(ring);
    Battery b;
    CHECK(s.battery(b).ok);
    CHECK_EQ(b.percent, 0x59);
    std::string serial;
    CHECK(s.serial(serial).ok);
    CHECK_EQ(serial, std::string("XXXXXXXXXXXXXX"));

    // Not authenticated: 2f022f01 instead of a battery frame.
    MockRing locked;
    locked.script["0c00"] = { "2f022f01" };
    RingSession s2(locked);
    CHECK(!s2.battery(b).ok);
}

// A ring holding `total` events, one per decisecond from `first`, served in
// pages of `page` events per extended request.
struct HistoryRing
{
    uint32_t first = 1000;
    uint32_t total = 10;
    uint32_t page = 4;
    bool omitSummary = false;
    int requests = 0;

    std::vector<std::string> serve(const Bytes &r)
    {
        std::vector<std::string> frames;
        if (r.size() < 3)
            return frames;
        if (r[0] == 0x28)
            return { "290100" };
        if (r[0] != 0x2f || r[2] != 0x41)
            return frames;
        ++requests;
        uint64_t startMs = 0;
        for (int i = 7; i >= 0; --i)
            startMs = (startMs << 8) | r[4 + i];
        const uint32_t startDs = static_cast<uint32_t>(startMs / 100);

        // Collect the events at or after the cursor.
        std::vector<uint32_t> stamps;
        for (uint32_t t = first; t < first + total && stamps.size() < page; ++t)
            if (t >= startDs)
                stamps.push_back(t);

        // One envelope holding all of them: control, tag 0x43, len, varint ts, body.
        // First timestamp is absolute ms, the rest deltas of 100 ms.
        Bytes envelope;
        uint64_t prev = 0;
        for (size_t i = 0; i < stamps.size(); ++i) {
            const uint64_t ms = static_cast<uint64_t>(stamps[i]) * 100;
            const uint64_t value = i == 0 ? ms : ms - prev;
            prev = ms;
            Bytes varint;
            uint64_t v = value;
            do {
                uint8_t b = v & 0x7f;
                v >>= 7;
                if (v)
                    b |= 0x80;
                varint.push_back(b);
            } while (v);
            envelope.push_back(0xaa);
            envelope.push_back(0x43);
            envelope.push_back(static_cast<uint8_t>(varint.size() + 1));
            envelope.insert(envelope.end(), varint.begin(), varint.end());
            envelope.push_back(0x77);
        }
        if (!stamps.empty()) {
            Bytes chunk { 0x2f, 0, 0x43 };
            chunk.push_back(static_cast<uint8_t>(envelope.size() & 0xff));
            chunk.push_back(static_cast<uint8_t>(envelope.size() >> 8));
            chunk.insert(chunk.end(), envelope.begin(), envelope.end());
            chunk[1] = static_cast<uint8_t>(chunk.size() - 2);
            frames.push_back(toHex(chunk));
        }
        const uint32_t served = stamps.empty() ? std::max(startDs, first) : stamps.back() + 1;
        const uint32_t left = served >= first + total ? 0 : (first + total - served) * 10;
        if (!omitSummary) {
            Bytes conf { 0x2f, 0x0a, 0x42, static_cast<uint8_t>(stamps.size()), 0, 0 };
            for (int i = 0; i < 4; ++i)
                conf.push_back(static_cast<uint8_t>((left >> (8 * i)) & 0xff));
            conf.push_back(0);
            conf.push_back(0);
            frames.push_back(toHex(conf));
        }
        return frames;
    }
};

static void draining()
{
    // 10 events in pages of 4: three batches, resuming from each cursor.
    {
        HistoryRing h;
        MockRing ring;
        ring.fallback = [&](const Bytes &r) { return h.serve(r); };
        RingSession s(ring);
        std::vector<uint32_t> seen;
        std::vector<uint32_t> cursors;
        DrainResult r = s.drain(0, [&](const std::vector<RingEvent> &events, const SyncProgress &p) {
            for (const RingEvent &e : events)
                seen.push_back(e.timestamp);
            cursors.push_back(p.cursor);
            return true;
        });
        CHECK(r.status.ok);
        CHECK_EQ(r.eventsSynced, uint32_t(10));
        CHECK_EQ(seen.size(), size_t(10));
        CHECK_EQ(seen.front(), uint32_t(1000));
        CHECK_EQ(seen.back(), uint32_t(1009));
        CHECK_EQ(r.nextCursor, uint32_t(1010));
        CHECK(cursors.size() >= 3);
    }
    // Resuming from a stored cursor fetches only the remainder.
    {
        HistoryRing h;
        MockRing ring;
        ring.fallback = [&](const Bytes &r) { return h.serve(r); };
        RingSession s(ring);
        uint32_t count = 0;
        DrainResult r = s.drain(1008, [&](const std::vector<RingEvent> &events, const SyncProgress &) {
            count += static_cast<uint32_t>(events.size());
            return true;
        });
        CHECK(r.status.ok);
        CHECK_EQ(count, uint32_t(2));
    }
    // A cursor past everything: one empty pass, then done, nothing replayed.
    {
        HistoryRing h;
        MockRing ring;
        ring.fallback = [&](const Bytes &r) { return h.serve(r); };
        RingSession s(ring);
        uint32_t count = 0;
        DrainResult r = s.drain(5000, [&](const std::vector<RingEvent> &events, const SyncProgress &) {
            count += static_cast<uint32_t>(events.size());
            return true;
        });
        CHECK(r.status.ok);
        CHECK_EQ(count, uint32_t(0));
        CHECK_EQ(r.nextCursor, uint32_t(5000));
        CHECK_EQ(h.requests, 1);
    }
    // A batch with no summary is a lost link, never "drained".
    {
        HistoryRing h;
        h.omitSummary = true;
        MockRing ring;
        ring.fallback = [&](const Bytes &r) { return h.serve(r); };
        RingSession s(ring);
        DrainResult r = s.drain(0, [](const std::vector<RingEvent> &, const SyncProgress &) { return true; });
        CHECK(!r.status.ok);
        CHECK(r.status.error.find("summary") != std::string::npos);
    }
    // The callback can abort, which is how a failed database write stops the sync
    // without acknowledging events that were not stored.
    {
        HistoryRing h;
        MockRing ring;
        ring.fallback = [&](const Bytes &r) { return h.serve(r); };
        RingSession s(ring);
        DrainResult r = s.drain(0, [](const std::vector<RingEvent> &, const SyncProgress &) { return false; });
        CHECK(!r.status.ok);
    }
}

static void legacyFallback()
{
    // A ring that does not know the extended request answers ext 0x00, and the
    // session falls back to the legacy 0x10 fetch.
    MockRing ring;
    ring.script["280100"] = { "290100" };
    ring.fallback = [](const Bytes &r) -> std::vector<std::string> {
        if (r.size() > 2 && r[0] == 0x2f && r[2] == 0x41)
            return { "2f020046" };
        if (r[0] == 0x10) {
            // start = 0: one event at ts 100, then the summary with nothing left.
            return { "43086400000074657374", "1106010000000000" };
        }
        return {};
    };
    RingSession s(ring);
    uint32_t count = 0;
    DrainResult r = s.drain(0, [&](const std::vector<RingEvent> &events, const SyncProgress &) {
        count += static_cast<uint32_t>(events.size());
        return true;
    });
    // The second pass starts at 101 and the mock repeats ts 100: a replay, so it ends.
    CHECK(r.status.ok);
    CHECK_EQ(count, uint32_t(1));
}

static void featureSwitch()
{
    MockRing ring;
    ring.script["2f02200b"] = { "2f06210b00000000" };
    ring.script["2f03220b01"] = { "2f03230b00" };
    RingSession s(ring);
    uint8_t mode = 9;
    CHECK(s.featureMode(0x0b, mode).ok);
    CHECK_EQ(int(mode), 0);
    CHECK(s.setFeatureMode(0x0b, 1).ok);

    MockRing declining;
    declining.script["2f03220b01"] = { "2f03230b01" };
    RingSession d(declining);
    CHECK(!d.setFeatureMode(0x0b, 1).ok);
}

int main()
{
    authentication();
    keyInstall();
    factoryReset();
    reads();
    draining();
    legacyFallback();
    featureSwitch();
    return finish("session");
}

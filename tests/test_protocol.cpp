// g++ -std=c++17 -Wall -Wextra -I ../src ../src/ring/aes128.cpp ../src/ring/protocol.cpp ../src/ring/events.cpp test_protocol.cpp -o /tmp/protocol && /tmp/protocol
//
// Vectors are the ones open_oura captured from a Ring 3 Horizon and a Ring 5
// (its protocol cheatsheet and unit tests), plus the request shapes re-read in
// the Oura Android app 7.27.0 bytecode. None is a capture from the author's
// ring, so the file runs anywhere.
#include "hex.h"
#include "ring/events.h"
#include "ring/protocol.h"

using namespace Oura;

static Packet packet(const std::string &hex)
{
    auto v = parsePackets(fromHex(hex));
    return v.at(0);
}

static void requests()
{
    CHECK_EQ(toHex(req::firmware()), std::string("0803000000"));
    CHECK_EQ(toHex(req::authNonce()), std::string("2f012b"));
    CHECK_EQ(toHex(req::getEvent(0, 8, -1)), std::string("10090000000008ffffffff"));
    CHECK_EQ(toHex(req::extGetEvent(123400, 0xffff, 0)), std::string("2f0c410008e2010000000000ffff"));
    CHECK_EQ(toHex(req::dataFlush()), std::string("280100"));
    CHECK_EQ(toHex(req::streamSubscribe(2)), std::string("160102"));
    CHECK_EQ(toHex(req::eventSubscribe(0x14, 0x1000)), std::string("1803140010"));
    CHECK_EQ(toHex(req::paramRead(0x02)), std::string("2f022002"));
    CHECK_EQ(toHex(req::setFeatureMode(0x02, 0x03)), std::string("2f03220203"));
    CHECK_EQ(toHex(req::realtimeOff()), std::string("060400000000"));
    CHECK_EQ(toHex(req::battery()), std::string("0c00"));

    // SyncTime as Android 7.27.0 builds it: 12 09 <u64 LE seconds> <tz half-hours>.
    // 1_700_000_000 = 0x6553F100; UTC+2 is four half hours.
    CHECK_EQ(toHex(req::syncTime(1700000000ULL, 4)), std::string("120900f15365000000000" "4"));

    AesKey key{};
    for (size_t i = 0; i < 16; ++i)
        key[i] = static_cast<uint8_t>(i);
    CHECK_EQ(toHex(req::setAuthKey(key)), std::string("241000" "0102030405060708090a0b0c0d0e0f"));
    CHECK_EQ(toHex(req::factoryReset()), std::string("1a00"));
    AesBlock enc{};
    CHECK_EQ(toHex(req::authenticate(enc)), std::string("2f112d00000000000000000000000000000000"));
}

static void framing()
{
    // Two packets in one notification (Ring 5 packs events like this).
    auto both = parsePackets(fromHex("0d0639000000360f290100"));
    CHECK_EQ(both.size(), size_t(2));
    CHECK_EQ(both[0].tag, uint8_t(0x0d));
    CHECK_EQ(both[1].tag, uint8_t(0x29));

    // Declared length longer than the buffer: the lenient path keeps the rest.
    auto padded = parsePackets(fromHex("250100"));
    CHECK_EQ(padded.size(), size_t(1));
    CHECK_EQ(padded[0].payload.size(), size_t(1));

    // A trailing fragment after a complete frame is ignored, not mis-parsed.
    auto frag = parsePackets(fromHex("2501000c"));
    CHECK_EQ(frag.size(), size_t(1));
}

static void responses()
{
    auto info = parseDeviceInfo(packet("091202000003040301000105000cffeeddccbbaa"));
    CHECK(info.has_value());
    CHECK_EQ(info->apiVersion, std::string("2.0.0"));
    CHECK_EQ(info->firmwareVersion, std::string("3.4.3"));
    CHECK_EQ(info->mac, std::string("aa:bb:cc:dd:ee:ff"));

    auto battery = parseBattery(packet("0d0659000001f00f"));
    CHECK(battery.has_value());
    CHECK_EQ(battery->percent, 0x59);

    auto serial = parseProductAscii(packet("191100585858585858585858585858"));
    CHECK(serial.has_value());
    CHECK_EQ(*serial, std::string("XXXXXXXXXXXX"));

    CHECK(authResultFromByte(0) == AuthResult::Success);
    CHECK(authResultFromByte(1) == AuthResult::AuthenticationError);
    CHECK(authResultFromByte(2) == AuthResult::InFactoryReset);
}

static void history()
{
    // Legacy: one debug event at ts 100, then the 0x11 summary.
    {
        auto b = decodeBatch({ packet("43086400000074657374"), packet("1106000000000000") });
        CHECK(b.haveSummary);
        CHECK_EQ(b.events.size(), size_t(1));
        CHECK_EQ(b.events[0].timestamp, uint32_t(100));
        CHECK_EQ(b.bytesLeft, uint32_t(0));
    }
    // Extended: one bundled event, then the 0x42 confirmation.
    {
        auto b = decodeBatch({ packet("2f09430600aa430364bbcc"), packet("2f0a42010000000000000000") });
        CHECK(b.haveSummary);
        CHECK_EQ(b.events.size(), size_t(1));
        CHECK_EQ(b.events[0].timestamp, uint32_t(1));
        CHECK_EQ(toHex(b.events[0].body), std::string("bbcc"));
        CHECK_EQ(b.bytesLeft, uint32_t(0));
        CHECK_EQ(b.resultCode, 0);
    }
    // No summary: the link died mid-batch.
    {
        auto b = decodeBatch({ packet("43086400000074657374") });
        CHECK(!b.haveSummary);
    }
    // iOS sync terminal frame with result 0xff: a rejected request.
    {
        auto b = decodeBatch({ packet("2f0a420000000000000000ff") });
        CHECK(b.haveSummary);
        CHECK_EQ(b.resultCode, 0xff);
    }
    // An envelope split across two notifications reassembles.
    {
        ExtEventEnvelopeParser parser;
        auto first = parser.push(packet("2f05430600aa43"));
        CHECK(first.empty());
        auto second = parser.push(packet("2f054303" "64bbcc"));
        CHECK_EQ(second.size(), size_t(1));
    }
}

static HistoryBatch batchOf(std::initializer_list<uint32_t> stamps, uint32_t bytesLeft = 0)
{
    HistoryBatch b;
    b.haveSummary = true;
    b.bytesLeft = bytesLeft;
    for (uint32_t t : stamps) {
        RingEvent e;
        e.tag = 0x43;
        e.timestamp = t;
        b.events.push_back(e);
    }
    return b;
}

static void validation()
{
    // Horizon fw 3.4.3 answers a cursor past its newest event with its last few
    // events again; that must not count as progress.
    {
        auto v = validateBatch(batchOf({ 7756750, 7756751, 7756752 }), 7756757);
        CHECK(v.events.empty());
        CHECK_EQ(v.replayed, uint32_t(3));
        CHECK_EQ(v.nextCursor, uint32_t(7756757));
        CHECK(!v.progressed(7756757));
    }
    // New events mixed with a replayed tail.
    {
        auto v = validateBatch(batchOf({ 99, 100, 105 }), 100);
        CHECK_EQ(v.events.size(), size_t(2));
        CHECK_EQ(v.replayed, uint32_t(1));
        CHECK_EQ(v.nextCursor, uint32_t(106));
        CHECK(v.progressed(100));
    }
    // The Ring 5 corrupt-envelope vector: one good event, six impossible jumps.
    {
        auto v = validateBatch(batchOf({ 6906620, 2390248269u, 3970646416u, 3970646462u, 3970646516u,
                                         3970646537u, 3970646538u }),
                               6906561);
        CHECK_EQ(v.events.size(), size_t(1));
        CHECK_EQ(v.rejected, uint32_t(6));
        CHECK_EQ(v.nextCursor, uint32_t(6906621));
    }
}

static void decoders()
{
    // hrv_event: (hr, rmssd) pairs.
    {
        auto h = decodeHrv(fromHex("3c2a3b30"));
        CHECK(h.has_value());
        CHECK_EQ(h->hr.size(), size_t(2));
        CHECK_EQ(h->hr[0], 0x3c);
        CHECK_EQ(h->rmssd[1], 0x30);
        CHECK(!decodeHrv(fromHex("3c2a3b")).has_value());
    }
    // temperatures: 3512 centi-degrees = 35.12 C; implausible rejects the lot.
    {
        auto t = decodeTemperatures(fromHex("b80dc00d"));
        CHECK_EQ(t.size(), size_t(2));
        CHECK(t[0] > 35.11 && t[0] < 35.13);
        CHECK(decodeTemperatures(fromHex("1027")).empty()); // 100.00 C
    }
    // green ibi: b0=0x64 gives 100<<3 = 800 ms, b1 contributes the low three
    // bits and the 2-bit quality in bits 3-4.
    {
        // b1 = 0x0d: low 3 bits 5, bits 3-4 = 1 -> quality 1.
        auto beats = decodeGreenIbi(fromHex("640d"));
        CHECK_EQ(beats[0].ibiMs, 805);
        CHECK_EQ(beats[0].quality, 1);
        CHECK(beats[0].good());
        CHECK_EQ(beats[0].bpm(), 74);
    }
    // sleep phases: 0x1b = 00 01 10 11 = deep light rem awake.
    {
        auto page = decodeSleepPhasePage(fromHex("031b"));
        CHECK(page.has_value());
        CHECK_EQ(page->page, 3);
        CHECK_EQ(page->phases.size(), size_t(4));
        CHECK_EQ(page->phases[0], uint8_t(PhaseDeep));
        CHECK_EQ(page->phases[1], uint8_t(PhaseLight));
        CHECK_EQ(page->phases[2], uint8_t(PhaseRem));
        CHECK_EQ(page->phases[3], uint8_t(PhaseAwake));
    }
    // bedtime_period.
    {
        auto b = decodeBedtime(fromHex("10270000" "20bf0100"));
        CHECK(b.has_value());
        CHECK_EQ(b->startDs, uint32_t(10000));
        CHECK_EQ(b->endDs, uint32_t(114464));
    }
    // sleep_period_information_2: hr 0x74*0.5 = 58, breath 0x80/8 = 16.
    {
        auto p = decodeSleepPeriod(fromHex("74000000" "80000501" "0000"));
        CHECK(p.has_value());
        CHECK(p->averageHr > 57.9 && p->averageHr < 58.1);
        CHECK(p->breath > 15.9 && p->breath < 16.1);
        CHECK_EQ(p->sleepState, 1);
        // Out-of-range sleep state is dropped, as the native parser throws.
        CHECK(!decodeSleepPeriod(fromHex("74000000" "80000503" "0000")).has_value());
    }
    // activity MET: 0x0a -> 1.0, 0x80 -> 12.8, 0x82 -> 13.2.
    {
        auto a = decodeActivityMet(fromHex("010a8082"));
        CHECK(a.has_value());
        CHECK_EQ(a->met.size(), size_t(3));
        CHECK(a->met[0] > 0.99 && a->met[0] < 1.01);
        CHECK(a->met[1] > 12.79 && a->met[1] < 12.81);
        CHECK(a->met[2] > 13.19 && a->met[2] < 13.21);
    }
    // spo2: header then percentages, trailing ff dropped.
    {
        auto s = decodeSpo2(fromHex("00605f5eff"));
        CHECK_EQ(s.size(), size_t(3));
        CHECK_EQ(s[0], 96);
    }
    // time_sync and rtc_beacon read the u32 unix time.
    {
        auto t = decodeTimeSync(fromHex("00f15365"));
        CHECK(t.has_value());
        CHECK_EQ(*t, uint32_t(1700000000));
        CHECK(decodeRtcBeacon(fromHex("00f1536500000000" "0102")).has_value());
        CHECK(!decodeRtcBeacon(fromHex("00f15365")).has_value());
    }
    // battery log: 92 %, 4280 mV (the range open_oura validated).
    {
        auto b = decodeBatteryLog(fromHex("245cb810"));
        CHECK(b.has_value());
        CHECK_EQ(b->percent, 92);
        CHECK_EQ(b->millivolts, 4280);
    }
    // SpO2 calibration: R=0.5 -> 105.2 - 2.55 - 3.35 = 99.3.
    {
        const double p = spo2PercentFromR(0.5);
        CHECK(p > 99.2 && p < 99.4);
        CHECK(spo2PercentFromR(2.0) >= 85.0);
    }
}

int main()
{
    requests();
    framing();
    responses();
    history();
    validation();
    decoders();
    return finish("protocol");
}

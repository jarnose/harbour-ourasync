#include "session.h"

#include <algorithm>

namespace Oura {

namespace {

const Packet *findTag(const std::vector<Packet> &packets, uint8_t tag)
{
    for (const Packet &p : packets)
        if (p.tag == tag)
            return &p;
    return nullptr;
}

const Packet *findExt(const std::vector<Packet> &packets, int ext)
{
    for (const Packet &p : packets)
        if (p.extTag() == ext)
            return &p;
    return nullptr;
}

std::string hexOf(const std::vector<Packet> &packets)
{
    static const char *digits = "0123456789abcdef";
    std::string out;
    for (const Packet &p : packets) {
        if (!out.empty())
            out += ' ';
        out += digits[p.tag >> 4];
        out += digits[p.tag & 15];
        out += ':';
        for (uint8_t b : p.payload) {
            out += digits[b >> 4];
            out += digits[b & 15];
        }
    }
    return out;
}

} // namespace

Outcome RingSession::request(const Bytes &bytes, const std::function<bool(const Packet &)> &terminal,
                             std::vector<Packet> &out, int quietMs)
{
    out.clear();
    std::string error;
    if (!m_transport.transact(bytes, terminal, quietMs, out, error))
        return Outcome::failure(error.empty() ? "Bluetooth link lost" : error);
    return {};
}

Outcome RingSession::firmware(DeviceInfo &out)
{
    std::vector<Packet> packets;
    Outcome o = request(req::firmware(), [](const Packet &p) { return p.tag == 0x09; }, packets);
    if (!o.ok)
        return o;
    const Packet *p = findTag(packets, 0x09);
    if (!p)
        return Outcome::failure("the ring did not answer the firmware request");
    auto info = parseDeviceInfo(*p);
    if (!info)
        return Outcome::failure("unreadable firmware response");
    out = *info;
    return {};
}

Outcome RingSession::serial(std::string &out)
{
    std::vector<Packet> packets;
    Outcome o = request(req::productSerial(), [](const Packet &p) { return p.tag == 0x19; }, packets);
    if (!o.ok)
        return o;
    const Packet *p = findTag(packets, 0x19);
    auto text = p ? parseProductAscii(*p) : std::nullopt;
    if (!text)
        return Outcome::failure("no serial number in the answer");
    out = *text;
    return {};
}

Outcome RingSession::hardwareId(std::string &out)
{
    std::vector<Packet> packets;
    Outcome o = request(req::productHardware(), [](const Packet &p) { return p.tag == 0x19; }, packets);
    if (!o.ok)
        return o;
    const Packet *p = findTag(packets, 0x19);
    auto text = p ? parseProductAscii(*p) : std::nullopt;
    if (!text)
        return Outcome::failure("no hardware id in the answer");
    out = *text;
    return {};
}

Outcome RingSession::authenticate(const AesKey &key, AuthResult *result)
{
    std::vector<Packet> packets;
    Outcome o = request(req::authNonce(), [](const Packet &p) { return p.extTag() == 0x2c; }, packets);
    if (!o.ok)
        return o;
    const Packet *nonceReply = findExt(packets, 0x2c);
    if (!nonceReply || nonceReply->payload.size() < 2) {
        // 2f022f01 is what a ring says when it wants the key first; for the
        // nonce request itself, silence means the link is not encrypted yet.
        return Outcome::failure("no nonce from the ring (" + hexOf(packets)
                                + "). The Bluetooth link may not be paired and encrypted yet.");
    }

    const AesBlock encrypted = encryptNonce(key, nonceReply->payload.data() + 1,
                                            nonceReply->payload.size() - 1);
    o = request(req::authenticate(encrypted), [](const Packet &p) { return p.extTag() == 0x2e; }, packets);
    if (!o.ok)
        return o;
    const Packet *verdict = findExt(packets, 0x2e);
    if (!verdict || verdict->payload.size() < 2)
        return Outcome::failure("no authentication verdict from the ring (" + hexOf(packets) + ")");

    const AuthResult r = authResultFromByte(verdict->payload[1]);
    if (result)
        *result = r;
    if (r != AuthResult::Success)
        return Outcome::failure(authResultText(r));
    return {};
}

Outcome RingSession::setAuthKey(const AesKey &key)
{
    std::vector<Packet> packets;
    Outcome o = request(req::setAuthKey(key), [](const Packet &p) { return p.tag == 0x25; }, packets);
    if (!o.ok)
        return o;
    const Packet *p = findTag(packets, 0x25);
    if (!p || p->payload.empty())
        return Outcome::failure("the ring did not acknowledge the key (" + hexOf(packets) + ")");
    if (p->payload[0] != 0x00)
        return Outcome::failure("the ring refused the key (status "
                                + std::to_string(p->payload[0])
                                + "); it only accepts a new key after a factory reset");
    return {};
}

Outcome RingSession::battery(Battery &out)
{
    std::vector<Packet> packets;
    Outcome o = request(req::battery(), [](const Packet &p) { return p.tag == 0x0d || p.extTag() == 0x2f; },
                        packets);
    if (!o.ok)
        return o;
    const Packet *p = findTag(packets, 0x0d);
    auto b = p ? parseBattery(*p) : std::nullopt;
    if (!b)
        return Outcome::failure("no battery level in the answer (" + hexOf(packets) + ")");
    out = *b;
    return {};
}

Outcome RingSession::syncTime(uint64_t unixSeconds, int8_t timezoneHalfHours)
{
    std::vector<Packet> packets;
    // The ring answers 0x13; the quiet window covers a ring that does not.
    return request(req::syncTime(unixSeconds, timezoneHalfHours),
                   [](const Packet &p) { return p.tag == 0x13; }, packets);
}

Outcome RingSession::checkSleepAnalysis()
{
    std::vector<Packet> packets;
    return request(req::checkSleepAnalysis(false), [](const Packet &p) { return p.tag == 0x29; }, packets);
}

Outcome RingSession::setupAppStream()
{
    std::string hardware;
    if (!hardwareId(hardware).ok)
        return {};
    const size_t us = hardware.rfind('_');
    if (us == std::string::npos || hardware.substr(us + 1) != "05")
        return {};

    std::vector<Packet> packets;
    Outcome o = request(req::streamSubscribe(0x02), [](const Packet &p) { return p.tag == 0x17; }, packets);
    if (!o.ok)
        return o;
    static const struct { uint8_t category; uint16_t flags; } kCategories[] = {
        { 0x14, 0x1000 }, { 0x18, 0x1000 }, { 0x28, 0x0900 },
        { 0x34, 0x0400 }, { 0x04, 0x1000 }, { 0x08, 0x1000 },
    };
    for (const auto &c : kCategories) {
        o = request(req::eventSubscribe(c.category, c.flags),
                    [](const Packet &p) { return p.tag == 0x19; }, packets);
        if (!o.ok)
            return o;
    }
    // Parameter reads answer with ext 0x21; the `2f 02 03 01` state poll with
    // ext 0x04. Both were confirmed in on-device captures.
    static const uint8_t kParams[] = { 0x02, 0x04, 0x0b, 0x0d, 0x03, 0x0b, 0x10 };
    for (uint8_t id : kParams) {
        o = request(req::paramRead(id), [](const Packet &p) { return p.extTag() == 0x21; }, packets);
        if (!o.ok)
            return o;
        if (id == 0x04) {
            o = request(Bytes { 0x2f, 0x02, 0x03, 0x01 },
                        [](const Packet &p) { return p.extTag() == 0x04; }, packets);
            if (!o.ok)
                return o;
        }
    }
    return {};
}

Outcome RingSession::featureMode(uint8_t feature, uint8_t &mode)
{
    std::vector<Packet> packets;
    Outcome o = request(req::featureStatus(feature), [](const Packet &p) { return p.extTag() == 0x21; },
                        packets);
    if (!o.ok)
        return o;
    // 2f 06 21 <feature> <mode> <status> <state> <subscription>
    const Packet *p = findExt(packets, 0x21);
    if (!p || p->payload.size() < 5)
        return Outcome::failure("no feature status in the answer (" + hexOf(packets) + ")");
    mode = p->payload[2];
    return {};
}

Outcome RingSession::setFeatureMode(uint8_t feature, uint8_t mode)
{
    std::vector<Packet> packets;
    Outcome o = request(req::setFeatureMode(feature, mode),
                        [](const Packet &p) { return p.extTag() == 0x23; }, packets);
    if (!o.ok)
        return o;
    const Packet *p = findExt(packets, 0x23);
    if (!p || p->payload.size() < 3)
        return Outcome::failure("no answer to the feature switch (" + hexOf(packets) + ")");
    if (p->payload[2] != 0)
        return Outcome::failure("the ring declined the feature switch (result "
                                + std::to_string(p->payload[2]) + ")");
    return {};
}

DrainResult RingSession::drain(uint32_t cursor, const BatchHandler &onBatch)
{
    DrainResult result;
    result.nextCursor = cursor;
    uint32_t start = cursor;
    int tailPasses = 0;
    bool extended = true;

    // A batch ends with the ring's own summary packet (0x11 legacy, ext 0x42
    // extended); an ext 0x00 reply is "this request is not supported".
    const auto batchTerminal = [](const Packet &p) {
        return p.tag == 0x11 || p.extTag() == 0x42 || p.extTag() == 0x00;
    };

    // Safety bound against a ring that never reports drained.
    for (int pass = 0; pass < 100000; ++pass) {
        std::vector<Packet> packets;
        Outcome o = request(req::dataFlush(), [](const Packet &p) { return p.tag == 0x29; }, packets);
        if (!o.ok) {
            result.status = o;
            return result;
        }

        std::vector<Packet> fetched;
        if (extended) {
            o = request(req::extGetEvent(static_cast<uint64_t>(start) * 100, kExtBatchMaxEvents, 0),
                        batchTerminal, fetched, kBatchQuietMs);
            if (!o.ok) {
                result.status = o;
                return result;
            }
            if (findExt(fetched, 0x00)) {
                extended = false;
                fetched.clear();
            }
        }
        if (!extended) {
            o = request(req::getEvent(start, 255, -1), batchTerminal, fetched, kBatchQuietMs);
            if (!o.ok) {
                result.status = o;
                return result;
            }
        }
        packets.insert(packets.end(), fetched.begin(), fetched.end());

        const HistoryBatch raw = decodeBatch(packets);
        // The summary is the explicit terminator. A batch without one means the
        // link died mid-batch, which is an error, never a finished sync.
        if (!raw.haveSummary) {
            result.status = Outcome::failure(
                    "the event batch ended without the ring's summary: the link was lost "
                    "mid-transfer. Sync again to resume from the last stored batch.");
            return result;
        }
        if (raw.resultCode != 0) {
            result.status = Outcome::failure("the ring rejected the history request (result code "
                                             + std::to_string(raw.resultCode) + ")");
            return result;
        }

        const ValidatedBatch batch = validateBatch(raw, start);
        const bool progressed = batch.progressed(start);
        const bool tail = progressed && batch.bytesLeft == 0 && batch.events.size() < kTailBatchEvents;
        if (progressed)
            start = batch.nextCursor;

        result.eventsSynced += static_cast<uint32_t>(batch.events.size());
        result.nextCursor = start;

        SyncProgress progress;
        progress.cursor = start;
        progress.bytesLeft = batch.bytesLeft;
        progress.eventsSynced = result.eventsSynced;
        if (!onBatch(batch.events, progress)) {
            result.status = Outcome::failure("sync aborted");
            return result;
        }

        if (!progressed) {
            if (batch.bytesLeft == 0)
                break; // drained: an empty pass and the ring agrees
            result.status = Outcome::failure(
                    "the ring reports " + std::to_string(batch.bytesLeft)
                    + " bytes of events left but sent none; stopping instead of looping");
            return result;
        }
        // bytes_left == 0 alone is not proof of a drained ring: a Gen 3 ring
        // reports 0 on the first batch and keeps serving. Pull until a pass
        // comes back empty.
        if (tail) {
            if (++tailPasses >= kMaxTailPasses)
                break;
        } else {
            tailPasses = 0;
        }
    }
    return result;
}

} // namespace Oura

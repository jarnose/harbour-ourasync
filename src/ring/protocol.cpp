#include "protocol.h"

#include <algorithm>

namespace Oura {

const char *const kServiceUuid = "98ed0001-a541-11e4-b6a0-0002a5d5c51b";
const char *const kWriteCharUuid = "98ed0002-a541-11e4-b6a0-0002a5d5c51b";
const char *const kNotifyCharUuid = "98ed0003-a541-11e4-b6a0-0002a5d5c51b";

namespace {

void putLe(Bytes &out, uint64_t value, int bytes)
{
    for (int i = 0; i < bytes; ++i)
        out.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xff));
}

uint32_t le32(const Bytes &b, size_t i)
{
    return static_cast<uint32_t>(b[i]) | (static_cast<uint32_t>(b[i + 1]) << 8)
            | (static_cast<uint32_t>(b[i + 2]) << 16) | (static_cast<uint32_t>(b[i + 3]) << 24);
}

std::string version3(const Bytes &b, size_t at)
{
    return std::to_string(b[at]) + "." + std::to_string(b[at + 1]) + "." + std::to_string(b[at + 2]);
}

} // namespace

Bytes encodePacket(uint8_t tag, const Bytes &payload)
{
    Bytes out;
    out.reserve(payload.size() + 2);
    out.push_back(tag);
    out.push_back(static_cast<uint8_t>(payload.size()));
    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

std::vector<Packet> parsePackets(const Bytes &frame)
{
    std::vector<Packet> out;
    size_t i = 0;
    while (i + 2 <= frame.size()) {
        const size_t len = frame[i + 1];
        const size_t start = i + 2;
        const size_t end = start + len;
        if (end > frame.size())
            break;
        Packet p;
        p.tag = frame[i];
        p.payload.assign(frame.begin() + static_cast<long>(start), frame.begin() + static_cast<long>(end));
        out.push_back(std::move(p));
        i = end;
    }
    if (out.empty() && frame.size() >= 2) {
        Packet p;
        p.tag = frame[0];
        p.payload.assign(frame.begin() + 2, frame.end());
        out.push_back(std::move(p));
    }
    return out;
}

// ---- requests ---------------------------------------------------------------

namespace req {

Bytes firmware() { return { 0x08, 0x03, 0x00, 0x00, 0x00 }; }
Bytes battery() { return { 0x0c, 0x00 }; }
Bytes authNonce() { return { 0x2f, 0x01, 0x2b }; }

Bytes authenticate(const AesBlock &encryptedNonce)
{
    Bytes payload { 0x2d };
    payload.insert(payload.end(), encryptedNonce.begin(), encryptedNonce.end());
    return encodePacket(0x2f, payload);
}

Bytes setAuthKey(const AesKey &key)
{
    return encodePacket(0x24, Bytes(key.begin(), key.end()));
}

Bytes syncTime(uint64_t unixSeconds, int8_t timezoneHalfHours)
{
    Bytes payload;
    putLe(payload, unixSeconds, 8);
    payload.push_back(static_cast<uint8_t>(timezoneHalfHours));
    return encodePacket(0x12, payload);
}

Bytes streamSubscribe(uint8_t mode) { return { 0x16, 0x01, mode }; }

Bytes eventSubscribe(uint8_t category, uint16_t flags)
{
    Bytes payload { category };
    putLe(payload, flags, 2);
    return encodePacket(0x18, payload);
}

Bytes setNotification(uint8_t flags) { return { 0x1c, 0x01, flags }; }
Bytes capabilities(uint8_t page) { return { 0x2f, 0x02, 0x01, page }; }
Bytes productSerial() { return { 0x18, 0x03, 0x08, 0x00, 0x10 }; }
Bytes productHardware() { return { 0x18, 0x03, 0x18, 0x00, 0x10 }; }

Bytes getEvent(uint32_t startDeciseconds, uint8_t maxEvents, int32_t flags)
{
    Bytes payload;
    putLe(payload, startDeciseconds, 4);
    payload.push_back(maxEvents);
    putLe(payload, static_cast<uint32_t>(flags), 4);
    return encodePacket(0x10, payload);
}

Bytes extGetEvent(uint64_t startMs, uint16_t maxEvents, uint8_t bufferId)
{
    Bytes payload { 0x41, bufferId };
    putLe(payload, startMs, 8);
    putLe(payload, maxEvents, 2);
    return encodePacket(0x2f, payload);
}

Bytes dataFlush() { return { 0x28, 0x01, 0x00 }; }
Bytes featureStatus(uint8_t f) { return { 0x2f, 0x02, 0x20, f }; }
Bytes featureLatest(uint8_t f) { return { 0x2f, 0x02, 0x24, f }; }
Bytes setFeatureMode(uint8_t f, uint8_t mode) { return { 0x2f, 0x03, 0x22, f, mode }; }
Bytes paramRead(uint8_t id) { return { 0x2f, 0x02, 0x20, id }; }
Bytes checkSleepAnalysis(bool force) { return { 0x28, 0x01, static_cast<uint8_t>(force ? 1 : 0) }; }
Bytes realtimeOff() { return { 0x06, 0x04, 0x00, 0x00, 0x00, 0x00 }; }

} // namespace req

// ---- responses --------------------------------------------------------------

AuthResult authResultFromByte(uint8_t b)
{
    switch (b) {
    case 0: return AuthResult::Success;
    case 1: return AuthResult::AuthenticationError;
    case 2: return AuthResult::InFactoryReset;
    case 3: return AuthResult::NotOriginalOnboardedDevice;
    default: return AuthResult::Unknown;
    }
}

const char *authResultText(AuthResult r)
{
    switch (r) {
    case AuthResult::Success:
        return "authenticated";
    case AuthResult::AuthenticationError:
        return "the ring rejected the key: it is not the key installed on this ring";
    case AuthResult::InFactoryReset:
        return "the ring is in factory-reset state: it has no key yet and must be set up first";
    case AuthResult::NotOriginalOnboardedDevice:
        return "the ring was onboarded by a different device; its key is not the one in use";
    case AuthResult::Unknown:
        break;
    }
    return "unexpected authentication state";
}

std::optional<DeviceInfo> parseDeviceInfo(const Packet &p)
{
    if (p.tag != 0x09 || p.payload.size() < 18)
        return std::nullopt;
    DeviceInfo info;
    info.apiVersion = version3(p.payload, 0);
    info.firmwareVersion = version3(p.payload, 3);
    info.bootloaderVersion = version3(p.payload, 6);
    info.btStackVersion = version3(p.payload, 9);
    static const char *digits = "0123456789abcdef";
    for (int i = 17; i >= 12; --i) {
        const uint8_t b = p.payload[static_cast<size_t>(i)];
        if (!info.mac.empty())
            info.mac += ':';
        info.mac += digits[b >> 4];
        info.mac += digits[b & 15];
    }
    return info;
}

std::optional<Battery> parseBattery(const Packet &p)
{
    if (p.tag != 0x0d || p.payload.size() < 3)
        return std::nullopt;
    Battery b;
    b.percent = p.payload[0];
    b.chargingProgress = p.payload[1];
    b.chargingRecommended = p.payload[2];
    return b;
}

std::optional<std::string> parseProductAscii(const Packet &p)
{
    if (p.tag != 0x19 || p.payload.empty() || p.payload[0] != 0)
        return std::nullopt;
    std::string text(p.payload.begin() + 1, p.payload.end());
    while (!text.empty() && (text.back() == '\0' || text.back() == ' '))
        text.pop_back();
    if (text.empty())
        return std::nullopt;
    return text;
}

// ---- history ----------------------------------------------------------------

RingEvent eventFromPacket(const Packet &p)
{
    RingEvent e;
    e.tag = p.tag;
    if (p.payload.size() >= 4)
        e.timestamp = le32(p.payload, 0);
    if (p.payload.size() > 4)
        e.body.assign(p.payload.begin() + 4, p.payload.end());
    return e;
}

namespace {

bool readVarint(const uint8_t *data, size_t length, uint64_t &value, size_t &used)
{
    value = 0;
    for (size_t i = 0; i < length && i < 10; ++i) {
        value |= static_cast<uint64_t>(data[i] & 0x7f) << (7 * i);
        if (!(data[i] & 0x80)) {
            used = i + 1;
            return true;
        }
    }
    return false;
}

} // namespace

std::vector<Packet> ExtEventEnvelopeParser::push(const Packet &chunk)
{
    std::vector<Packet> out;
    if (chunk.tag != 0x2f || chunk.payload.empty() || chunk.payload[0] != 0x43)
        return out;

    const uint8_t *data = chunk.payload.data() + 1;
    size_t remaining = chunk.payload.size() - 1;

    while (remaining > 0) {
        if (!m_haveLength) {
            const size_t take = std::min<size_t>(2 - m_lenBuf.size(), remaining);
            m_lenBuf.insert(m_lenBuf.end(), data, data + take);
            data += take;
            remaining -= take;
            if (m_lenBuf.size() < 2)
                continue;
            m_expected = static_cast<size_t>(m_lenBuf[0]) | (static_cast<size_t>(m_lenBuf[1]) << 8);
            m_lenBuf.clear();
            m_payload.clear();
            m_haveLength = m_expected != 0;
            if (!m_haveLength)
                continue;
        }

        const size_t take = std::min(m_expected - m_payload.size(), remaining);
        m_payload.insert(m_payload.end(), data, data + take);
        data += take;
        remaining -= take;
        if (m_payload.size() == m_expected) {
            std::vector<Packet> bundle = parseBundle();
            out.insert(out.end(), std::make_move_iterator(bundle.begin()),
                       std::make_move_iterator(bundle.end()));
            m_payload.clear();
            m_haveLength = false;
        }
    }
    return out;
}

std::vector<Packet> ExtEventEnvelopeParser::parseBundle()
{
    std::vector<Packet> out;
    size_t i = 0;
    bool haveLast = false;
    uint64_t lastMs = 0;
    bool haveLastTs = false;
    uint32_t lastTs = 0;

    while (i + 3 <= m_payload.size()) {
        const uint8_t tag = m_payload[i + 1];
        const size_t extLen = m_payload[i + 2];
        const size_t start = i + 3;
        const size_t end = start + extLen;
        if (tag < kHistoryEventPrefix || end > m_payload.size())
            break;

        uint64_t encoded = 0;
        size_t used = 0;
        if (!readVarint(m_payload.data() + start, end - start, encoded, used))
            break;

        // The first timestamp in a bundle is absolute milliseconds; the rest
        // are deltas. A delta can never be as large as 1e8 ms (27 hours), so a
        // value that big is taken as a fresh absolute one.
        const uint64_t ms = (haveLast && encoded < 100000000ULL) ? lastMs + encoded : encoded;
        lastMs = ms;
        haveLast = true;

        uint32_t ts = static_cast<uint32_t>(ms / 100);
        if (haveLastTs && encoded > 0 && ts <= lastTs)
            ts = lastTs + 1;
        lastTs = ts;
        haveLastTs = true;

        Packet p;
        p.tag = tag;
        putLe(p.payload, ts, 4);
        p.payload.insert(p.payload.end(), m_payload.begin() + static_cast<long>(start + used),
                         m_payload.begin() + static_cast<long>(end));
        out.push_back(std::move(p));
        i = end;
    }
    return out;
}

HistoryBatch decodeBatch(const std::vector<Packet> &packets)
{
    HistoryBatch batch;
    ExtEventEnvelopeParser envelopes;

    for (const Packet &p : packets) {
        if (p.tag == 0x11) {
            if (p.payload.size() >= 6) {
                batch.bytesLeft = le32(p.payload, 2);
                batch.haveSummary = true;
            }
        } else if (p.extTag() == 0x42) {
            // events:u16, sleep progress:u8, bytes left:u32, buffer:u8, result:u8
            if (p.payload.size() >= 10) {
                batch.bytesLeft = le32(p.payload, 4);
                batch.resultCode = p.payload[9];
                batch.haveSummary = true;
            }
        } else if (p.extTag() == 0x43) {
            for (const Packet &event : envelopes.push(p))
                if (event.tag >= kHistoryEventPrefix)
                    batch.events.push_back(eventFromPacket(event));
        } else if (p.tag >= kHistoryEventPrefix) {
            batch.events.push_back(eventFromPacket(p));
        }
    }
    return batch;
}

namespace {

// A retained batch may legitimately jump over quiet periods, but never by
// years. 180 days, in deciseconds.
constexpr uint32_t kMaxCursorAdvance = 180u * 24u * 60u * 60u * 10u;

} // namespace

ValidatedBatch validateBatch(const HistoryBatch &batch, uint32_t batchStart)
{
    ValidatedBatch out;
    out.bytesLeft = batch.bytesLeft;
    uint32_t maxTs = batchStart;
    // Saturating add: batchStart is a u32 and may be near the top.
    const uint64_t ceiling = static_cast<uint64_t>(batchStart) + kMaxCursorAdvance;

    for (const RingEvent &e : batch.events) {
        if (e.timestamp < batchStart) {
            ++out.replayed;
        } else if (e.timestamp <= ceiling) {
            maxTs = std::max(maxTs, e.timestamp);
            out.events.push_back(e);
        } else {
            ++out.rejected;
        }
    }
    out.nextCursor = out.events.empty() ? batchStart : maxTs + 1;
    return out;
}

} // namespace Oura

#pragma once

#include "aes128.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// The Oura ring's GATT protocol, as far as a read-only client needs it.
//
// Provenance, so that a future reader knows what was verified against what:
//  * The framing, request layouts and event envelopes come from the open_oura
//    reverse-engineering (github.com/Th0rgal/open_oura), which was tested live
//    against a Ring 3 Horizon and a Ring 5.
//  * The authentication exchange (0x2f/0x2b nonce, 0x2f/0x2d authenticate,
//    0x24 set-key, 0x12 time sync, 0x2f/0x41 extended event fetch and its 0x42
//    confirmation) was re-read in the bytecode of the Oura Android app 7.27.0's
//    com.ouraring.ourakit package and found unchanged. See docs/apk-findings.md.
//
// Everything here is Qt-free so it compiles and is tested with plain g++.
namespace Oura {

using Bytes = std::vector<uint8_t>;

// GATT identifiers (BlueZ reports UUIDs in lower case).
extern const char *const kServiceUuid;
extern const char *const kWriteCharUuid;
extern const char *const kNotifyCharUuid;

// Tags at or above this value are history-event frames rather than command
// responses.
constexpr uint8_t kHistoryEventPrefix = 0x41;

// One `tag | length | payload` frame.
struct Packet
{
    uint8_t tag = 0;
    Bytes payload;

    // First payload byte of an extended (0x2f) frame, which names the
    // extended operation. -1 for anything else.
    int extTag() const { return tag == 0x2f && !payload.empty() ? payload[0] : -1; }
};

Bytes encodePacket(uint8_t tag, const Bytes &payload);

// Every complete frame packed into one notification. The ring concatenates
// several events into a single notification, so parsing only the first would
// silently drop the rest. A trailing fragment is ignored; if nothing complete
// is found, the lenient single-frame parse is tried (rings occasionally pad).
std::vector<Packet> parsePackets(const Bytes &notification);

// ---- request builders -------------------------------------------------------
namespace req {

Bytes firmware();
Bytes battery();
Bytes authNonce();
Bytes authenticate(const AesBlock &encryptedNonce);
// Installs the app-auth key. Only a factory-reset ring accepts it.
Bytes setAuthKey(const AesKey &key);
// Erases the key, every bond and the ring's own history (open_oura, docs/factory-reset.md).
Bytes factoryReset();
// `12 09 <u64 LE unix seconds> <timezone in half hours>`. This is the shape
// SyncTime.generateRequestBytes builds in the Android app (7.27.0).
Bytes syncTime(uint64_t unixSeconds, int8_t timezoneHalfHours);
Bytes streamSubscribe(uint8_t mode);
Bytes eventSubscribe(uint8_t category, uint16_t flags);
Bytes setNotification(uint8_t flags);
Bytes capabilities(uint8_t page);
Bytes productSerial();
Bytes productHardware();
Bytes getEvent(uint32_t startDeciseconds, uint8_t maxEvents, int32_t flags);
// Extended fetch, cursor in milliseconds. Buffer 0 is the normal history.
Bytes extGetEvent(uint64_t startMs, uint16_t maxEvents, uint8_t bufferId);
Bytes dataFlush();
Bytes featureStatus(uint8_t feature);
Bytes featureLatest(uint8_t feature);
Bytes setFeatureMode(uint8_t feature, uint8_t mode);
Bytes paramRead(uint8_t id);
Bytes checkSleepAnalysis(bool force);
Bytes realtimeOff();

} // namespace req

// ---- feature ids ------------------------------------------------------------
namespace feature {
constexpr uint8_t DaytimeHr = 0x02;
constexpr uint8_t ExerciseHr = 0x03;
constexpr uint8_t Spo2 = 0x04;
constexpr uint8_t RestingHr = 0x08;
constexpr uint8_t RealSteps = 0x0b;
} // namespace feature

namespace featureMode {
constexpr uint8_t Off = 0;
constexpr uint8_t Automatic = 1;
} // namespace featureMode

// ---- responses --------------------------------------------------------------

enum class AuthResult : uint8_t {
    Success = 0,
    AuthenticationError = 1,
    InFactoryReset = 2,
    NotOriginalOnboardedDevice = 3,
    Unknown = 0xff,
};

AuthResult authResultFromByte(uint8_t b);
const char *authResultText(AuthResult r);

struct DeviceInfo
{
    std::string apiVersion;
    std::string firmwareVersion;
    std::string bootloaderVersion;
    std::string btStackVersion;
    std::string mac;
};

std::optional<DeviceInfo> parseDeviceInfo(const Packet &p);

struct Battery
{
    int percent = 0;
    int chargingProgress = 0;
    int chargingRecommended = 0;
};

std::optional<Battery> parseBattery(const Packet &p);

// Product-info response (tag 0x19): status byte, then ASCII.
std::optional<std::string> parseProductAscii(const Packet &p);

// ---- history events ---------------------------------------------------------

// A history event with its envelope decoded and its body kept raw. Bodies are
// stored lossless so a decoder fixed later can be re-run without re-syncing.
struct RingEvent
{
    uint8_t tag = 0;
    uint32_t timestamp = 0; // ring clock, deciseconds
    Bytes body;
};

RingEvent eventFromPacket(const Packet &p);

// Reassembles `ExtGetEvent` envelope chunks (0x2f, ext 0x43) into ordinary
// history-event packets. The Android app accumulates length-prefixed
// envelopes; each holds bundled `control | tag | len | varint ts | body`
// events whose first timestamp is absolute milliseconds and the rest deltas.
class ExtEventEnvelopeParser
{
public:
    std::vector<Packet> push(const Packet &chunk);

private:
    std::vector<Packet> parseBundle();

    Bytes m_lenBuf;
    bool m_haveLength = false;
    size_t m_expected = 0;
    Bytes m_payload;
};

struct HistoryBatch
{
    std::vector<RingEvent> events;
    uint32_t bytesLeft = 0;
    bool haveSummary = false;
    // Non-zero when the extended confirmation reported a failure.
    int resultCode = 0;
};

// Folds the packets of one fetch into a batch: legacy events plus the 0x11
// summary, or extended envelopes plus the 0x42 confirmation.
HistoryBatch decodeBatch(const std::vector<Packet> &packets);

struct ValidatedBatch
{
    std::vector<RingEvent> events;
    uint32_t bytesLeft = 0;
    uint32_t nextCursor = 0;
    uint32_t rejected = 0; // implausible jump past the cursor: corrupt envelope
    uint32_t replayed = 0; // older than the cursor: the ring re-sent its tail

    bool progressed(uint32_t previousCursor) const
    {
        return !events.empty() && nextCursor > previousCursor;
    }
};

// Drops replayed and implausible events and computes the cursor to continue
// from. A corrupt extended envelope otherwise poisons the persisted cursor with
// a near-u32::MAX timestamp and forces every later sync to start from zero.
ValidatedBatch validateBatch(const HistoryBatch &batch, uint32_t batchStart);

} // namespace Oura

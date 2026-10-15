#pragma once

#include "events.h"
#include "protocol.h"

#include <functional>
#include <string>
#include <vector>

// The ring conversation, written against an abstract Transport so that the Qt
// BlueZ link and a scripted test double are interchangeable. Nothing in here
// knows about Qt or D-Bus.
namespace Oura {

class Transport
{
public:
    virtual ~Transport() = default;

    // Writes `request`, then collects notification packets until `terminal`
    // returns true for one of them, or `quietMs` pass without any frame. The
    // quiet window is only the fallback for an unexpected reply or a dead link;
    // a known reply ends the wait the moment it arrives.
    //
    // Returns false when the write itself failed or the link is gone, with the
    // reason in `error`. Receiving nothing is not a failure here: the caller
    // decides what an empty answer means.
    virtual bool transact(const Bytes &request, const std::function<bool(const Packet &)> &terminal,
                          int quietMs, std::vector<Packet> &out, std::string &error) = 0;
};

struct Outcome
{
    bool ok = true;
    std::string error;

    static Outcome failure(std::string why) { return { false, std::move(why) }; }
};

struct SyncProgress
{
    uint32_t cursor = 0;      // next ring-clock cursor, deciseconds
    uint32_t bytesLeft = 0;   // what the ring says it still holds
    uint32_t eventsSynced = 0;
};

struct DrainResult
{
    Outcome status;
    uint32_t eventsSynced = 0;
    uint32_t nextCursor = 0;
};

class RingSession
{
public:
    explicit RingSession(Transport &transport) : m_transport(transport) {}

    // Reads that need no authentication.
    Outcome firmware(DeviceInfo &out);
    Outcome serial(std::string &out);
    Outcome hardwareId(std::string &out);

    // App-level authentication, repeated on every connection. On a ring that
    // has a key installed, everything below needs it.
    Outcome authenticate(const AesKey &key, AuthResult *result = nullptr);

    // Installs a key on a factory-reset ring.
    Outcome setAuthKey(const AesKey &key);

    // Erases the ring: key, bonds and its stored history. Needs an authenticated session. The
    // ring usually drops the link before it answers, so no answer is not a failure; a status
    // other than zero in an answer is.
    Outcome factoryReset();

    Outcome battery(Battery &out);
    Outcome syncTime(uint64_t unixSeconds, int8_t timezoneHalfHours);
    Outcome checkSleepAnalysis();

    // Registers the stream the way the official app does before it fetches.
    // Ring 5 only; a no-op on other generations.
    Outcome setupAppStream();

    // The three on-ring measurement features that need switching on. Reads the
    // present mode first and writes only when it differs.
    Outcome featureMode(uint8_t feature, uint8_t &mode);
    Outcome setFeatureMode(uint8_t feature, uint8_t mode);

    // Pulls history events from `cursor` (ring deciseconds) until the ring
    // reports nothing left. `onBatch` receives each validated batch and returns
    // false to abort, which is how the caller persists events and the cursor
    // together: a dead link then resumes from the last stored batch, it does
    // not start over.
    using BatchHandler = std::function<bool(const std::vector<RingEvent> &, const SyncProgress &)>;
    DrainResult drain(uint32_t cursor, const BatchHandler &onBatch);

    static constexpr int kQuietMs = 1500;
    static constexpr int kBatchQuietMs = 6000;
    static constexpr uint16_t kExtBatchMaxEvents = 4096;
    // After `bytes_left` reaches zero a Gen 3 ring can keep producing a few new
    // events on every request. Accept this many small trailing batches, then
    // stop; the cursor is persisted so nothing is lost.
    static constexpr int kMaxTailPasses = 3;
    static constexpr size_t kTailBatchEvents = 32;

private:
    Outcome request(const Bytes &bytes, const std::function<bool(const Packet &)> &terminal,
                    std::vector<Packet> &out, int quietMs = kQuietMs);

    Transport &m_transport;
};

} // namespace Oura

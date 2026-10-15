#pragma once

#include "ring/protocol.h"

#include <cstdint>
#include <vector>

// Puts the ring's clock on the calendar.
//
// Every event carries a timestamp in deciseconds on a counter that belongs to
// the ring, not to the world: it restarts near zero after some resets, and it
// stalls while the ring is off. What ties it to UTC is the ring's own record of
// the wall clock at a given counter value: the time_sync event written when the
// phone sets the time, and the rtc_beacon. So events are grouped into "epochs"
// (a stretch over which the counter never jumped backwards), and each epoch is
// anchored by whichever of its own time records lies nearest in counter terms.
//
// An epoch with no anchor at all falls back to arithmetic from the moment the
// phone downloaded its newest event. That is only as good as the gap between
// the event happening and the sync, and is reported as approximate rather than
// passed off as exact.
namespace Oura {

struct StoredEvent
{
    int64_t id = 0;            // insertion order
    uint8_t tag = 0;
    int64_t ds = 0;            // ring clock
    Bytes body;
    int64_t capturedUnix = 0;  // when the phone downloaded it
};

class RingClock
{
public:
    // `events` in insertion order, which is also download order. That order is
    // what separates one epoch from the next: within a download the counter
    // only goes up.
    explicit RingClock(const std::vector<StoredEvent> &events);

    int epochCount() const { return static_cast<int>(m_epochs.size()); }
    // Epoch of the i-th event passed to the constructor.
    int epochOf(size_t index) const { return m_epochOf[index]; }

    // UTC seconds for a ring time within an epoch.
    double utc(int epoch, int64_t ds) const;

    // True when the epoch has a real time record rather than download-time
    // arithmetic.
    bool anchored(int epoch) const;

private:
    struct Anchor
    {
        int64_t ds;
        int64_t utc;
    };
    struct Epoch
    {
        int64_t minDs = 0;
        int64_t maxDs = 0;
        int64_t capturedAtMax = 0;
        std::vector<Anchor> anchors;
    };

    std::vector<Epoch> m_epochs;
    std::vector<int> m_epochOf;
};

} // namespace Oura

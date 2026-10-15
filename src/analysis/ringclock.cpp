#include "ringclock.h"

#include "ring/events.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Oura {

namespace {

// How far the counter may step back inside one stretch before it counts as a
// reset. Events inside a batch are not strictly ordered, so a small step back
// is normal; six hours is not.
constexpr int64_t kResetSlackDs = 6LL * 3600 * 10;

} // namespace

RingClock::RingClock(const std::vector<StoredEvent> &events)
{
    m_epochOf.reserve(events.size());
    for (const StoredEvent &e : events) {
        if (m_epochs.empty() || e.ds < m_epochs.back().maxDs - kResetSlackDs) {
            Epoch epoch;
            epoch.minDs = epoch.maxDs = e.ds;
            epoch.capturedAtMax = e.capturedUnix;
            m_epochs.push_back(epoch);
        }
        Epoch &epoch = m_epochs.back();
        if (e.ds >= epoch.maxDs) {
            epoch.maxDs = e.ds;
            epoch.capturedAtMax = e.capturedUnix;
        }
        epoch.minDs = std::min(epoch.minDs, e.ds);

        if (e.tag == Tag::TimeSync || e.tag == Tag::RtcBeacon) {
            const auto stamp = e.tag == Tag::TimeSync ? decodeTimeSync(e.body) : decodeRtcBeacon(e.body);
            // A ring that has never been told the time reports a value near
            // 1970; that is no anchor.
            if (stamp && *stamp > 1000000000u)
                epoch.anchors.push_back({ e.ds, static_cast<int64_t>(*stamp) });
        }
        m_epochOf.push_back(static_cast<int>(m_epochs.size()) - 1);
    }
    for (Epoch &epoch : m_epochs)
        std::sort(epoch.anchors.begin(), epoch.anchors.end(),
                  [](const Anchor &a, const Anchor &b) { return a.ds < b.ds; });
}

bool RingClock::anchored(int epoch) const
{
    return epoch >= 0 && epoch < epochCount() && !m_epochs[static_cast<size_t>(epoch)].anchors.empty();
}

double RingClock::utc(int epochIndex, int64_t ds) const
{
    if (epochIndex < 0 || epochIndex >= epochCount())
        return std::nan("");
    const Epoch &epoch = m_epochs[static_cast<size_t>(epochIndex)];

    if (!epoch.anchors.empty()) {
        const Anchor *best = &epoch.anchors.front();
        for (const Anchor &a : epoch.anchors)
            if (std::llabs(a.ds - ds) < std::llabs(best->ds - ds))
                best = &a;
        return static_cast<double>(best->utc) + static_cast<double>(ds - best->ds) / 10.0;
    }
    return static_cast<double>(epoch.capturedAtMax) - static_cast<double>(epoch.maxDs - ds) / 10.0;
}

} // namespace Oura

#include "events.h"

#include <algorithm>
#include <cmath>

namespace Oura {

namespace {

uint16_t le16(const Bytes &b, size_t i)
{
    return static_cast<uint16_t>(b[i] | (b[i + 1] << 8));
}

uint32_t le32(const Bytes &b, size_t i)
{
    return static_cast<uint32_t>(b[i]) | (static_cast<uint32_t>(b[i + 1]) << 8)
            | (static_cast<uint32_t>(b[i + 2]) << 16) | (static_cast<uint32_t>(b[i + 3]) << 24);
}

} // namespace

const char *eventName(uint8_t tag)
{
    switch (tag) {
    case 0x41: return "ring_start";
    case 0x42: return "time_sync";
    case 0x43: return "debug_event";
    case 0x44: return "ibi_event";
    case 0x45: return "state_change";
    case 0x46: return "temp_event";
    case 0x47: return "motion_event";
    case 0x48: return "sleep_period_information";
    case 0x49: return "sleep_summary_1";
    case 0x4b: return "sleep_phase_information";
    case 0x4c: return "sleep_summary_2";
    case 0x4e: return "sleep_phase_details";
    case 0x4f: return "sleep_summary_3";
    case 0x50: return "activity_information";
    case 0x51: return "activity_summary_1";
    case 0x52: return "activity_summary_2";
    case 0x53: return "wear_event";
    case 0x54: return "recovery_summary";
    case 0x55: return "sleep_heart_rate";
    case 0x58: return "sleep_summary_4";
    case 0x5a: return "sleep_phase_data";
    case 0x5c: return "user_information";
    case 0x5d: return "hrv_event";
    case 0x60: return "ibi_and_amplitude_event";
    case 0x61: return "debug_data";
    case 0x62: return "on_demand_meas";
    case 0x69: return "temp_period";
    case 0x6a: return "sleep_period_information_2";
    case 0x6b: return "motion_period";
    case 0x6c: return "feature_session";
    case 0x6f: return "spo2_event";
    case 0x70: return "spo2_smoothed_event";
    case 0x72: return "sleep_acm_period";
    case 0x75: return "sleep_temp_event";
    case 0x76: return "bedtime_period";
    case 0x7e: return "real_step_event_feature_1";
    case 0x7f: return "real_step_event_feature_2";
    case 0x80: return "green_ibi_quality_event";
    case 0x81: return "cva_raw_ppg_data";
    case 0x85: return "rtc_beacon";
    case 0x8b: return "spo2_r_pi_event";
    default: return "unknown";
    }
}

std::optional<HrvSamples> decodeHrv(const Bytes &body)
{
    if (body.empty() || body.size() % 2 != 0)
        return std::nullopt;
    HrvSamples s;
    for (size_t i = 0; i < body.size(); i += 2) {
        s.hr.push_back(body[i]);
        s.rmssd.push_back(body[i + 1]);
    }
    return s;
}

std::vector<double> decodeTemperatures(const Bytes &body)
{
    std::vector<double> out;
    if (body.empty() || body.size() % 2 != 0)
        return out;
    for (size_t i = 0; i < body.size(); i += 2) {
        const double celsius = static_cast<int16_t>(le16(body, i)) / 100.0;
        if (celsius < -40.0 || celsius > 85.0)
            return {};
        out.push_back(celsius);
    }
    return out;
}

std::vector<GreenBeat> decodeGreenIbi(const Bytes &body)
{
    std::vector<GreenBeat> out;
    for (size_t i = 0; i + 1 < body.size(); i += 2) {
        GreenBeat b;
        b.ibiMs = (body[i + 1] & 0x07) | (body[i] << 3);
        b.quality = (body[i + 1] >> 3) & 0x03;
        out.push_back(b);
    }
    return out;
}

std::optional<SleepPhasePage> decodeSleepPhasePage(const Bytes &body)
{
    if (body.size() < 2)
        return std::nullopt;
    SleepPhasePage page;
    page.page = body[0];
    for (size_t i = 1; i < body.size(); ++i)
        for (int shift : { 6, 4, 2, 0 })
            page.phases.push_back(static_cast<uint8_t>((body[i] >> shift) & 0x03));
    return page;
}

std::optional<Bedtime> decodeBedtime(const Bytes &body)
{
    if (body.size() < 8)
        return std::nullopt;
    Bedtime b;
    b.startDs = le32(body, 0);
    b.endDs = le32(body, 4);
    return b;
}

std::optional<SleepPeriod> decodeSleepPeriod(const Bytes &body)
{
    if (body.size() < 10)
        return std::nullopt;
    const int motion = body[6];
    const int state = static_cast<int8_t>(body[7]);
    // The native parser throws on these ranges; drop the frame the same way.
    if (motion >= 120 || state < 0 || state >= 3)
        return std::nullopt;
    SleepPeriod p;
    p.averageHr = body[0] * 0.5;
    p.breath = body[4] / 8.0;
    p.motionCount = motion;
    p.sleepState = state;
    return p;
}

std::optional<ActivityMet> decodeActivityMet(const Bytes &body)
{
    if (body.empty())
        return std::nullopt;
    ActivityMet a;
    a.state = body[0];
    for (size_t i = 1; i < body.size(); ++i) {
        const uint8_t b = body[i];
        // Native scaling: below 128 the unit is 0.1 MET, above it 0.2 MET
        // steps continue from 12.8.
        const double met = b < 0x80 ? b * 0.1 : 12.8 + (b - 128) * 0.2;
        a.met.push_back(std::round(met * 100.0) / 100.0);
    }
    return a;
}

std::vector<int> decodeSpo2(const Bytes &body)
{
    std::vector<int> out;
    if (body.size() < 2)
        return out;
    size_t end = body.size();
    if (body[end - 1] == 0xff)
        --end;
    for (size_t i = 1; i < end; ++i)
        out.push_back(body[i]);
    return out;
}

std::vector<Spo2Sample> decodeSpo2RPi(const Bytes &body)
{
    std::vector<Spo2Sample> out;
    if (body.size() < 4 || (body.size() - 1) % 3 != 0)
        return out;
    for (size_t i = 1; i + 3 <= body.size(); i += 3) {
        Spo2Sample s;
        s.r = ((body[i] << 8) | body[i + 1]) / 16384.0;
        s.perfusionIndex = body[i + 2] / 255.0 * 0.05;
        out.push_back(s);
    }
    return out;
}

double spo2PercentFromR(double r)
{
    // gen4/oreo coefficients of ecore's spo2_simple_calculate.
    const double pct = 105.2 - 5.1 * r - 13.4 * r * r;
    return std::min(100.0, std::max(85.0, pct));
}

std::optional<int> decodeMotionSeconds(const Bytes &body)
{
    if (body.size() < 4)
        return std::nullopt;
    // The native parser rejects a body whose intensity bytes carry bit 0x40.
    if (body.size() >= 5 && (body[4] & 0x40))
        return std::nullopt;
    if (body.size() >= 6 && (body[5] & 0x40))
        return std::nullopt;
    return body[0] & 0x1f;
}

std::optional<MotionPeriod> decodeMotionPeriod(const Bytes &body)
{
    // Header and at least one code byte; the longest record seen carries 27 bytes of codes.
    if (body.size() < 2 || body.size() > 40)
        return std::nullopt;
    MotionPeriod m;
    m.periodType = body[0] >> 6;
    m.sequence = body[0] & 0x0f;
    int last = (body[0] >> 4) & 0x03;
    if (last == 0)
        last = 4;
    for (size_t i = 1; i < body.size(); ++i) {
        const int codes = i + 1 == body.size() ? last : 4;
        for (int j = 0; j < codes; ++j)
            m.states.push_back(static_cast<uint8_t>((body[i] >> (6 - 2 * j)) & 3));
    }
    return m;
}

std::optional<uint32_t> decodeTimeSync(const Bytes &body)
{
    if (body.size() < 4)
        return std::nullopt;
    return le32(body, 0);
}

std::optional<uint32_t> decodeRtcBeacon(const Bytes &body)
{
    if (body.size() < 10)
        return std::nullopt;
    return le32(body, 0);
}

std::optional<BatteryLog> decodeBatteryLog(const Bytes &body)
{
    if (body.size() < 4 || body[0] != 0x24)
        return std::nullopt;
    BatteryLog b;
    b.percent = body[1];
    b.millivolts = le16(body, 2);
    // Battery gauge percentages live in 0..100; anything else is another
    // debug subtype that happens to start with 0x24.
    if (b.percent > 100)
        return std::nullopt;
    return b;
}

} // namespace Oura

#pragma once

#include "protocol.h"

#include <optional>
#include <string>
#include <vector>

// Typed decoders for the history-event bodies this app visualises.
//
// The layouts come from open_oura, which recovered them from the ring's native
// parser (libringeventparser.so) and checked each against captured bytes. The
// ones open_oura itself marks "unvalidated" are not decoded here at all: a
// chart built on a guessed layout looks authoritative and is not.
//
// Every decoder returns an empty optional / empty vector for a body it does not
// recognise, so a firmware that changes a layout produces a gap, not a lie.
namespace Oura {
namespace Tag {
constexpr uint8_t RingStart = 0x41;
constexpr uint8_t TimeSync = 0x42;
constexpr uint8_t DebugEvent = 0x43;
constexpr uint8_t StateChange = 0x45;
constexpr uint8_t Temp = 0x46;
constexpr uint8_t Motion = 0x47;
constexpr uint8_t ActivityInfo = 0x50;
constexpr uint8_t WearEvent = 0x53;
constexpr uint8_t SleepPhaseInfo = 0x4b;
constexpr uint8_t SleepPhaseDetails = 0x4e;
constexpr uint8_t SleepPhaseData = 0x5a;
constexpr uint8_t Hrv = 0x5d;
constexpr uint8_t IbiAmplitude = 0x60;
constexpr uint8_t DebugData = 0x61;
constexpr uint8_t TempPeriod = 0x69;
constexpr uint8_t MotionPeriod = 0x6b;
constexpr uint8_t SleepPeriodInfo2 = 0x6a;
constexpr uint8_t Spo2 = 0x6f;
constexpr uint8_t SleepTemp = 0x75;
constexpr uint8_t BedtimePeriod = 0x76;
constexpr uint8_t GreenIbiQuality = 0x80;
constexpr uint8_t RtcBeacon = 0x85;
constexpr uint8_t Spo2RPi = 0x8b;
} // namespace Tag

const char *eventName(uint8_t tag);

// hrv_event: pairs of (average heart rate bpm, average RMSSD ms), one pair per five minutes.
// The first pair is the oldest and the event's timestamp is the end of the last one; a closing
// record pads its tail with a 0/0 pair.
struct HrvSamples
{
    std::vector<int> hr;
    std::vector<int> rmssd;
    int intervalMinutes = 5;
};
std::optional<HrvSamples> decodeHrv(const Bytes &body);

// ibi_and_amplitude_event (0x60): six beat-to-beat intervals in 14 bytes. Interval i is
// (p[i] << 3) | its middle two bits from p[12] and p[13] | (p[6 + i] & 1); the rest of p[6..11]
// is pulse amplitude, not decoded. The first interval is the oldest and the last one ends at the
// event's timestamp. (Layout from open_ring, checked against ringverse's reading of the event.)
std::vector<int> decodeIbi(const Bytes &body);

// temp_event / temp_period / sleep_temp_event: int16 LE centi-degrees Celsius.
// Rejected wholesale if any value is outside a physical sensor range.
std::vector<double> decodeTemperatures(const Bytes &body);

// green_ibi_quality_event: two bytes per beat, ibi_ms = (b1 & 7) | (b0 << 3),
// quality = (b1 >> 3) & 3. Only quality 1 is the firmware's own "good" flag.
struct GreenBeat
{
    int ibiMs = 0;
    int quality = 0;
    bool good() const { return quality == 1 && ibiMs >= 300 && ibiMs <= 2000; }
    int bpm() const { return 60000 / ibiMs; }
};
std::vector<GreenBeat> decodeGreenIbi(const Bytes &body);

// sleep_phase_data and friends: a page header (counts pages in one burst), then
// 2-bit phase codes, four per byte, most significant first. 52 per 14-byte page.
enum Phase : uint8_t { PhaseDeep = 0, PhaseLight = 1, PhaseRem = 2, PhaseAwake = 3 };
struct SleepPhasePage
{
    int page = 0;
    std::vector<uint8_t> phases;
};
std::optional<SleepPhasePage> decodeSleepPhasePage(const Bytes &body);

// bedtime_period: the ring's detected sleep window, ring deciseconds.
struct Bedtime
{
    uint32_t startDs = 0;
    uint32_t endDs = 0;
};
std::optional<Bedtime> decodeBedtime(const Bytes &body);

// sleep_period_information_2: one per-window record (HR, breathing, motion).
struct SleepPeriod
{
    double averageHr = 0;     // bpm
    double breath = 0;        // breaths per minute
    int motionCount = 0;
    int sleepState = 0;       // 0 awake, 1 asleep, 2 deep per the native range
};
std::optional<SleepPeriod> decodeSleepPeriod(const Bytes &body);

// activity_information: a state byte, then one MET value per minute.
struct ActivityMet
{
    int state = 0;
    std::vector<double> met;
};
std::optional<ActivityMet> decodeActivityMet(const Bytes &body);

// spo2_event: a header byte, then SpO2 percent per sample. A trailing 0xff is a
// "continued" sentinel rather than a sample.
std::vector<int> decodeSpo2(const Bytes &body);

// spo2_r_pi_event: header, then 3-byte samples of (R-ratio u16 big-endian
// / 16384, perfusion index u8). Oura's "SpO2 Simple" calibration turns R into a
// percentage: a + b*R + c*R^2, clamped to 85..100 for display.
struct Spo2Sample
{
    double r = 0;
    double perfusionIndex = 0;
};
std::vector<Spo2Sample> decodeSpo2RPi(const Bytes &body);
double spo2PercentFromR(double r);

// motion_event (0x47): the low five bits of the first byte are the seconds of
// motion in the window, the top three the orientation. Oura's sleep analysis
// takes the seconds, per 30-second epoch.
std::optional<int> decodeMotionSeconds(const Bytes &body);

// motion_period (0x6b): the ring's own motion state per 30-second epoch, which is what Oura's
// movement_30_sec and restless periods are made of. The first body byte is a header (bits 7-6
// the period type, bits 5-4 how many codes the last byte carries, 0 meaning all four, bits 3-0
// a rolling counter); the codes follow, two bits each, four to a byte, most significant first:
// 0 no motion, 1 restless, 2 tossing and turning, 3 active. (Layout as re-derived from captures
// by NOOP's notes and open_ring; the older reading with a 12-bit period dropped the first code
// byte.) The time axis is not in the event: the codes are taken to end at its timestamp.
struct MotionPeriod
{
    int periodType = 0;
    int sequence = 0;
    std::vector<uint8_t> states;
};
std::optional<MotionPeriod> decodeMotionPeriod(const Bytes &body);

// time_sync (0x42) and rtc_beacon (0x85): the ring's own record of the wall
// clock at a given ring time. These anchor the ring's rebooting decisecond
// counter to UTC.
std::optional<uint32_t> decodeTimeSync(const Bytes &body);
std::optional<uint32_t> decodeRtcBeacon(const Bytes &body);

// debug_data (0x61), battery subtype 0x24: percent and millivolts.
struct BatteryLog
{
    int percent = 0;
    int millivolts = 0;
};
std::optional<BatteryLog> decodeBatteryLog(const Bytes &body);

} // namespace Oura

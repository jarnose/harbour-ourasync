// g++ -std=c++17 -Wall -Wextra -I ../src ../src/ring/aes128.cpp ../src/ring/protocol.cpp ../src/ring/events.cpp ../src/analysis/ringclock.cpp ../src/analysis/sleepscore.cpp ../src/analysis/daysummary.cpp ../src/analysis/cloudmerge.cpp ../src/cloud/json.cpp ../src/cloud/csv.cpp ../src/cloud/clouddoc.cpp test_cloudmerge.cpp -o /tmp/cm && /tmp/cm
//
// An Oura export written out by hand in the shape its files have: semicolons,
// JSON cells, a wrapped line, a short row. No real person's data.
#include "hex.h"
#include "analysis/cloudmerge.h"

#include <cmath>

using namespace Oura;

static const char *kDailySleep =
    "id;contributors;day;score;timestamp\n"
    "ds1;{\"deep_sleep\":85,\"efficiency\":96,\"latency\":92,\"rem_sleep\":97,\"restfulness\":73,\"timing\":99,\"total_sleep\":83};2026-10-14;82;2026-10-14T00:00:00+00:00\n";

static const char *kReadiness =
    "id;contributors;day;score;temperature_deviation;temperature_trend_deviation\n"
    "dr1;{\"activity_balance\":80,\"hrv_balance\":88};2026-10-14;79;-0.31;0.02\n";

static const char *kActivity =
    "id;class_5_min;score;active_calories;steps;total_calories;day;contributors;met;low_activity_time;medium_activity_time;high_activity_time;sedentary_time;average_met_minutes\n"
    "da1;1122;81;420;9876;2410;2026-10-13;{\"meet_daily_targets\":90,\"move_every_hour\":70};{\"interval\":60,\"items\":[1.0,1.0,4.0,4.0],\"timestamp\":\"2026-10-13T00:00:00+03:00\"};3600;1800;600;30000;1.6\n";

static const char *kSleepModel =
    "id;average_breath;average_heart_rate;average_hrv;bedtime_end;bedtime_start;day;deep_sleep_duration;efficiency;heart_rate;hrv;latency;light_sleep_duration;lowest_heart_rate;movement_30_sec;period;rem_sleep_duration;restless_periods;sleep_phase_5_min;time_in_bed;total_sleep_duration;type;awake_time\n"
    "sm1;14.5;55.2;48;2026-10-14T07:10:00+03:00;2026-10-13T23:10:00+03:00;2026-10-14;4800;92;{\"interval\":300,\"items\":[60,58,null,55],\"timestamp\":\"2026-10-13T23:10:00+03:00\"};{\"interval\":300,\"items\":[40,45,50,52],\"timestamp\":\"2026-10-13T23:10:00+03:00\"};720;14400;49;112;0;5400;30;44112233;28800;24600;long_sleep;2700\n";

static AnalysisOptions opts()
{
    AnalysisOptions o;
    o.utcOffset = [](int64_t) { return 3 * 3600; };
    return o;
}

static CloudImport importAll()
{
    CloudImport in;
    addCsvFile(in, "export/dailysleep.csv", kDailySleep);
    addCsvFile(in, "export/dailyreadiness.csv", kReadiness);
    addCsvFile(in, "export/dailyactivity.csv", kActivity);
    addCsvFile(in, "export/sleepmodel.csv", kSleepModel);
    addCsvFile(in, "export/heartrate.csv",
               "timestamp;bpm;source\n2026-10-13T06:00:00+03:00;60;rest\n2026-10-13T06:05:00+03:00;64;rest\n2026-10-13T06:10:00+03:00;300;bad\n");
    return in;
}

static void times()
{
    CHECK_EQ(*parseIso8601("1970-01-01T00:00:00Z"), int64_t(0));
    CHECK_EQ(*parseIso8601("1970-01-01T03:00:00+03:00"), int64_t(0));
    CHECK_EQ(*parseIso8601("1970-01-01T00:00:00-02:00"), int64_t(7200));
    CHECK_EQ(*parseIso8601("2026-10-14T07:10:00.123+03:00"), *parseIso8601("2026-10-14T04:10:00Z"));
    CHECK_EQ(*parseIso8601("2026-10-14"), *parseIso8601("2026-10-14T00:00:00Z"));
    CHECK(!parseIso8601("nonsense").has_value());
    CHECK_EQ(isoOffsetSeconds("2026-10-14T07:10:00+03:00"), 10800);
    CHECK_EQ(isoOffsetSeconds("2026-10-14T07:10:00Z"), 0);
}

static void importing()
{
    CloudImport in = importAll();
    CHECK_EQ(in.docs.size(), size_t(4));
    CHECK_EQ(in.hr.size(), size_t(2)); // 300 bpm refused
    CHECK(in.notes.size() >= 5);
    CHECK_EQ(in.docs[0].kind, std::string("daily_sleep"));
    CHECK_EQ(in.docs[0].day, std::string("2026-10-14"));
    CHECK_EQ(in.docs[0].json.get("contributors")->num("timing"), 99.0);
    // The activity file's short-row rule is not triggered here (full row).
    CHECK_EQ(in.docs[2].json.num("steps"), 9876.0);

    // API page: same documents, with pagination.
    auto reply = Json::parse(R"({"data":[{"id":"x","day":"2026-10-12","score":70}],"next_token":"abc"})");
    CloudImport api;
    CHECK_EQ(addApiPage(api, "daily_sleep", *reply), std::string("abc"));
    CHECK_EQ(api.docs.size(), size_t(1));
    auto last = Json::parse(R"({"data":[],"next_token":null})");
    CHECK_EQ(addApiPage(api, "daily_sleep", *last), std::string(""));
}

static void cloudOnlyHistory()
{
    CloudImport in = importAll();
    Summary s; // no ring data at all: history before the first sync
    mergeCloud(s, in.docs, in.hr, opts());

    CHECK_EQ(s.nights.size(), size_t(1));
    const Night &n = s.nights[0];
    CHECK(n.fromCloud);
    CHECK(n.main);
    CHECK_EQ(n.date, std::string("2026-10-14"));
    CHECK_EQ(n.endUnix - n.startUnix, int64_t(8 * 3600));
    CHECK(std::fabs(n.asleepMin - 410.0) < 0.01);
    CHECK(n.staged);
    CHECK_EQ(n.epochSeconds, 300);
    CHECK_EQ(n.hypnogram.size(), size_t(8));
    // "44112233": awake, awake, deep, deep, light, light, REM, REM.
    CHECK_EQ(int(n.hypnogram[0]), int(StageAwake));
    CHECK_EQ(int(n.hypnogram[2]), int(StageDeep));
    CHECK_EQ(int(n.hypnogram[4]), int(StageLight));
    CHECK_EQ(int(n.hypnogram[6]), int(StageRem));
    CHECK(n.hr.size() == 3);                          // the null sample is skipped
    CHECK(n.restingHr.has_value() && *n.restingHr == 49);
    CHECK(n.avgHrv.has_value() && *n.avgHrv == 48);
    CHECK(n.efficiencyPct.has_value() && *n.efficiencyPct == 92);
    CHECK(n.latencyMin.has_value() && std::fabs(*n.latencyMin - 12.0) < 1e-9);
    // Official score and contributors ride along; the estimate is this app's own.
    CHECK(n.ouraScore.has_value() && *n.ouraScore == 82);
    CHECK_EQ(n.ouraContributors.size(), size_t(7));
    CHECK(n.score.valid);
    // Temperature deviation arrives from the readiness document.
    CHECK(n.tempDeviation.has_value() && std::fabs(*n.tempDeviation + 0.31) < 1e-9);

    // Days: activity from 13th, readiness on the 14th.
    CHECK_EQ(s.days.size(), size_t(2));
    CHECK_EQ(s.days[0].date, std::string("2026-10-13"));
    CHECK(s.days[0].fromCloud);
    CHECK(s.days[0].steps.has_value() && *s.days[0].steps == 9876);
    CHECK(s.days[0].ouraActivityScore.has_value() && *s.days[0].ouraActivityScore == 81);
    CHECK_EQ(s.days[0].activeKcal, 420.0);
    CHECK_EQ(s.days[0].moderateMin, 30);
    CHECK(std::fabs(s.days[0].metByHour[0] - 2.5) < 1e-9);   // (1+1+4+4)/4 in hour 0
    // Heart rate by local hour from the export's heartrate.csv (06:00 local).
    CHECK_EQ(s.days[0].hr[6].count, 2);
    CHECK_EQ(s.days[1].date, std::string("2026-10-14"));
    CHECK(s.days[1].readinessScore.has_value() && *s.days[1].readinessScore == 79);
}

static void ringTakesPrecedence()
{
    CloudImport in = importAll();
    Summary s;
    Night ring;
    ring.startUnix = *parseIso8601("2026-10-13T23:30:00+03:00");
    ring.endUnix = *parseIso8601("2026-10-14T06:50:00+03:00");
    ring.date = "2026-10-14";
    ring.asleepMin = 400;
    ring.main = true;
    s.nights.push_back(ring);
    Day day;
    day.date = "2026-10-13";
    day.activeKcal = 111;
    day.hr.assign(24, HourStat());
    day.metByHour.assign(24, 0.0);
    s.days.push_back(day);

    mergeCloud(s, in.docs, in.hr, opts());
    // The cloud night overlaps the ring's: not added twice.
    CHECK_EQ(s.nights.size(), size_t(1));
    CHECK(!s.nights[0].fromCloud);
    // But Oura's score is attached to the ring's night.
    CHECK(s.nights[0].ouraScore.has_value() && *s.nights[0].ouraScore == 82);
    // And the ring's day keeps its own kcal while gaining Oura's steps and score.
    CHECK_EQ(s.days[0].activeKcal, 111.0);
    CHECK(!s.days[0].fromCloud);
    CHECK(s.days[0].steps.has_value() && *s.days[0].steps == 9876);
}

static void nothingToMerge()
{
    Summary s;
    mergeCloud(s, {}, {}, opts());
    CHECK(s.nights.empty());
    CHECK(s.days.empty());
    // Garbage documents are ignored, not fatal.
    CloudImport in;
    addCsvFile(in, "dailysleep.csv", "id;day;score\n;;\nx;not a date;5\n");
    Summary t;
    mergeCloud(t, in.docs, in.hr, opts());
    CHECK(t.nights.empty());
}

int main()
{
    times();
    importing();
    cloudOnlyHistory();
    ringTakesPrecedence();
    nothingToMerge();
    return finish("cloudmerge");
}

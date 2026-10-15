#pragma once

#include "csv.h"
#include "json.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Oura's cloud documents, from either of the two ways to get them:
//  * the data export (membership.ouraring.com/data-export): a ZIP of CSV files,
//    available to every ring owner, no API access or membership needed;
//  * the Oura Cloud API v2 (api.ouraring.com/v2/usercollection/...): JSON.
// Both name their fields the same way (the export is the API's documents written
// out as CSV), so both become the same CloudDoc.
namespace Oura {

struct CloudDoc
{
    std::string kind; // daily_sleep, daily_readiness, daily_activity, sleep, ...
    std::string id;   // the document's own id, or kind:day when it has none
    std::string day;  // YYYY-MM-DD, the calendar day Oura files it under
    Json json;
};

struct CloudHrPoint
{
    int64_t utc = 0;
    int bpm = 0;
};

struct CloudImport
{
    std::vector<CloudDoc> docs;
    std::vector<CloudHrPoint> hr;
    // What was recognised, for the person who imported: "dailysleep.csv: 214 rows".
    std::vector<std::string> notes;
};

// Document kinds, in the API's own spelling.
namespace CloudKind {
constexpr const char *DailySleep = "daily_sleep";
constexpr const char *DailyReadiness = "daily_readiness";
constexpr const char *DailyActivity = "daily_activity";
constexpr const char *Sleep = "sleep";
constexpr const char *DailySpo2 = "daily_spo2";
constexpr const char *DailyStress = "daily_stress";
constexpr const char *DailyResilience = "daily_resilience";
constexpr const char *CardiovascularAge = "daily_cardiovascular_age";
constexpr const char *Workout = "workout";
constexpr const char *Session = "session";
constexpr const char *Tag = "tag";
constexpr const char *SleepTime = "sleep_time";
} // namespace CloudKind

// The document kind a file of the export carries, empty for files not used
// ("dailysleep.csv" -> daily_sleep). `lowerName` has no directory.
std::string kindForCsvName(const std::string &lowerName);

// Adds the rows of one export file. Heart rate goes to `hr`, everything else to
// `docs`. Rows that cannot be dated are skipped and counted in a note.
void addCsvFile(CloudImport &out, const std::string &fileName, const std::string &text);

// Adds the documents of one API reply `{"data":[...],"next_token":...}`. Returns
// the next page token, empty when the reply was the last page.
std::string addApiPage(CloudImport &out, const std::string &kind, const Json &reply);

// "2026-10-14T23:10:12+03:00", "...Z", with or without fractional seconds, or a
// bare date (midnight UTC).
std::optional<int64_t> parseIso8601(const std::string &text);

// Seconds east of UTC written in an ISO timestamp, 0 when there is none.
int isoOffsetSeconds(const std::string &text);

} // namespace Oura

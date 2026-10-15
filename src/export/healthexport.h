#pragma once

#include "analysis/daysummary.h"

#include <functional>
#include <string>

// What leaves the app: the nights and days it holds, as a FHIR R4 Bundle (JSON) and as CSV.
//
// Nothing here knows about files or Qt. FHIR is the format for another health system to read;
// the CSV files are for a spreadsheet. Both carry the figures with their provenance: measured
// by the ring, taken from Oura's cloud, or estimated here, and the Oura score and this app's
// own estimate are never the same field.
namespace Oura {

struct ExportOptions
{
    // Seconds east of UTC at an instant, for timestamps with their offset.
    std::function<int(int64_t)> utcOffset = [](int64_t) { return 0; };
    int64_t generatedUnix = 0;
};

// A FHIR R4 Bundle of type "collection": one Device, a Patient with no personal details, and
// Observations for every night (sleep duration with its stages, scores and efficiency as
// components; resting heart rate, HRV, breathing, SpO2 and temperature deviation on their own)
// and every day (steps, calories, activity and readiness scores).
std::string fhirBundle(const Summary &summary, const ExportOptions &options);

// One row per sleep period / per day, header first, comma-separated, point as decimal mark.
std::string nightsCsv(const Summary &summary, const ExportOptions &options);
std::string daysCsv(const Summary &summary);

// "YYYY-MM-DDTHH:MM:SS+HH:MM" for an instant at an offset.
std::string isoTimestamp(int64_t utc, int offsetSeconds);

} // namespace Oura

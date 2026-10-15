#include "healthexport.h"

#include "cloud/json.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <locale>
#include <sstream>

namespace Oura {

namespace {

const char *const kLocalSystem = "https://github.com/jarnose/harbour-ourasync/CodeSystem/measure";
const char *const kLoinc = "http://loinc.org";
const char *const kUcum = "http://unitsofmeasure.org";
const char *const kCategory = "http://terminology.hl7.org/CodeSystem/observation-category";

std::string number(double v, int decimals)
{
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out.setf(std::ios::fixed);
    out.precision(decimals);
    out << v;
    return out.str();
}

Json coding(const char *system, const std::string &code, const std::string &display)
{
    Json c = Json::object();
    c.set("system", Json::string(system));
    c.set("code", Json::string(code));
    c.set("display", Json::string(display));
    Json list = Json::array();
    list.push(std::move(c));
    Json codeable = Json::object();
    codeable.set("coding", std::move(list));
    codeable.set("text", Json::string(display));
    return codeable;
}

Json quantity(double value, const std::string &unit, const std::string &ucum)
{
    Json q = Json::object();
    q.set("value", Json::number(value));
    q.set("unit", Json::string(unit));
    q.set("system", Json::string(kUcum));
    q.set("code", Json::string(ucum));
    return q;
}

Json category(const char *code)
{
    Json c = Json::object();
    c.set("system", Json::string(kCategory));
    c.set("code", Json::string(code));
    Json list = Json::array();
    list.push(std::move(c));
    Json codeable = Json::object();
    codeable.set("coding", std::move(list));
    Json outer = Json::array();
    outer.push(std::move(codeable));
    return outer;
}

Json reference(const std::string &to)
{
    Json r = Json::object();
    r.set("reference", Json::string(to));
    return r;
}

// An Observation with the parts every one of them shares.
Json observation(const std::string &id, const char *cat, Json code, const std::string &note)
{
    Json o = Json::object();
    o.set("resourceType", Json::string("Observation"));
    o.set("id", Json::string(id));
    o.set("status", Json::string("final"));
    o.set("category", category(cat));
    o.set("code", std::move(code));
    o.set("subject", reference("Patient/owner"));
    o.set("device", reference("Device/oura-ring"));
    if (!note.empty()) {
        Json n = Json::object();
        n.set("text", Json::string(note));
        Json list = Json::array();
        list.push(std::move(n));
        o.set("note", std::move(list));
    }
    return o;
}

Json entry(Json resource)
{
    Json e = Json::object();
    const std::string type = resource.str("resourceType");
    // An absolute base lets the relative references (Patient/owner) resolve inside the bundle.
    e.set("fullUrl", Json::string("http://localhost/fhir/" + type + "/" + resource.str("id")));
    e.set("resource", std::move(resource));
    return e;
}

Json component(const std::string &code, const std::string &display, Json value, const char *valueKey)
{
    Json c = Json::object();
    c.set("code", coding(kLocalSystem, code, display));
    c.set(valueKey, std::move(value));
    return c;
}

std::string sourceNote(const Night &n)
{
    return n.fromCloud ? "From Oura's cloud." : "Read from the ring over Bluetooth; stages and scores follow the ring's own analysis.";
}

void addVital(Json &entries, const Night &n, const std::string &key, const char *loinc, const std::string &display,
              double value, const std::string &unit, const std::string &ucum, const std::string &when, const std::string &note)
{
    Json o = observation("n" + std::to_string(n.startUnix) + "-" + key, "vital-signs",
                         loinc ? coding(kLoinc, loinc, display) : coding(kLocalSystem, key, display), note);
    o.set("effectiveDateTime", Json::string(when));
    o.set("valueQuantity", quantity(value, unit, ucum));
    entries.push(entry(std::move(o)));
}

} // namespace

std::string isoTimestamp(int64_t utc, int offsetSeconds)
{
    const int64_t local = utc + offsetSeconds;
    int64_t days = local / 86400;
    int64_t rem = local % 86400;
    if (rem < 0) {
        rem += 86400;
        --days;
    }
    // Civil date from days since 1970-01-01 (Howard Hinnant's algorithm).
    const int64_t z = days + 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const int64_t y = static_cast<int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned d = doy - (153 * mp + 2) / 5 + 1;
    const unsigned m = mp < 10 ? mp + 3 : mp - 9;
    const int64_t year = y + (m <= 2 ? 1 : 0);
    const int off = std::min(std::abs(offsetSeconds), 24 * 3600 - 1);
    char buf[80];
    std::snprintf(buf, sizeof buf, "%04lld-%02u-%02uT%02d:%02d:%02d%c%02d:%02d", static_cast<long long>(year), m, d,
                  static_cast<int>(rem / 3600), static_cast<int>(rem % 3600 / 60), static_cast<int>(rem % 60),
                  offsetSeconds < 0 ? '-' : '+', off / 3600, off % 3600 / 60);
    return buf;
}

std::string fhirBundle(const Summary &summary, const ExportOptions &options)
{
    Json entries = Json::array();

    Json patient = Json::object();
    patient.set("resourceType", Json::string("Patient"));
    patient.set("id", Json::string("owner"));
    entries.push(entry(std::move(patient)));

    Json device = Json::object();
    device.set("resourceType", Json::string("Device"));
    device.set("id", Json::string("oura-ring"));
    Json names = Json::array();
    Json name = Json::object();
    name.set("name", Json::string("Oura ring"));
    name.set("type", Json::string("model-name"));
    names.push(std::move(name));
    device.set("deviceName", std::move(names));
    device.set("manufacturer", Json::string("Oura Health Oy"));
    entries.push(entry(std::move(device)));

    for (const Night &n : summary.nights) {
        const int offset = options.utcOffset(n.startUnix);
        const std::string start = isoTimestamp(n.startUnix, offset);
        const std::string end = isoTimestamp(n.endUnix, options.utcOffset(n.endUnix));

        Json sleep = observation("n" + std::to_string(n.startUnix), "activity",
                                 coding(kLoinc, "93832-4", "Sleep duration"),
                                 sourceNote(n) + (n.main ? "" : " A nap."));
        Json period = Json::object();
        period.set("start", Json::string(start));
        period.set("end", Json::string(end));
        sleep.set("effectivePeriod", std::move(period));
        sleep.set("valueQuantity", quantity(std::round(n.asleepMin * 10) / 10.0, "min", "min"));

        Json parts = Json::array();
        auto minutes = [&](const char *code, const char *display, double v) {
            parts.push(component(code, display, quantity(std::round(v * 10) / 10.0, "min", "min"), "valueQuantity"));
        };
        minutes("in_bed", "Time in bed", n.inBedMin);
        if (n.staged) {
            minutes("deep_sleep", "Deep sleep duration", n.deepMin);
            minutes("light_sleep", "Light sleep duration", n.lightMin);
            minutes("rem_sleep", "REM sleep duration", n.remMin);
            minutes("awake", "Awake time in bed", n.awakeMin);
        }
        if (n.efficiencyPct)
            parts.push(component("efficiency", "Sleep efficiency", quantity(*n.efficiencyPct, "%", "%"), "valueQuantity"));
        if (n.latencyMin)
            minutes("latency", "Time to fall asleep", *n.latencyMin);
        if (n.ouraScore)
            parts.push(component("oura_sleep_score", "Oura sleep score (Oura's own)",
                                 quantity(*n.ouraScore, "score", "{score}"), "valueQuantity"));
        if (n.score.valid)
            parts.push(component("estimated_sleep_score", "Sleep score estimated by Oura Sync, not Oura's",
                                 quantity(n.score.score, "score", "{score}"), "valueQuantity"));
        sleep.set("component", std::move(parts));
        entries.push(entry(std::move(sleep)));

        const std::string when = start;
        const std::string note = sourceNote(n);
        if (n.restingHr)
            addVital(entries, n, "resting_hr", "40443-4", "Heart rate --resting", *n.restingHr, "beats/min", "/min", when, note);
        if (n.avgHrv)
            addVital(entries, n, "hrv", "80404-7", "Heart rate variability (RMSSD, average of the night)", *n.avgHrv, "ms", "ms", when,
                     note + " Oura reports RMSSD; the code names the closest LOINC concept.");
        if (n.avgBreath)
            addVital(entries, n, "breathing", "9279-1", "Respiratory rate", *n.avgBreath, "breaths/min", "/min", when, note);
        if (n.avgSpo2)
            addVital(entries, n, "spo2", "59408-5", "Oxygen saturation by pulse oximetry", *n.avgSpo2, "%", "%", when, note);
        if (n.tempDeviation)
            addVital(entries, n, "skin_temp_deviation", nullptr, "Skin temperature deviation from baseline", *n.tempDeviation, "Cel", "Cel", when, note);
    }

    for (const Day &d : summary.days) {
        const std::string day = d.date;
        auto dayObservation = [&](const char *key, const char *loinc, const std::string &display, double value,
                                  const std::string &unit, const std::string &ucum) {
            Json o = observation("d" + day + "-" + key, "activity",
                                 loinc ? coding(kLoinc, loinc, display) : coding(kLocalSystem, key, display),
                                 d.fromCloud ? "From Oura's cloud." : "Estimated from the ring's movement data.");
            o.set("effectiveDateTime", Json::string(day));
            o.set("valueQuantity", quantity(value, unit, ucum));
            entries.push(entry(std::move(o)));
        };
        if (d.steps)
            dayObservation("steps", "41950-7", "Number of steps in 24 hour Measured", *d.steps, "steps", "{steps}");
        if (d.totalKcal)
            dayObservation("total_calories", "41981-2", "Calories burned", *d.totalKcal, "kcal", "kcal");
        if (d.activeKcal > 0)
            dayObservation("active_calories", nullptr, "Active calories", std::round(d.activeKcal), "kcal", "kcal");
        if (d.ouraActivityScore)
            dayObservation("oura_activity_score", nullptr, "Oura activity score (Oura's own)", *d.ouraActivityScore, "score", "{score}");
        if (d.readinessScore)
            dayObservation("oura_readiness_score", nullptr, "Oura readiness score (Oura's own)", *d.readinessScore, "score", "{score}");
    }

    Json bundle = Json::object();
    bundle.set("resourceType", Json::string("Bundle"));
    bundle.set("type", Json::string("collection"));
    if (options.generatedUnix > 0)
        bundle.set("timestamp", Json::string(isoTimestamp(options.generatedUnix, 0)));
    bundle.set("entry", std::move(entries));
    return bundle.dump();
}

namespace {

std::string field(const std::optional<double> &v, int decimals = 1)
{
    return v ? number(*v, decimals) : std::string();
}

} // namespace

std::string nightsCsv(const Summary &summary, const ExportOptions &options)
{
    std::string out = "date,start,end,source,main,in_bed_min,asleep_min,deep_min,light_min,rem_min,awake_min,"
                      "efficiency_pct,latency_min,resting_hr,avg_hrv_ms,avg_breath,avg_spo2,skin_temp_c,"
                      "temp_deviation_c,oura_score,estimated_score\n";
    for (const Night &n : summary.nights) {
        out += n.date + ',' + isoTimestamp(n.startUnix, options.utcOffset(n.startUnix)) + ',' +
               isoTimestamp(n.endUnix, options.utcOffset(n.endUnix)) + ',' + (n.fromCloud ? "oura_cloud" : "ring") + ',' +
               (n.main ? "1" : "0") + ',' + number(n.inBedMin, 1) + ',' + number(n.asleepMin, 1) + ',';
        out += n.staged ? number(n.deepMin, 1) + ',' + number(n.lightMin, 1) + ',' + number(n.remMin, 1) + ',' + number(n.awakeMin, 1)
                        : std::string(",,,");
        out += ',' + field(n.efficiencyPct, 0) + ',' + field(n.latencyMin) + ',' + field(n.restingHr, 0) + ',' + field(n.avgHrv, 0) + ',' +
               field(n.avgBreath) + ',' + field(n.avgSpo2) + ',' + field(n.skinTemp, 2) + ',' + field(n.tempDeviation, 2) + ',' +
               (n.ouraScore ? std::to_string(*n.ouraScore) : std::string()) + ',' +
               (n.score.valid ? number(n.score.score, 0) : std::string()) + '\n';
    }
    return out;
}

std::string daysCsv(const Summary &summary)
{
    std::string out = "date,source,steps,total_kcal,active_kcal,avg_met,light_min,moderate_min,vigorous_min,sedentary_min,"
                      "oura_activity_score,oura_readiness_score,temp_deviation_c,stress_high_min,recovery_high_min\n";
    for (const Day &d : summary.days) {
        out += d.date + ',' + (d.fromCloud ? "oura_cloud" : "ring") + ',' + (d.steps ? std::to_string(*d.steps) : std::string()) + ',' +
               (d.totalKcal ? std::to_string(*d.totalKcal) : std::string()) + ',' + number(d.activeKcal, 0) + ',' + field(d.avgMet, 2) + ',' +
               std::to_string(d.lightMin) + ',' + std::to_string(d.moderateMin) + ',' + std::to_string(d.vigorousMin) + ',' +
               std::to_string(d.sedentaryMin) + ',' + (d.ouraActivityScore ? std::to_string(*d.ouraActivityScore) : std::string()) + ',' +
               (d.readinessScore ? std::to_string(*d.readinessScore) : std::string()) + ',' + field(d.temperatureDeviation, 2) + ',' +
               (d.stressHighMin ? std::to_string(*d.stressHighMin) : std::string()) + ',' +
               (d.recoveryHighMin ? std::to_string(*d.recoveryHighMin) : std::string()) + '\n';
    }
    return out;
}

} // namespace Oura

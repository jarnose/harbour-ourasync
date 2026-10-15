// g++ -std=c++17 -Wall -Wextra -I ../src ../src/export/healthexport.cpp ../src/cloud/json.cpp ../src/analysis/{daysummary,ringclock,sleepscore,oura_sleep_state}.cpp ../src/ring/{protocol,events,aes128}.cpp test_healthexport.cpp -o /tmp/he && /tmp/he
#include "hex.h"
#include "cloud/json.h"
#include "export/healthexport.h"

#include <clocale>

using namespace Oura;

static Summary sample()
{
    Summary s;
    Night n;
    n.startUnix = 1760000000;            // 2025-10-09 08:53:20 UTC
    n.endUnix = n.startUnix + 7 * 3600;
    n.date = "2025-10-09";
    n.staged = true;
    n.inBedMin = 420;
    n.asleepMin = 392.5;
    n.deepMin = 70;
    n.lightMin = 200;
    n.remMin = 122.5;
    n.awakeMin = 27.5;
    n.efficiencyPct = 93;
    n.latencyMin = 12;
    n.restingHr = 51;
    n.avgHrv = 38;
    n.ouraScore = 84;
    n.fromCloud = true;
    s.nights.push_back(n);
    Day d;
    d.date = "2025-10-09";
    d.steps = 9876;
    d.totalKcal = 2410;
    d.ouraActivityScore = 81;
    d.fromCloud = true;
    s.days.push_back(d);
    return s;
}

int main()
{
    // A comma-decimal locale must not leak into the files.
    for (const char *name : { "fi_FI.UTF-8", "de_DE.UTF-8", "fi_FI", "de_DE" })
        if (std::setlocale(LC_ALL, name))
            break;

    ExportOptions options;
    options.utcOffset = [](int64_t) { return 3 * 3600; };
    options.generatedUnix = 1760000000;
    const Summary s = sample();

    CHECK_EQ(isoTimestamp(1760000000, 3 * 3600), std::string("2025-10-09T11:53:20+03:00"));
    CHECK_EQ(isoTimestamp(0, -5 * 3600 - 1800), std::string("1969-12-31T18:30:00-05:30"));

    const auto bundle = Json::parse(fhirBundle(s, options));
    CHECK(bundle.has_value());
    if (!bundle)
        return finish("healthexport");
    CHECK_EQ(bundle->str("resourceType"), std::string("Bundle"));
    CHECK_EQ(bundle->str("type"), std::string("collection"));
    const Json *entries = bundle->get("entry");
    CHECK(entries && entries->isArray());
    // Patient, Device, the sleep, resting HR, HRV, steps, calories, activity score.
    CHECK_EQ(entries->items().size(), size_t(8));
    bool sleep = false, steps = false;
    for (const Json &e : entries->items()) {
        const Json *r = e.get("resource");
        const Json *code = r ? r->get("code") : nullptr;
        if (!code)
            continue;
        const std::string loinc = code->get("coding")->items()[0].str("code");
        if (loinc == "93832-4") {
            sleep = true;
            CHECK_EQ(r->get("valueQuantity")->num("value"), 392.5);
            CHECK_EQ(r->get("effectivePeriod")->str("start"), std::string("2025-10-09T11:53:20+03:00"));
            // Oura's score and no estimate (the sample has none).
            bool official = false;
            for (const Json &c : r->get("component")->items())
                if (c.get("code")->get("coding")->items()[0].str("code") == "oura_sleep_score")
                    official = c.get("valueQuantity")->num("value") == 84;
            CHECK(official);
        }
        if (loinc == "41950-7") {
            steps = true;
            CHECK_EQ(r->get("valueQuantity")->num("value"), 9876.0);
        }
    }
    for (const Json &e : entries->items())
        CHECK(e.get("resource")->str("id").find('_') == std::string::npos);
    CHECK(sleep);
    CHECK(steps);

    const std::string nights = nightsCsv(s, options);
    CHECK(nights.rfind("date,start,end,source", 0) == 0);
    CHECK(nights.find("2025-10-09,2025-10-09T11:53:20+03:00,2025-10-09T18:53:20+03:00,oura_cloud,1,420.0,392.5,70.0,200.0,122.5,27.5,93,12.0,51,38,,,,,84,") != std::string::npos);
    const std::string days = daysCsv(s);
    CHECK(days.find("2025-10-09,oura_cloud,9876,2410,0,,0,0,0,0,81,") != std::string::npos);
    return finish("healthexport");
}

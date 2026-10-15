// The app's own pipeline over an import from Oura's cloud, for compare_pipeline.py: reads
// documents from stdin (one per line: kind TAB id TAB day TAB json), merges them into a
// Summary exactly as the app does, computes each main night's score with the app's code and
// prints it beside Oura's own.
//
// Output per night: date officialScore computedScore then the seven official contributors
// (-1 where absent) and the seven computed ones, then complete and restfulnessEstimated.
//
// g++ -std=c++17 -I ../../src ../../src/analysis/{cloudmerge,daysummary,oura_night_score,oura_sleep_score,oura_sleep_state,ringclock,sleepscore}.cpp \
//     ../../src/cloud/{json,clouddoc,csv}.cpp ../../src/ring/{protocol,events,aes128}.cpp pipeline_cli.cpp -o /tmp/pipeline_cli
#include "analysis/cloudmerge.h"
#include "analysis/oura_night_score.h"
#include "cloud/clouddoc.h"
#include "cloud/json.h"

#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    const int age = argc > 1 ? std::atoi(argv[1]) : 44;
    const int ageSwitchDay = argc > 2 ? std::atoi(argv[2]) : 0;   // unused: the caller picks the age per file
    (void) ageSwitchDay;
    std::vector<Oura::CloudDoc> docs;
    std::string line;
    while (std::getline(std::cin, line)) {
        const size_t a = line.find('\t'), b = line.find('\t', a + 1), c = line.find('\t', b + 1);
        if (a == std::string::npos || b == std::string::npos || c == std::string::npos)
            continue;
        auto json = Oura::Json::parse(line.substr(c + 1));
        if (!json)
            continue;
        Oura::CloudDoc d;
        d.kind = line.substr(0, a);
        d.id = line.substr(a + 1, b - a - 1);
        d.day = line.substr(b + 1, c - b - 1);
        d.json = *json;
        docs.push_back(std::move(d));
    }
    Oura::Summary summary;
    Oura::AnalysisOptions options;
    options.ageYears = age;
    // Local time of the night: the bedtime strings carry their own offset, but the merge takes
    // the zone from the options; the analysed person lives at UTC+2/+3 (Finland, summer time).
    options.utcOffset = [](int64_t utc) {
        // EU summer time: last Sunday of March 01:00 UTC to last Sunday of October 01:00 UTC.
        const int64_t days = utc / 86400;
        const long z = static_cast<long>(days) + 719468;
        const long era = (z >= 0 ? z : z - 146096) / 146097;
        const unsigned doe = static_cast<unsigned>(z - era * 146097);
        const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        long y = static_cast<long>(yoe) + era * 400;
        const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const unsigned mp = (5 * doy + 2) / 153;
        const unsigned m = mp < 10 ? mp + 3 : mp - 9;
        if (m <= 2) ++y;
        auto lastSunday = [](long year, unsigned month) {
            // day of month of the last Sunday
            static const unsigned dim[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
            unsigned last = dim[month - 1];
            // weekday of the last day (0 = Sunday)
            const long zz = Oura::daysFromCivil(year, month, last);
            const int wd = static_cast<int>(((zz % 7) + 11) % 7);   // 1970-01-01 was a Thursday (4)
            return last - static_cast<unsigned>(wd);
        };
        const int64_t startDst = Oura::daysFromCivil(y, 3, lastSunday(y, 3)) * 86400 + 3600;
        const int64_t endDst = Oura::daysFromCivil(y, 10, lastSunday(y, 10)) * 86400 + 3600;
        return (utc >= startDst && utc < endDst) ? 3 * 3600 : 2 * 3600;
    };
    Oura::mergeCloud(summary, docs, {}, options);
    for (const Oura::Night &n : summary.nights) {
        if (!n.main || !n.ouraScore)
            continue;
        std::cout << n.date << ' ' << *n.ouraScore;
        const auto r = Oura::ouraScoreForNight(n, age, summary.nights);
        std::cout << ' ' << (r ? r->score : -1);
        static const char *const keys[7] = { "total_sleep", "deep_sleep", "rem_sleep", "efficiency", "latency", "restfulness", "timing" };
        for (const char *k : keys) {
            int v = -1;
            for (const auto &c : n.ouraContributors)
                if (c.first == k)
                    v = c.second;
            std::cout << ' ' << v;
        }
        for (int i = 0; i < 7; ++i)
            std::cout << ' ' << (r ? r->displayed[static_cast<size_t>(i)] : -1);
        std::cout << ' ' << (r && r->complete ? 1 : 0) << ' ' << (r && r->restfulnessEstimated ? 1 : 0) << '\n';
    }
}

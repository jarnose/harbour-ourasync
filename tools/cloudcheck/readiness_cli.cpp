// The app's readiness estimate over a cloud import, for compare_readiness.py: documents from
// stdin (kind TAB id TAB day TAB json), merged as the app does, then the estimate for every day,
// printed beside Oura's score and contributors.
// g++ -std=c++17 -I ../../src ../../src/analysis/{cloudmerge,daysummary,oura_night_score,oura_sleep_score,oura_sleep_state,ringclock,sleepscore,stager,activity_score,readiness_estimate}.cpp \
//     ../../src/cloud/{json,clouddoc,csv}.cpp ../../src/ring/{protocol,events,aes128}.cpp readiness_cli.cpp -o /tmp/readiness_cli
#include "analysis/cloudmerge.h"
#include "analysis/readiness_estimate.h"
#include "cloud/clouddoc.h"
#include "cloud/json.h"

#include <iostream>
#include <string>

int main(int argc, char **argv)
{
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
    options.ageYears = argc > 1 ? std::atoi(argv[1]) : 44;
    options.utcOffset = [](int64_t) { return 3 * 3600; };
    Oura::mergeCloud(summary, docs, {}, options);
    Oura::estimateReadiness(summary, options, true);
    for (const Oura::Day &d : summary.days) {
        if (!d.readinessScore || !d.estimatedReadinessScore)
            continue;
        std::cout << d.date << ' ' << *d.readinessScore << ' ' << *d.estimatedReadinessScore;
        for (int v : d.estimatedReadinessContributors)
            std::cout << ' ' << v;
        std::cout << '\n';
    }
}

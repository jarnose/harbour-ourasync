// Prints this repository's port of the app's score path (calculate_minutes, first set of
// limits), one case per line on stdin:
//   age total deep rem efficiency latencyEpochs wakeUps restless motionEvents midpoint type centre
// and writes the seven contributors the app shows.
// g++ -std=c++17 -I ../../src ../../src/analysis/oura_sleep_score.cpp diff_app_cli.cpp -o /tmp/diff_app_cli
#include "analysis/oura_sleep_score.h"

#include <iostream>

int main()
{
    Oura::OuraAppInput in;
    int events, restless;
    while (std::cin >> in.ageYears >> in.totalMinutes >> in.deepMinutes >> in.remMinutes >> in.efficiencyPercent
           >> in.latencyEpochs >> in.wakeUps >> restless >> events >> in.midpointMinutes
           >> in.chronotypeType >> in.chronotypeCenterMinutes) {
        in.restlessEpochs = restless;
        in.motionEvents = events;
        const Oura::OuraSleepScore s = Oura::ouraSleepScoreApp(in);
        for (size_t i = 0; i < 7; ++i)
            std::cout << (i ? " " : "") << s.displayed[i];
        std::cout << '\n';
    }
}

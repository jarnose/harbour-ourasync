// For motion_probe.py: for nights read from stdin, the state machine over (a) the ring's motion
// seconds per epoch and (b) the cloud's movement flags, and the displayed restfulness for every value
// of the third input ("motion events") from 0 to 255 with the second's wake-ups and restless epochs.
// Per case: age epochs startMinutes nStages nSeconds nFlags, then stages, seconds, flags.
// Output: wakeUps restlessFlags restlessSeconds eventsSeconds eventsFlags, then 256 restfulness values.
//
// g++ -std=c++17 -I ../../src ../../src/analysis/oura_sleep_state.cpp ../../src/analysis/oura_sleep_score.cpp restful_cli.cpp -o /tmp/restful_cli
#include "analysis/oura_sleep_score.h"
#include "analysis/oura_sleep_state.h"

#include <iostream>

int main()
{
    int age, epochs, tod, ns, nsec, nflag;
    while (std::cin >> age >> epochs >> tod >> ns >> nsec >> nflag) {
        Oura::OuraNightEpochs night;
        night.epochs = epochs;
        night.startMinutesOfDay = tod;
        for (int i = 0; i < ns; ++i) {
            int v;
            std::cin >> v;
            night.stages.push_back(static_cast<uint8_t>(v));
        }
        std::vector<uint8_t> seconds, flags;
        for (int i = 0; i < nsec; ++i) {
            int v;
            std::cin >> v;
            seconds.push_back(static_cast<uint8_t>(v));
        }
        for (int i = 0; i < nflag; ++i) {
            int v;
            std::cin >> v;
            flags.push_back(static_cast<uint8_t>(v));
        }
        Oura::OuraNightEpochs withSeconds = night, withFlags = night;
        withSeconds.motion = seconds;
        withFlags.motion = flags;
        const Oura::OuraNightSummary a = Oura::summariseNight(withSeconds);
        const Oura::OuraNightSummary b = Oura::summariseNight(withFlags);
        std::cout << b.wakeUps << ' ' << b.restlessEpochs << ' ' << a.restlessEpochs << ' ' << a.motionEvents << ' '
                  << b.motionEvents;
        for (int c = 0; c < 256; ++c) {
            Oura::OuraAppInput in;
            in.ageYears = age;
            const int asleep = b.deepEpochs + b.lightEpochs + b.remEpochs;
            in.totalMinutes = (asleep + 1) >> 1;
            in.deepMinutes = (b.deepEpochs + 1) >> 1;
            in.remMinutes = (b.remEpochs + 1) >> 1;
            in.efficiencyPercent = 80;
            in.latencyEpochs = b.latencyEpochs;
            in.midpointMinutes = b.midpointMinutes;
            in.wakeUps = b.wakeUps;
            in.restlessEpochs = b.restlessEpochs;
            in.motionEvents = c;
            std::cout << ' ' << Oura::ouraSleepScoreApp(in).displayed[5];
        }
        std::cout << '\n';
    }
}

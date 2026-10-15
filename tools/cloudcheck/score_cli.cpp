// Runs the sleep state machine and the score mapper of this repository over nights
// read from stdin, for tools/cloudcheck/compare_sleep.py.
// Per case: age epochs startMinutes chronoType chronoCenter nStages nMotion, then the
// stage codes, then the motion bytes.
// Output: score, 7 displayed contributors, then inBed deep light rem latency wakeUps restless events midpoint.
// g++ -std=c++17 -I ../../src ../../src/analysis/oura_sleep_state.cpp ../../src/analysis/oura_sleep_score.cpp score_cli.cpp -o /tmp/score_cli
#include "analysis/oura_sleep_score.h"
#include "analysis/oura_sleep_state.h"

#include <iostream>

int main()
{
    int age, epochs, tod, ctype, ccenter, ns, nm;
    while (std::cin >> age >> epochs >> tod >> ctype >> ccenter >> ns >> nm) {
        Oura::OuraNightEpochs night;
        night.epochs = epochs;
        night.startMinutesOfDay = tod;
        for (int i = 0; i < ns; ++i) {
            int v;
            std::cin >> v;
            night.stages.push_back(static_cast<uint8_t>(v));
        }
        for (int i = 0; i < nm; ++i) {
            int v;
            std::cin >> v;
            night.motion.push_back(static_cast<uint8_t>(v));
        }
        const Oura::OuraNightSummary s = Oura::summariseNight(night);
        Oura::OuraSleepInput in;
        in.ageYears = age;
        in.inBedEpochs = s.inBedEpochs;
        in.deepEpochs = s.deepEpochs;
        in.lightEpochs = s.lightEpochs;
        in.remEpochs = s.remEpochs;
        in.latencyEpochs = s.latencyEpochs;
        in.midpointMinutes = s.midpointMinutes;
        in.chronotypeType = ctype;
        in.chronotypeCenterMinutes = ccenter;
        if (nm > 0) {
            in.restfulA = s.wakeUps;
            in.restfulB = s.restlessEpochs;
            in.restfulC = s.motionEvents;
        }
        const Oura::OuraSleepScore r = Oura::ouraSleepScore(in);
        std::cout << r.score;
        for (int c : r.displayed)
            std::cout << ' ' << c;
        std::cout << ' ' << s.inBedEpochs << ' ' << s.deepEpochs << ' ' << s.lightEpochs << ' ' << s.remEpochs << ' '
                  << s.latencyEpochs << ' ' << s.wakeUps << ' ' << s.restlessEpochs << ' ' << s.motionEvents << ' '
                  << s.midpointMinutes << ' ' << (r.complete ? 1 : 0) << '\n';
    }
}

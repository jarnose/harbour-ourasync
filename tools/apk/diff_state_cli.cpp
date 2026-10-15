// Reads nights from stdin and prints the C++ state machine's summary.
// Per case: epochs startMinutes nStages nMotion, then the stage codes, then the motion bytes.
// Output: inBed deep light rem latency wakeUps restless events midpoint
// g++ -std=c++17 -I ../../src ../../src/analysis/oura_sleep_state.cpp diff_state_cli.cpp -o /tmp/diff_state_cli
#include "analysis/oura_sleep_state.h"

#include <iostream>

int main()
{
    int epochs, tod, ns, nm;
    while (std::cin >> epochs >> tod >> ns >> nm) {
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
        std::cout << s.inBedEpochs << ' ' << s.deepEpochs << ' ' << s.lightEpochs << ' ' << s.remEpochs << ' '
                  << s.latencyEpochs << ' ' << s.wakeUps << ' ' << s.restlessEpochs << ' ' << s.motionEvents << ' '
                  << s.midpointMinutes << '\n';
    }
}

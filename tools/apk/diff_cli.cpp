// Prints this repository's port of the sleep-score mapper for the native argument
// list, one case per line on stdin:
//   age w1 w2 w3 w4 w5 w6 w7 a8 a9 a10 a11
// and writes: score c0 c1 c2 c3 c4 c5 c6 c7 c8   (-1 where not computed)
// g++ -std=c++17 -I ../../src ../../src/analysis/oura_sleep_score.cpp diff_cli.cpp -o /tmp/diff_cli
#include "analysis/oura_sleep_score.h"

#include <iostream>

int main()
{
    int age, w1, w2, w3, w4, w5, w6, w7, a8, a9, a10, a11;
    while (std::cin >> age >> w1 >> w2 >> w3 >> w4 >> w5 >> w6 >> w7 >> a8 >> a9 >> a10 >> a11) {
        Oura::OuraSleepInput in;
        in.ageYears = age;
        in.inBedEpochs = w1 - 1;
        in.deepEpochs = w2;
        in.lightEpochs = w3;
        in.remEpochs = w4;
        in.latencyEpochs = w5;
        in.restfulA = w6;
        in.restfulB = w7;
        in.restfulC = a8;
        in.midpointMinutes = a9;
        in.chronotypeType = a10;
        in.chronotypeCenterMinutes = a11;
        const Oura::OuraSleepScore s = Oura::ouraSleepScore(in);
        std::cout << s.score;
        for (int c : s.contributor)
            std::cout << ' ' << c;
        for (int c : s.displayed)
            std::cout << ' ' << c;
        std::cout << '\n';
    }
}

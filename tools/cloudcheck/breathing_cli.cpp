// The app's breathing rate over beats read from stdin, for breathing_check.py: a first line
// "startUtc endUtc", then one "utcSeconds ibiMs" per line. Prints the rate or "none".
// g++ -std=c++17 -I ../../src ../../src/analysis/breathing.cpp breathing_cli.cpp -o /tmp/breathing_cli
#include "analysis/breathing.h"

#include <iostream>

int main()
{
    double start, end;
    std::cin >> start >> end;
    std::vector<Oura::BreathBeat> beats;
    double t;
    int ibi;
    while (std::cin >> t >> ibi)
        beats.push_back({ t, ibi });
    const auto r = Oura::breathingRate(beats, start, end);
    if (r)
        std::cout << *r << '\n';
    else
        std::cout << "none\n";
}

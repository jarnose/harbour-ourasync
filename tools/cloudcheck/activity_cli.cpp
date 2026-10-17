// The app's activity score over days read from stdin, for compare_activity.py.
// Per day, one line: active_kcal target_kcal then 1440 MET values ("nan" for none), oldest day first.
// Output per day from the seventh: the score and the six contributors.
// g++ -std=c++17 -I ../../src ../../src/analysis/activity_score.cpp activity_cli.cpp -o /tmp/activity_cli
#include "analysis/activity_score.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

int main()
{
    std::vector<Oura::ActivityDay> history;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        double kcal, target;
        in >> kcal >> target;
        std::vector<double> met;
        std::string tok;
        while (in >> tok)
            met.push_back(tok == "nan" ? std::nan("") : std::stod(tok));
        history.push_back(Oura::activityDayFromMinutes(met, kcal, target));
        if (history.size() >= 7) {
            const std::vector<Oura::ActivityDay> week(history.end() - 7, history.end());
            const auto s = Oura::activityScore(week);
            std::cout << s->score;
            for (int c : s->contributors)
                std::cout << ' ' << c;
            std::cout << '\n';
        } else {
            std::cout << "-1\n";
        }
    }
}

// The app's stress estimate over the days stress_fit.py --emit wrote, for stress_check.py: reads
// "date TAB truthSlots TAB nightHrv TAB priorHrv,... TAB slots,... TAB history,..." and prints
// "date truthSlots estimatedMinutes" ("-" where it makes none).
//
// g++ -std=c++17 -I ../../src ../../src/analysis/stress_estimate.cpp stress_cli.cpp -o /tmp/stress_cli
#include "analysis/stress_estimate.h"

#include <iostream>
#include <sstream>
#include <string>

static std::vector<double> numbers(const std::string &s)
{
    std::vector<double> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ','))
        if (!item.empty())
            out.push_back(std::stod(item));
    return out;
}

int main()
{
    std::string line;
    while (std::getline(std::cin, line)) {
        std::vector<std::string> f;
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, '\t'))
            f.push_back(item);
        f.resize(6);
        Oura::StressInput in;
        const double hrv = std::stod(f[2]);
        if (hrv >= 0)
            in.nightHrv = hrv;
        in.priorHrv = numbers(f[3]);
        in.today = numbers(f[4]);
        in.history = numbers(f[5]);
        const auto minutes = Oura::estimateStressMinutes(in);
        std::cout << f[0] << ' ' << f[1] << ' ' << (minutes ? std::to_string(*minutes) : std::string("-")) << '\n';
    }
}

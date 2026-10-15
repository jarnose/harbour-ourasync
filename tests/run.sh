#!/bin/sh
# Builds and runs every Qt-free test with plain g++, once per language mode.
#
# Both modes on purpose. -std=c++17 (strict) is what a bare g++ gives; the
# Sailfish build uses gnu++17, where `unix` is a predefined macro for 1 and
# turns any variable of that name into a compile error far from the cause. The
# first version of the analysis code had exactly that bug, and only the second
# mode would have caught it.
#
# Usage: tests/run.sh      (from anywhere)
set -e
cd "$(dirname "$0")"

CORE="../src/ring/aes128.cpp ../src/ring/protocol.cpp ../src/ring/events.cpp"
ANALYSIS="../src/analysis/ringclock.cpp ../src/analysis/sleepscore.cpp ../src/analysis/daysummary.cpp"

run() {
    name=$1; shift
    for std in c++17 gnu++17; do
        echo "== $name ($std)"
        g++ -std=$std -Wall -Wextra -Werror -I ../src "$@" "test_$name.cpp" -o "/tmp/ouras_$name"
        "/tmp/ouras_$name"
    done
}

run aes128 ../src/ring/aes128.cpp
run protocol $CORE
run session $CORE ../src/ring/session.cpp
run daysummary $CORE $ANALYSIS
run oura_sleep_score ../src/analysis/oura_sleep_score.cpp

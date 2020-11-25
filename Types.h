#pragma once

#include <chrono>

const int FULL_DEBUG = 2;
const int SOLVE_DEBUG = 1;
const int NO_DEBUG = 0;

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::time_point<Time> TimePoint;
typedef std::chrono::milliseconds ms;

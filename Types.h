#pragma once

#include <chrono>
#include <vector>

const int FULL_DEBUG = 2;
const int SOLVE_DEBUG = 1;
const int NO_DEBUG = 0;

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::time_point<Time> TimePoint;
typedef std::chrono::milliseconds ms;

struct Graph {
    int totalVertices; // общее количество вершин
    int haloVertices;
    int ownVertices; // количество своих вершин

    std::vector<int> l2g;
    std::vector<int> part;

    std::vector<int> ia;
    std::vector<int> ja;
    std::vector<double> a;
    std::vector<double> b;
};

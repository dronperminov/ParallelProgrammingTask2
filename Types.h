#pragma once

#include <chrono>
#include <vector>

const int FULL_DEBUG = 2;
const int SOLVE_DEBUG = 1;
const int NO_DEBUG = 0;

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::time_point<Time> TimePoint;
typedef std::chrono::milliseconds ms;

// параметры задачи
struct TaskParams {
    int nx, ny; // размеры сетки
    int k1, k2; // параметры разбиения
    int px, py; // количество областей для процессов
    double eps; // точность вычисления
    int debug; // уровень отладки
};

// структура для графа
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

// схема обменов
struct Communication {
    std::vector<int> neighbours;
    std::vector<int> sendOffset;
    std::vector<int> recvOffset;
    std::vector<int> send;
    std::vector<int> recv;
};

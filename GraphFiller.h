#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include "Types.h"

const double DIAGONAL_DOMINANCE_COEFFICIENT = 1.234; // коэффициент диагонального преобладания

// заполняющая функция для Aij
inline double Fa(int i, int j) {
    return cos(i*j + i + j);
}

// заполняющая функция для bi
inline double Fb(int i) {
    return sin(i);
}

class GraphFiller {
    bool debug; // нужна ли отладка
    std::ofstream &fout;

    void PrintDebug(const Graph& graph) const; // вывод отладочных значений
public:
    GraphFiller(std::ofstream &fout, bool debug);

    void Fill(Graph &graph) const; // заполнение
};
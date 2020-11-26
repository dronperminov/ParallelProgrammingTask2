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
    int ownVertices; // количество своих вершин
    int totalVertices; // общее количество вершин
    int *ia;
    int *ja;
    int *l2g;
    bool debug; // нужна ли отладка
    std::ofstream &fout;

    void PrintDebug(double *a, double *b) const; // вывод отладочных значений
public:
    GraphFiller(std::ofstream &fout, int totalVertices, int ownVertices, int *ia, int *ja, int *l2g, bool debug);

    void Fill(double *&a, double *&b) const; // заполнение
};
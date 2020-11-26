#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "VectorMath.h"
#include "Types.h"

struct Solvation {
    std::vector<double> x;
    int iterations;
    double res;
};

class ConjugateGradientSolver {
    double eps; // точность решения
    int debug; // нужна ли отладка

    double GetResidualNorm(const Graph &graph, const Communication &communication, std::vector<double> &x, std::vector<double> &r) const; // получение нормы невязки
    void InitValues(const Graph &graph, std::vector<double> &x, std::vector<double> &r, std::vector<double> &m) const; // инициализация начальных значений
    void PrintVector(std::ofstream &fout, const std::vector<double> &x) const; // вывод вектора
    void PrintDebug(const Solvation &solvation, const std::vector<double> &norms, const std::vector<double> &rho, int id) const; // вывод отладки
public:
    ConjugateGradientSolver(double eps, int debug);

    Solvation Solve(const Graph &graph, const Communication &communication); // решение системы
};
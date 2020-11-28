#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "Types.h"

class CommunicationGenerator {
    int processCount; // количество процессов
    bool debug; // нужна ли отладка

    void PrintVector(std::ofstream &fout, const std::vector<int> &vector) const; // вывод вектора
    void PrintVector(std::ofstream &fout, const std::vector<int> &vector, const std::vector<int> &l2g) const; // вывод вектора с глобальными вершинами
    void PrintDebug(const Graph &graph, const Communication &communication, std::vector<std::vector<int>> sendToProcess, std::vector<std::vector<int>> recvFromProcess, double time) const; // вывод отладочной информации
public:
    CommunicationGenerator(int processCount, bool debug);

    Communication Build(const Graph &graph); // построение схемы обменов
};
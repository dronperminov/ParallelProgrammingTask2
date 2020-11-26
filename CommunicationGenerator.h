#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "Types.h"

class CommunicationGenerator {
    int processCount; // количество процессов
    bool debug; // нужна ли отладка
    std::ofstream &fout;

    void PrintVector(const std::vector<int> &vector) const; // вывод вектора
    void PrintVector(const std::vector<int> &vector, const std::vector<int> &l2g) const; // вывод вектора с глобальными вершинами
    void PrintDebug(const Graph &graph, const Communication &communication, std::vector<std::vector<int>> sendToProcess, std::vector<std::vector<int>> recvFromProcess) const; // вывод отладочной информации
public:
    CommunicationGenerator(std::ofstream &fout, int processCount, bool debug);

    Communication Build(const Graph &graph); // построение схемы обменов
};
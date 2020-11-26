#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include "Types.h"

struct Communication {
    std::vector<int> neighbours;
    std::vector<int> sendOffset;
    std::vector<int> recvOffset;
    std::vector<int> send;
    std::vector<int> recv;
};

class CommunicationGenerator {
    int totalVertices; // общее количество вершин
    int ownVertices; // количество своих вершин
    int *ia;
    int *ja;
    int *part;
    int *l2g;
    int processCount; // количество процессов
    bool debug; // нужна ли отладка
    std::ofstream &fout;

    void PrintVector(const std::vector<int> &vector, bool global = false) const; // вывод вектора
public:
    CommunicationGenerator(std::ofstream &fout, int totalVertices, int ownVertices, int *ia, int *ja, int *part, int *l2g, int processCount, bool debug);

    Communication Build(); // построение схемы обменов
};
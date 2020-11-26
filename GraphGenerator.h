#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include <omp.h>
#include "Types.h"

struct LinkInfo {
    int count; // количество элементов
    int vertices[5]; // связные вершины
};

class GraphGenerator {
    int nx;
    int ny;
    int k1;
    int k2;
    int px;
    int py;
    bool debug;
    std::ofstream &fout;

    bool IsTriangleVertex(int v) const; // проверка, что вершина треугольная
    bool IsUpVertex(int v) const; // проверка, что треугольная вершина сверху
    bool IsDownVertex(int v) const; // проверка, что треугольная вершина снизу

    int Vertex2Index(int v) const; // получение индекса вершины на сетке
    int Index2Vertex(int index) const; // получение вершины по индексу

    int Process2StartRow(int idy) const; // номер процесса в строку начала области
    int Process2StartColumn(int idx) const; // номер процесса в столбце начала области

    int GetOwnVerticesCount(int i_start, int i_end, int j_start, int j_end) const; // количество собственных вершин в области
    int GetHaloVerices(int i_start, int i_end, int j_start, int j_end) const; // количество HALO вершин в области

    void GenerateOwnVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, std::vector<int> &l2g, std::vector<int> &part); // формирование собственных вершин
    void GenerateHaloVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, std::vector<int> &l2g, std::vector<int> &part); // формирование HALO вершин

    std::vector<LinkInfo> MakeEdges(int ownVertices, const std::vector<int> &l2g) const; // формирование рёбер для вершины v
    std::vector<int> MakeIA(const std::vector<LinkInfo> &edges, int ownVertices) const; // формирование массива IA
    std::vector<int> MakeJA(const std::vector<LinkInfo> &edges, const std::vector<int> ia, int ownVertices, std::unordered_map<int, int> &global2local) const; // формирование массива JA

    void PrintEdges(const std::vector<LinkInfo> &edges, int ownVertices) const; // вывод рёбер
    void PrintArray(const std::vector<int> array, const char *message) const; // вывод массива
public:
    GraphGenerator(std::ofstream &fout, int nx, int ny, int k1, int k2, int px, int py, bool debug);

    Graph Generate(int id);
};

#pragma once

#include <iostream>
#include <iomanip>
#include <unordered_map>
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

    bool IsTriangleVertex(int v) const; // проверка, что вершина треугольная
    bool IsUpVertex(int v) const; // проверка, что треугольная вершина сверху
    bool IsDownVertex(int v) const; // проверка, что треугольная вершина снизу

    int Vertex2Index(int v) const; // получение индекса вершины на сетке
    int Index2Vertex(int index) const; // получение вершины по индексу

    int Process2StartRow(int idy) const; // номер процесса в строку начала области
    int Process2StartColumn(int idx) const; // номер процесса в столбце начала области

    int GetVerticesCount() const; // вычисление количества вершин
    int GetOwnVerticesCount(int i_start, int i_end, int j_start, int j_end) const; // количество собственных вершин в области
    int GetHaloVerices(int i_start, int i_end, int j_start, int j_end) const; // количество HALO вершин в области

    void GenerateOwnVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, int *l2g, int *part); // формирование собственных вершин
    void GenerateHaloVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, int *l2g, int *part); // формирование HALO вершин

    LinkInfo* MakeEdges(int ownVertices, int *l2g) const; // формирование рёбер для вершины v
    int* MakeIA(LinkInfo *edges, int ownVertices) const; // формирование массива IA

    void PrintEdges(LinkInfo *edges, int ownVertices) const; // вывод рёбер
    void PrintArray(int *array, int n, const char *message) const; // вывод массива
public:
    GraphGenerator(int nx, int ny, int k1, int k2, int px, int py, bool debug);

    int Generate(int id, int &totalVertices, int &ownVertices, int *&ia, int *&ja, int *&l2g, int *&part);
};

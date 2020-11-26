#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <mpi.h>
#include "Types.h"

double Dot(const std::vector<double> &x, const std::vector<double> &y, int n); // скалярное произведение двух векторов размерности n
void LinearCombination(double a, std::vector<double> &x, double b, const std::vector<double> &y, int n); // линейная комбинация x = ax + by
void MatrixVectorMultiplication(const Graph &graph, const Communication &communication, const std::vector<double> &x, std::vector<double> &result); // произведение матрицы на вектор
void VectorVectorMultiplication(const std::vector<double> &x, const std::vector<double> &y, std::vector<double> &result, int n); // покомпонентное произведение векторов
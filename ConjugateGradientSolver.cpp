#include "ConjugateGradientSolver.h"

ConjugateGradientSolver::ConjugateGradientSolver(double eps, int debug) {
    this->eps = eps;
    this->debug = debug;
}

// получение нормы невязки
double ConjugateGradientSolver::GetResidualNorm(const Graph &graph, const Communication &communication, std::vector<double> &x, std::vector<double> &r) const {
    MatrixVectorMultiplication(graph, communication, x, r); // r = Ax
    LinearCombination(-1, r, 1, graph.b, graph.ownVertices); // r = -Ax + b
    
    return sqrt(Dot(r, r, graph.ownVertices)); // ||b - Ax||
}

// инициализация начальных значений
void ConjugateGradientSolver::InitValues(const Graph &graph, std::vector<double> &x, std::vector<double> &r, std::vector<double> &m) const {
    for (int i = 0; i < graph.ownVertices; i++) {
        x[i] = 0;
        r[i] = graph.b[i]; // b - A*O = b

        int diagIndex = graph.ia[i];

        while (graph.ja[diagIndex] != i)
            diagIndex++;

        m[i] = 1.0 / graph.a[diagIndex];
    }
}

// вывод вектора
void ConjugateGradientSolver::PrintVector(std::ofstream &fout, const std::vector<double> &x) const {
    fout << "x: [ ";

    for (size_t i = 0; i < x.size(); i++) {
        fout << x[i] << " ";
    }

    fout << "]" << std::endl;
    fout << std::endl;
}

// вывод отладки
void ConjugateGradientSolver::PrintDebug(const Solvation &solvation, const std::vector<double> &norms, const std::vector<double> &rho, int id) const {
    std::ofstream fout("log/" + std::to_string(id) + ".txt", std::ios::app);

    for (int i = 0; i < solvation.iterations; i++)
        fout << "Iteration " << i << ", |b - Ax|: " << norms[i] << ", rho: " << rho[i] << std::endl;

    PrintVector(fout, solvation.x);
    fout.close();
}

// решение системы
Solvation ConjugateGradientSolver::Solve(const Graph &graph, const Communication &communication) {
    Solvation solvation;
    solvation.x = std::vector<double>(graph.ownVertices); // начальное решение

    std::vector<double> r(graph.ownVertices); // начальная невязка
    std::vector<double> m(graph.ownVertices); // вектор из обратных элементов диагонали матрицы A

    std::vector<double> z_k(graph.ownVertices);
    std::vector<double> p_k(graph.ownVertices);
    std::vector<double> q_k(graph.ownVertices);
    std::vector<double> debug_r(graph.ownVertices); // вектор для вычисления невязки в debug режиме

    std::vector<double> norms; // нормы на каждой итерации
    std::vector<double> rho;

    InitValues(graph, solvation.x, r, m); // инициализируем значения

    double rho_prev = 0; // предыдущее ро
    solvation.iterations = 1; // текущая итерация
    bool isConverge = false; // сошёлся ли алгоритм

    do {
        VectorVectorMultiplication(m, r, z_k, graph.ownVertices); // z_k = M^-1 * r_k
        double rho_k = Dot(r, z_k, graph.ownVertices); // rho_k = <r_0, z_k>

        if (debug != NO_DEBUG) {
            norms.push_back(GetResidualNorm(graph, communication, solvation.x, debug_r));
            rho.push_back(rho_k);
        }

        if (solvation.iterations == 1) {
            LinearCombination(0, p_k, 1, z_k, graph.ownVertices); // p_k = z_k
        }
        else {
            LinearCombination(rho_k / rho_prev, p_k, 1, z_k, graph.ownVertices); // p_k = z_k + beta * p_k
        }

        MatrixVectorMultiplication(graph, communication, p_k, q_k); // q_k = A * p_k
        double alpha = rho_k / Dot(p_k, q_k, graph.ownVertices); // rho_k / <p_k, q_k>
        LinearCombination(1, solvation.x, alpha, p_k, graph.ownVertices); // x = x + alpha * p_k
        LinearCombination(1, r, -alpha, q_k, graph.ownVertices); // r_k = r_k - alpha * q_k

        if (rho_k < eps) {
            isConverge = true;
        }
        else {
            solvation.iterations++;
        }

        rho_prev = rho_k;
    }
    while (!isConverge);

    solvation.res = GetResidualNorm(graph, communication, solvation.x, debug_r);

    if (debug == FULL_DEBUG) {
        PrintDebug(solvation, norms, rho, graph.id);
    }

    return solvation;
}
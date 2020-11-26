#include "VectorMath.h"

// скалярное произведение двух векторов размерности n
double Dot(const std::vector<double> &x, const std::vector<double> &y, int n) {
    double sum = 0;

    for (int i = 0; i < n; i++) {
        sum += x[i] * y[i];
    }

    double dot = 0;
    MPI_Reduce(&sum, &dot, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Bcast(&dot, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    return dot;
}

// линейная комбинация x = ax + by
void LinearCombination(double a, std::vector<double> &x, double b, const std::vector<double> &y, int n) {
    for (int i = 0; i < n; i++) {
        x[i] = x[i] * a + y[i] * b;
    }
}

// произведение матрицы на вектор
void MatrixVectorMultiplication(const Graph &graph, const Communication &communication, const std::vector<double> &x, std::vector<double> &result) {
    std::map<int, double> recvX;
    std::vector<double> sendX(graph.ownVertices);
    size_t totalSize = communication.recv.size() + communication.send.size();

    std::vector<MPI_Request> request(totalSize);
    std::vector<MPI_Status> status(totalSize);
    
    for (int i = 0; i < communication.neighbours.size(); i++) {
        for (int j = communication.sendOffset[i]; j < communication.sendOffset[i + 1]; j++) {
            sendX[communication.send[j]] = x[communication.send[j]];
            MPI_Isend(&sendX[communication.send[j]], 1, MPI_DOUBLE, communication.neighbours[i], 0, MPI_COMM_WORLD, &request[j]);
        }

        for (int j = communication.recvOffset[i]; j < communication.recvOffset[i + 1]; j++) {
            recvX[communication.recv[j]] = 0;
            MPI_Irecv(&recvX[communication.recv[j]], 1, MPI_DOUBLE, communication.neighbours[i], 0, MPI_COMM_WORLD, &request[communication.send.size() + j]);
        }
    }

    MPI_Waitall(totalSize, request.data(), status.data());

    for (int i = 0; i < graph.ownVertices; i++) {
        result[i] = 0;
        int end = i + 1 < graph.ownVertices ? graph.ia[i + 1] : graph.ia[graph.ownVertices];

        for (int index = graph.ia[i]; index < end; index++)
            result[i] += graph.a[index] * (graph.ja[index] < graph.ownVertices ? x[graph.ja[index]] : recvX[graph.ja[index]]);
    }
}

// покомпонентное произведение векторов
void VectorVectorMultiplication(const std::vector<double> &x, const std::vector<double> &y, std::vector<double> &result, int n) {
    for (int i = 0; i < n; i++) {
        result[i] = x[i] * y[i];
    }
}
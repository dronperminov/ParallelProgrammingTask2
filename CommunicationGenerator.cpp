#include "CommunicationGenerator.h"

CommunicationGenerator::CommunicationGenerator(int processCount, bool debug) {
    this->processCount = processCount;
    this->debug = debug;
}

// вывод вектора
void CommunicationGenerator::PrintVector(std::ofstream &fout, const std::vector<int> &vector) const {
    fout << "[ ";

    for (size_t i = 0; i < vector.size(); i++)
        fout << vector[i] << " ";

    fout << "]" << std::endl;
}

// вывод вектора с глобальными вершинами
void CommunicationGenerator::PrintVector(std::ofstream &fout, const std::vector<int> &vector, const std::vector<int> &l2g) const {
    fout << "[ ";

    for (size_t i = 0; i < vector.size(); i++)
        fout << l2g[vector[i]] << " ";

    fout << "]" << std::endl;
}

// вывод отладочной информации
void CommunicationGenerator::PrintDebug(const Graph &graph, const Communication &communication, std::vector<std::vector<int>> sendToProcess, std::vector<std::vector<int>> recvFromProcess, double time) const {
    std::ofstream fout("log/" + std::to_string(graph.id) + ".txt", std::ios::app);

    fout << "Communication generation time: " << time << "ms" << std::endl;

    for (int p = 0; p < processCount; p++) {
        if (!sendToProcess[p].size())
            continue;

        fout << "SendToProcess P" << p << ":";
        PrintVector(fout, sendToProcess[p], graph.l2g);
        fout << "RecvFromProcess P" << p << ":";
        PrintVector(fout, recvFromProcess[p], graph.l2g);
        fout << std::endl;
    }

    fout << "Neighbours: ";
    PrintVector(fout, communication.neighbours);
    fout << "Send: ";
    PrintVector(fout, communication.send, graph.l2g);
    fout << "Send offset: ";
    PrintVector(fout, communication.sendOffset);
    fout << "Recv: ";
    PrintVector(fout, communication.recv, graph.l2g);
    fout << "Recv offset: ";
    PrintVector(fout, communication.recvOffset);
    fout.close();
}

// построение схемы обменов
Communication CommunicationGenerator::Build(const Graph &graph) {
    TimePoint t0 = Time::now();

    std::vector<std::vector<int>> sendToProcess(processCount);
    std::vector<std::vector<int>> recvFromProcess(processCount);

    for (int i = 0; i < graph.ownVertices; i++) {
        for (int index = graph.ia[i]; index < graph.ia[i + 1]; index++) {
            int j = graph.ja[index];

            if (j >= graph.ownVertices) {
                sendToProcess[graph.part[j]].push_back(i);
                recvFromProcess[graph.part[j]].push_back(j);
            }
        }
    }

    Communication communication;
    communication.sendOffset.push_back(0);
    communication.recvOffset.push_back(0);

    for (int p = 0; p < processCount; p++) {
        if (!sendToProcess[p].size())
            continue;

        communication.neighbours.push_back(p);

        for (size_t i = 0; i < sendToProcess[p].size(); i++) {
            communication.send.push_back(sendToProcess[p][i]);
            communication.recv.push_back(recvFromProcess[p][i]);
        }

        communication.sendOffset.push_back(communication.send.size());
        communication.recvOffset.push_back(communication.recv.size());
    }

    TimePoint t1 = Time::now();
    double time = std::chrono::duration_cast<ms>(t1 - t0).count(); // вычисляем разницу времени

    double communicationTime = 0;
    MPI_Allreduce(&time, &communicationTime, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    if (graph.id == 0) {
        std::cout << "Communication generation time: " << communicationTime << " ms" << std::endl;
    }

    if (debug) {
        PrintDebug(graph, communication, sendToProcess, recvFromProcess, time);
    }

    return communication;
}
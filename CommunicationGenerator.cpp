#include "CommunicationGenerator.h"

CommunicationGenerator::CommunicationGenerator(std::ofstream &fout, int processCount, bool debug) : fout(fout) {
    this->processCount = processCount;
    this->debug = debug;
}

// вывод вектора
void CommunicationGenerator::PrintVector(const std::vector<int> &vector) const {
    fout << "[ ";

    for (size_t i = 0; i < vector.size(); i++)
        fout << vector[i] << " ";

    fout << "]" << std::endl;
}

// вывод вектора с глобальными вершинами
void CommunicationGenerator::PrintVector(const std::vector<int> &vector, const std::vector<int> &l2g) const {
    fout << "[ ";

    for (size_t i = 0; i < vector.size(); i++)
        fout << l2g[vector[i]] << " ";

    fout << "]" << std::endl;
}

// вывод отладочной информации
void CommunicationGenerator::PrintDebug(const Graph &graph, const Communication &communication, std::vector<std::vector<int>> sendToProcess, std::vector<std::vector<int>> recvFromProcess) const {
    for (int p = 0; p < processCount; p++) {
        if (!sendToProcess[p].size())
            continue;

        fout << "SendToProcess P" << p << ":";
        PrintVector(sendToProcess[p], graph.l2g);
        fout << "RecvFromProcess P" << p << ":";
        PrintVector(recvFromProcess[p], graph.l2g);
        fout << std::endl;
    }

    fout << "Neighbours: ";
    PrintVector(communication.neighbours);
    fout << "Send: ";
    PrintVector(communication.send, graph.l2g);
    fout << "Send offset: ";
    PrintVector(communication.sendOffset);
    fout << "Recv: ";
    PrintVector(communication.recv, graph.l2g);
    fout << "Recv offset: ";
    PrintVector(communication.recvOffset);
}

// построение схемы обменов
Communication CommunicationGenerator::Build(const Graph &graph) {
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

    if (debug) {
        PrintDebug(graph, communication, sendToProcess, recvFromProcess);
    }

    return communication;
}
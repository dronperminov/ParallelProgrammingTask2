#include "CommunicationGenerator.h"

CommunicationGenerator::CommunicationGenerator(std::ofstream &fout, int totalVertices, int ownVertices, int *ia, int *ja, int *part, int *l2g, int processCount, bool debug) : fout(fout) {
    this->totalVertices = totalVertices;
    this->ownVertices = ownVertices;

    this->ia = ia;
    this->ja = ja;
    this->part = part;
    this->l2g = l2g;
    this->processCount = processCount;

    this->debug = debug;
}

// вывод вектора
void CommunicationGenerator::PrintVector(const std::vector<int> &vector, bool global) const {
    fout << "[ ";

    for (size_t i = 0; i < vector.size(); i++)
        fout << (global ? l2g[vector[i]] : vector[i]) << " ";

    fout << "]" << std::endl;
}

// построение схемы обменов
Communication CommunicationGenerator::Build() {
    std::vector<std::vector<int>> sendToProcess(processCount);
    std::vector<std::vector<int>> recvFromProcess(processCount);

    for (int i = 0; i < ownVertices; i++) {
        for (int index = ia[i]; index < ia[i + 1]; index++) {
            int j = ja[index];

            if (j >= ownVertices) {
                sendToProcess[part[j]].push_back(i);
                recvFromProcess[part[j]].push_back(j);
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
        for (int p = 0; p < processCount; p++) {
            if (!sendToProcess[p].size())
                continue;

            fout << "SendToProcess P" << p << ":";
            PrintVector(sendToProcess[p], true);
            fout << "RecvFromProcess P" << p << ":";
            PrintVector(recvFromProcess[p], true);
            fout << std::endl;
        }

        fout << "Neighbours: ";
        PrintVector(communication.neighbours);
        fout << "Send: ";
        PrintVector(communication.send, true);
        fout << "Send offset: ";
        PrintVector(communication.sendOffset);
        fout << "Recv: ";
        PrintVector(communication.recv, true);
        fout << "Recv offset: ";
        PrintVector(communication.recvOffset);
    }

    return communication;
}
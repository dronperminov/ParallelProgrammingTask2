#include <iostream>
#include <fstream>
#include <cstring>
#include <mpi.h>

#include "ArgumentParser.h"
#include "GraphGenerator.h"
#include "GraphFiller.h"

using namespace std;

// инициализация MPI
bool InitMPI(int &processCount, int &pid) {
    int status = MPI_Init(NULL, NULL); // первым делом подключаемся к MPI - инициализация

    if (status != MPI_SUCCESS) {
        cout << "MPI_Init failed (code " << status << ")" << endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
        return false;
    }

    status = MPI_Comm_size(MPI_COMM_WORLD, &processCount); // узнаем количество процессов

    if (status != MPI_SUCCESS) {
        cout << "MPI_Comm_size failed (code " << status << ")" << endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
        return false;
    }

    status = MPI_Comm_rank(MPI_COMM_WORLD, &pid); // узнаем номер данного процесса

    if (status != MPI_SUCCESS) {
        cout << "MPI_Comm_rank failed (code " << status << ")" << endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
        return false;
    }

    return true;
}

// вывод информационного сообщения
void Help() {
    cout << "Usage: ./main path [debug] or ./main Nx Ny k1 k2 Px Py eps [debug]" << endl;
    cout << endl;
    cout << "Arguments description:" << endl;
    cout << "path  - path to file with arguments" << endl;
    cout << "Nx    - number of columns in grid (natural)" << endl;
    cout << "Ny    - number of rows in grid (natural)" << endl;
    cout << "k1    - number of non divisible cells (integer >= 0)" << endl;
    cout << "k2    - number of divisible cells (integer >= 0)" << endl;
    cout << "Px    - number of parts in horizontal grid (natural)" << endl;
    cout << "Py    - number of parts in vertical grid (natural)" << endl;
    cout << "eps   - relative solution accuracy (real)" << endl;
    cout << "debug - print [f]ull, [s]olve or [n]o debug info (f/s/n)" << endl;
}

// проверка аргументов на вывод help сообщения
bool CheckHelp(int argc, char **argv, int pid) {
    if (argc == 1 || (argc == 2 && !strcmp(argv[1], "--help"))) {
        if (pid == 0) {
            Help();
        }

        MPI_Abort(MPI_COMM_WORLD, -1);
        return true;
    }

    return false;
}

void Solve(const ArgumentParser& parser, int pid) {
    // входные аргументы
    int nx = parser.GetNx();
    int ny = parser.GetNy();
    int k1 = parser.GetK1();
    int k2 = parser.GetK2();
    int px = parser.GetPx();
    int py = parser.GetPy();
    double eps = parser.GetEps();
    int debug = parser.GetDebug();

    // выходные аргументы
    int totalVertices = 0;
    int ownVertices = 0;
    int *ia = NULL;
    int *ja = NULL;
    int *l2g = NULL;
    int *part = NULL;
    double *a = NULL;
    double *b = NULL;

    GraphGenerator generator(nx, ny, k1, k2, px, py, debug == FULL_DEBUG);
    generator.Generate(pid, totalVertices, ownVertices, ia, ja, l2g, part); // запускаем генерацию

    GraphFiller filler(ownVertices, ia, ja, l2g, debug == FULL_DEBUG);
    filler.Fill(a, b);

    // освобождаем выделенную память
    delete[] ia;
    delete[] ja;
    delete[] l2g;
    delete[] part;
    delete[] a;
    delete[] b;
}

int main(int argc, char **argv) {
    int processCount; // количество процессов
    int pid; // номер процесса

    if (!InitMPI(processCount, pid))
        return -1;

    // если аргументов нет или запустили вызов сообщения
    if (CheckHelp(argc, argv, pid))
        return 0;

    ArgumentParser parser;

    if (!parser.ParseArgs(argc, argv)) {
        MPI_Abort(MPI_COMM_WORLD, -1);
        return -1;
    }

    if (parser.GetDebug() == FULL_DEBUG && pid == 0)
        parser.PrintArguments();

    Solve(parser, pid);
}
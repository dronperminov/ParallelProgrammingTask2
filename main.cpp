#include <iostream>
#include <fstream>
#include <cstring>
#include <omp.h>

#include "ArgumentParser.h"
#include "GraphGenerator.h"

using namespace std;

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

void Solve(const ArgumentParser& parser) {
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
    int n = 0;
    int *ia = NULL;
    int *ja = NULL;
    int *g2l = NULL;
    int *l2g = NULL;
    int *part = NULL;

    GraphGenerator generator(nx, ny, k1, k2, px, py, debug == FULL_DEBUG);
    generator.Generate(n, ia, ja, g2l, l2g, part); // запускаем генерацию

    // освобождаем выделенную память
    delete[] ia;
    delete[] ja;
    delete[] g2l;
    delete[] l2g;
    delete[] part;
}

int main(int argc, char **argv) {
    // если аргументов нет или запустили вызов сообщения
    if (argc == 1 || (argc == 2 && !strcmp(argv[1], "--help"))) {
        Help();
        return 0;
    }

    ArgumentParser parser;

    if (!parser.ParseArgs(argc, argv))
        return -1;

    if (parser.GetDebug() == FULL_DEBUG)
        parser.PrintArguments();

    Solve(parser);
}
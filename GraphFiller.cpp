#include "GraphFiller.h"

GraphFiller::GraphFiller(std::ofstream &fout, int totalVertices, int ownVertices, int *ia, int *ja, int *l2g, bool debug) : fout(fout) {
    this->totalVertices = totalVertices;
    this->ownVertices = ownVertices;
    this->ia = ia;
    this->ja = ja;
    this->l2g = l2g;

    this->debug = debug;
}

// вывод отладочных значений
void GraphFiller::PrintDebug(double *a, double *b) const {
    fout << "Filled values: A b" << std::endl;

    for (int i = 0; i < ownVertices; i++) {
        fout << "  " << i << ": ";

        for (int j = ia[i]; j < ia[i + 1]; j++)
            fout << std::left << std::setw(12) << a[j] << " ";

        fout << " = " << b[i] << "" << std::endl;
    }

    fout << std::endl;
}

// заполнение
void GraphFiller::Fill(double *&a, double *&b) const {
    a = new double[ia[ownVertices]];
    b = new double[totalVertices];

    for (int i = 0; i < ownVertices; i++) {
        double sum = 0;
        int diagIndex = -1;

        for (int index = ia[i]; index < ia[i + 1]; index++) {
            int j = ja[index];

            if (i != j) {
                a[index] = Fa(l2g[i], l2g[j]); // a_ij
                sum += fabs(a[index]); // наращиваем сумму внедиагональных элементов
            }
            else {
                diagIndex = index;
            }
        }

        a[diagIndex] = DIAGONAL_DOMINANCE_COEFFICIENT * sum;
    }

    for (int i = 0; i < totalVertices; i++)
        b[i] = Fb(l2g[i]);

    if (debug) {
        PrintDebug(a, b);
    }
}
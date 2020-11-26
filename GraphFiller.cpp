#include "GraphFiller.h"

GraphFiller::GraphFiller(int ownVertices, int *ia, int *ja, int *l2g, bool debug) {
    this->ownVertices = ownVertices;
    this->ia = ia;
    this->ja = ja;
    this->l2g = l2g;

    this->debug = debug;
}

// вывод отладочных значений
void GraphFiller::PrintDebug(double *a, double *b) const {
    std::cout << "    Filled values: A b" << std::endl;

    for (int i = 0; i < ownVertices; i++) {
        std::cout << "      " << i << ": ";

        for (int j = ia[i]; j < ia[i + 1]; j++)
            std::cout << std::left << std::setw(12) << a[j] << " ";

        std::cout << " = " << b[i] << "" << std::endl;
    }

    std::cout << std::endl;
}

// заполнение
void GraphFiller::Fill(double *&a, double *&b) const {
    a = new double[ia[ownVertices]];
    b = new double[ownVertices];

    for (int i = 0; i < ownVertices; i++) {
        double sum = 0;
        int diagIndex = -1;

        for (int index = ia[i]; index < ia[i + 1]; index++) {
            int j = ja[index];

            if (i != j) {
                a[index] = Fa(l2g[i], l2g[j]); // a_ij
                sum += fabs(a[index]); // наразиваем сумму внедиагональных элементов
            }
            else {
                diagIndex = index;
            }
        }

        a[diagIndex] = DIAGONAL_DOMINANCE_COEFFICIENT * sum;
        b[i] = Fb(l2g[i]);
    }

    if (debug) {
        PrintDebug(a, b);
    }
}
#include "GraphFiller.h"

GraphFiller::GraphFiller(bool debug) {
    this->debug = debug;
}

// вывод отладочных значений
void GraphFiller::PrintDebug(const Graph& graph) const {
    std::ofstream fout("log/" + std::to_string(graph.id) + ".txt", std::ios::app);
    fout << "Filled values: A b" << std::endl;

    for (int i = 0; i < graph.ownVertices; i++) {
        fout << "  " << i << ": ";

        for (int j = graph.ia[i]; j < graph.ia[i + 1]; j++)
            fout << std::left << std::setw(12) << graph.a[j] << " ";

        fout << " = " << graph.b[i] << std::endl;
    }

    fout << std::endl;
    fout.close();
}

// заполнение
void GraphFiller::Fill(Graph &graph) const {
    graph.a = std::vector<double>(graph.ia[graph.ownVertices]);
    graph.b = std::vector<double>(graph.ownVertices);

    for (int i = 0; i < graph.ownVertices; i++) {
        double sum = 0;
        int diagIndex = -1;

        for (int index = graph.ia[i]; index < graph.ia[i + 1]; index++) {
            int j = graph.ja[index];

            if (i != j) {
                graph.a[index] = Fa(graph.l2g[i], graph.l2g[j]); // a_ij
                sum += fabs(graph.a[index]); // наращиваем сумму внедиагональных элементов
            }
            else {
                diagIndex = index;
            }
        }

        graph.a[diagIndex] = DIAGONAL_DOMINANCE_COEFFICIENT * sum;
        graph.b[i] = Fb(graph.l2g[i]);
    }

    if (debug) {
        PrintDebug(graph);
    }
}
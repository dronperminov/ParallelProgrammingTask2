#include "GraphGenerator.h"

// проверка, что вершина треугольная
bool GraphGenerator::IsTriangleVertex(int v) const {
    return v % (k1 + 2 * k2) >= k1;
}

// проверка, что треугольная вершина сверху
bool GraphGenerator::IsUpVertex(int v) const {
    int pos = v % (k1 + 2 * k2);
    return pos >= k1 && (pos - k1) & 1;
}

// проверка, что треугольная вершина снизу
bool GraphGenerator::IsDownVertex(int v) const {
    int pos = v % (k1 + 2 * k2);
    return pos >= k1 && !((pos - k1) & 1);
}

// получение индекса вершины на сетке
int GraphGenerator::Vertex2Index(int v) const {
    int div = v / (k1 + 2 * k2);
    int mod = v % (k1 + 2 * k2);
    int pos = div * (k1 + k2);

    if (mod < k1) {
        pos += mod;
    }
    else {
        pos += (mod + k1) / 2;
    }

    return pos;
}

// получение вершины по индексу
int GraphGenerator::Index2Vertex(int index) const {
    int div = index / (k1 + k2);
    int mod = index % (k1 + k2);
    int vertex = div * (k1 + 2 * k2);

    if (mod < k1) {
        vertex += mod;
    }
    else {
        vertex += mod * 2 - k1;
    }

    return vertex;
}

// номер процесса в строку начала области
int GraphGenerator::Process2StartRow(int idy) const {
    return idy * (ny / py) + std::min(idy, ny % py);
}

// номер процесса в столбце начала области
int GraphGenerator::Process2StartColumn(int idx) const {
    return idx * (nx / px) + std::min(idx, nx % px);
}

// формирование ребёр для вершины v
LinkInfo GraphGenerator::MakeEdgesForVertex(int v) const {
    LinkInfo edges;
    int index = Vertex2Index(v);
    int x = index % nx;
    int y = index / nx;

    edges.count = 0;

    // соседняя сверху, если не нижнетреугольная ячейка
    if (y > 0 && !IsDownVertex(v))
        edges.vertices[edges.count++] = Index2Vertex((y - 1) * nx + x);

    // соседняя слева
    if (x > 0 || (x == 0 && IsUpVertex(v)))
        edges.vertices[edges.count++] = v - 1;

    edges.vertices[edges.count++] = v;

    // соседняя справа
    if (x < nx - 1 || (x == nx - 1 && IsDownVertex(v)))
        edges.vertices[edges.count++] = v + 1;

    // соседняя снизу, если не верхнетреугольная ячейка
    if (y < ny - 1 && !IsUpVertex(v)) {
        int vertex = Index2Vertex((y + 1) * nx + x);

        if (IsTriangleVertex(vertex))
            vertex++;

        edges.vertices[edges.count++] = vertex;
    }

    return edges;
}

// формирование рёбер
std::vector<LinkInfo> GraphGenerator::MakeEdges(int ownVertices, const std::vector<int> &l2g) const {
    std::vector<LinkInfo> edges(ownVertices);

    for (int i = 0; i < ownVertices; i++)
        edges[i] = MakeEdgesForVertex(l2g[i]);

    return edges;
}

// формирование массива IA
std::vector<int> GraphGenerator::MakeIA(const std::vector<LinkInfo> &edges, int ownVertices) const {
    std::vector<int> ia(ownVertices + 1);
    ia[0] = 0;

    for (int i = 0; i < ownVertices; i++)
        ia[i + 1] = ia[i] + edges[i].count;

    return ia;
}

// формирование массива JA
std::vector<int> GraphGenerator::MakeJA(const std::vector<LinkInfo> &edges, const std::vector<int> ia, int ownVertices, std::unordered_map<int, int> &global2local) const {
    std::vector<int> ja = std::vector<int>(ia[ownVertices]);

    for (int i = 0; i < ownVertices; i++)
        for (int j = 0; j < edges[i].count; j++)
            ja[ia[i] + j] = global2local[edges[i].vertices[j]];

    return ja;
}

// вывод рёбер
void GraphGenerator::PrintEdges(const std::vector<LinkInfo> &edges, int ownVertices) const {
    fout << "Edges list: " << std::endl;

    for (int i = 0; i < ownVertices; i++) {
        fout << "      " << i << " -> [ ";

        for (int j = 0; j < edges[i].count; j++)
            fout << edges[i].vertices[j] << " ";

        fout << "]" << std::endl;
    }
}

// вывод массива
void GraphGenerator::PrintArray(const std::vector<int> array, const char *message) const {
    fout << message << ": [ ";

    for (size_t i = 0; i < array.size(); i++)
        fout << array[i] << " ";

    fout << "]" << std::endl;
}

GraphGenerator::GraphGenerator(std::ofstream &fout, int nx, int ny, int k1, int k2, int px, int py, bool debug) : fout(fout) {
    this->nx = nx;
    this->ny = ny;

    this->k1 = k1;
    this->k2 = k2;

    this->px = px;
    this->py = py;

    this->debug = debug;
}

// количество вершин в области
int GraphGenerator::GetOwnVerticesCount(int i_start, int i_end, int j_start, int j_end) const {
    int vertices = 0;

    for (int i = i_start; i < i_end; i++) {
        for (int j = j_start; j < j_end; j++) {
            int vertex = Index2Vertex(i * nx + j);

            if (IsTriangleVertex(vertex)) {
                vertices += 2;
            }
            else {
                vertices++;
            }
        }
    }

    return vertices;
}

// количество HALO вершин в области
int GraphGenerator::GetHaloVerices(int i_start, int i_end, int j_start, int j_end) const {
    int vertices = 0;

    if (i_start > 0)
        vertices += j_end - j_start;

    if (j_start > 0)
        vertices += i_end - i_start;

    if (i_end < ny)
        vertices += j_end - j_start;

    if (j_end < nx)
        vertices += i_end - i_start;

    return vertices;
}

// формирование собственных вершин
void GraphGenerator::GenerateOwnVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, std::vector<int> &l2g, std::vector<int> &part) {
    for (int i = i_start; i < i_end; i++) {
        for (int j = j_start; j < j_end; j++) {
            int vertex = Index2Vertex(i * nx + j);

            l2g[local] = vertex;
            part[local++] = id;

            if (IsTriangleVertex(vertex)) {
                l2g[local] = vertex + 1;
                part[local++] = id;                 
            }
        }
    }
}

// формирование HALO вершин
void GraphGenerator::GenerateHaloVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, std::vector<int> &l2g, std::vector<int> &part) {
    if (i_start > 0) {
        for (int j = j_start; j < j_end; j++) {
            l2g[local] = Index2Vertex((i_start - 1) * nx + j);
            part[local++] = id - px;
        }
    }

    for (int i = i_start; i < i_end; i++) {
        if (j_start > 0) {
            int vertex = Index2Vertex(i * nx + j_start - 1);

            if (IsTriangleVertex(vertex))
                vertex++;

            l2g[local] = vertex;
            part[local++] = id - 1;
        }

        if (j_end < nx) {
            l2g[local] = Index2Vertex(i * nx + j_end);
            part[local++] = id + 1;
        }
    }

    if (i_end < ny) {
        for (int j = j_start; j < j_end; j++) {
            int vertex = Index2Vertex(i_end * nx + j);

            if (IsTriangleVertex(vertex))
                vertex++;

            l2g[local] = vertex;
            part[local++] = id + px;
        }
    }
}

Graph GraphGenerator::Generate(int id) {
    int idx = id % px;
    int idy = id / px;

    // границы области процесса по вертикали
    int i_start = Process2StartRow(idy);
    int i_end = Process2StartRow(idy + 1);

    // границы области процесса по горизонтали
    int j_start = Process2StartColumn(idx);
    int j_end = Process2StartColumn(idx + 1);

    Graph graph; // создаём граф

    graph.ownVertices = GetOwnVerticesCount(i_start, i_end, j_start, j_end); // количество собственных вершин в области
    graph.haloVertices = GetHaloVerices(i_start, i_end, j_start, j_end); // количество HALO вершин в области
    graph.totalVertices = graph.ownVertices + graph.haloVertices; // считаем общее количество вершин

    if (debug) {
        fout << "P" << id << ": " << std::endl;
        fout << "rows: [" << i_start << ", " << i_end << ")" << std::endl;
        fout << "columns: [" << j_start << ", " << j_end << "), " << std::endl;
        fout << "OWN vertices (No): " << graph.ownVertices << std::endl;
        fout << "HALO vertices: " << graph.haloVertices << std::endl;
        fout << "TOTAL vertices (N): " << graph.totalVertices << std::endl;
    }

    graph.l2g = std::vector<int>(graph.totalVertices);
    graph.part = std::vector<int>(graph.totalVertices);

    int local = 0;
    GenerateOwnVertices(id, i_start, i_end, j_start, j_end, local, graph.l2g, graph.part);
    GenerateHaloVertices(id, i_start, i_end, j_start, j_end, local, graph.l2g, graph.part);

    std::unordered_map<int, int> global2local;

    for (int i = 0; i < graph.totalVertices; i++)
        global2local[graph.l2g[i]] = i;

    std::vector<LinkInfo> edges = MakeEdges(graph.ownVertices, graph.l2g);
    graph.ia = MakeIA(edges, graph.ownVertices);
    graph.ja = MakeJA(edges, graph.ia, graph.ownVertices, global2local);

    if (debug) {
        std::vector<int> jag(graph.ia[graph.ownVertices]);

        for (int i = 0; i < graph.ia[graph.ownVertices]; i++)
            jag[i] = graph.l2g[graph.ja[i]];

        PrintArray(graph.l2g, "L2G");
        PrintArray(graph.part, "Part");
        PrintArray(graph.ia, "IA");
        PrintArray(graph.ja, "JA");
        PrintArray(jag, "JA (GLOBAL)");
        PrintEdges(edges, graph.ownVertices);
        fout << std::endl;
    }

    return graph; // возвращаем граф
}
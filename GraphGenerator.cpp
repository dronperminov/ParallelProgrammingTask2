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

// вычисление количества вершин
int GraphGenerator::GetVerticesCount() const {
    int totalCells = nx * ny; // общее количество ячеек без деления
    int intPart = totalCells / (k1 + k2); // полноценно разбиваемая часть ячеек
    int modPart = totalCells % (k1 + k2); // сколько ячеек останется после целого разбиения

    int n = intPart * (k1 + 2 * k2); // как минимум вот столько вершин
    int rectangles = intPart * k1; // количество прямоугольников
    int triangles = intPart * 2 * k2; // количество треугольников

    // если ещё остались клетки
    if (modPart > 0) {
        int n1 = modPart > k1 ? k1 : modPart; // количество клеток без разбиения
        int n2 = modPart - n1; // количество клеток с разбиением

        n += n1 + n2 * 2; // добавляем оставшиеся клетки
        rectangles += n1;
        triangles += n2 * 2;
    }

    if (debug) {
        fout << std::endl;
        fout << "Rectangles count: " << rectangles << std::endl;
        fout << "Triangles count: " << triangles << std::endl;
        fout << "Vertices count: " << n << std::endl;
    }

    return n;
}

// формирование рёбер
LinkInfo* GraphGenerator::MakeEdges(int ownVertices, int *l2g) const {
    LinkInfo *edges = new LinkInfo[ownVertices];

    for (int i = 0; i < ownVertices; i++) {
        int v = l2g[i];
        int index = Vertex2Index(v);
        int x = index % nx;
        int y = index / nx;

        edges[i].count = 0;

        // соседняя сверху, если не нижнетреугольная ячейка
        if (y > 0 && !IsDownVertex(v))
            edges[i].vertices[edges[i].count++] = Index2Vertex((y - 1) * nx + x);

        // соседняя слева
        if (x > 0 || (x == 0 && IsUpVertex(v)))
            edges[i].vertices[edges[i].count++] = v - 1;

        edges[i].vertices[edges[i].count++] = v;

        // соседняя справа
        if (x < nx - 1 || (x == nx - 1 && IsDownVertex(v)))
            edges[i].vertices[edges[i].count++] = v + 1;

        // соседняя снизу, если не верхнетреугольная ячейка
        if (y < ny - 1 && !IsUpVertex(v)) {
            int vertex = Index2Vertex((y + 1) * nx + x);

            if (IsTriangleVertex(vertex))
                vertex++;

            edges[i].vertices[edges[i].count++] = vertex;
        }
    }

    return edges;
}

// формирование массива IA
int* GraphGenerator::MakeIA(LinkInfo *edges, int ownVertices) const {
    int *ia = new int[ownVertices + 1];
    ia[0] = 0;

    for (int i = 0; i < ownVertices; i++)
        ia[i + 1] = ia[i] + edges[i].count;

    return ia;
}

// вывод рёбер
void GraphGenerator::PrintEdges(LinkInfo *edges, int ownVertices) const {
    fout << "Edges list: " << std::endl;

    for (int i = 0; i < ownVertices; i++) {
        fout << "      " << i << " -> [ ";

        for (int j = 0; j < edges[i].count; j++)
            fout << edges[i].vertices[j] << " ";

        fout << "]" << std::endl;
    }
}

// вывод массива
void GraphGenerator::PrintArray(int *array, int n, const char *message) const {
    fout << message << ": [ ";

    for (int i = 0; i < n; i++)
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
void GraphGenerator::GenerateOwnVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, int *l2g, int *part) {
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
void GraphGenerator::GenerateHaloVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, int *l2g, int *part) {
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

int GraphGenerator::Generate(int id, int &totalVertices, int &ownVertices, int *&ia, int *&ja, int *&l2g, int *&part) {
    int idx = id % px;
    int idy = id / px;

    // границы области процесса по вертикали
    int i_start = Process2StartRow(idy);
    int i_end = Process2StartRow(idy + 1);

    // границы области процесса по горизонтали
    int j_start = Process2StartColumn(idx);
    int j_end = Process2StartColumn(idx + 1);

    ownVertices = GetOwnVerticesCount(i_start, i_end, j_start, j_end); // количество собственных вершин в области
    int haloVertices = GetHaloVerices(i_start, i_end, j_start, j_end); // количество HALO вершин в области
    totalVertices = ownVertices + haloVertices;

    if (debug) {
        fout << "P" << id << ": " << std::endl;
        fout << "rows: [" << i_start << ", " << i_end << ")" << std::endl;
        fout << "columns: [" << j_start << ", " << j_end << "), " << std::endl;
        fout << "OWN vertices (No): " << ownVertices << std::endl;
        fout << "HALO vertices: " << haloVertices << std::endl;
        fout << "TOTAL vertices (N): " << totalVertices << std::endl;
    }

    l2g = new int[totalVertices];
    part = new int[totalVertices];

    int local = 0;
    GenerateOwnVertices(id, i_start, i_end, j_start, j_end, local, l2g, part);
    GenerateHaloVertices(id, i_start, i_end, j_start, j_end, local, l2g, part);

    std::unordered_map<int, int> global2local;

    for (int i = 0; i < totalVertices; i++)
        global2local[l2g[i]] = i;

    LinkInfo *edges = MakeEdges(ownVertices, l2g);
    ia = MakeIA(edges, ownVertices);
    ja = new int[ia[ownVertices]];

    for (int i = 0; i < ownVertices; i++)
        for (int j = 0; j < edges[i].count; j++)
            ja[ia[i] + j] = global2local[edges[i].vertices[j]];

    if (debug) {
        int *jag = new int[ia[ownVertices]];

        for (int i = 0; i < ia[ownVertices]; i++)
            jag[i] = l2g[ja[i]];

        PrintArray(l2g, totalVertices, "L2G");
        PrintArray(part, totalVertices, "Part");
        PrintArray(ia, ownVertices + 1, "IA");
        PrintArray(ja, ia[ownVertices], "JA");
        PrintArray(jag, ia[ownVertices], "JA (GLOBAL)");
        PrintEdges(edges, ownVertices);
        fout << std::endl;
    }

    return 0; // возвращаем время
}
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
        std::cout << std::endl;
        std::cout << "Rectangles count: " << rectangles << std::endl;
        std::cout << "Triangles count: " << triangles << std::endl;
        std::cout << "Vertices count: " << n << std::endl;
    }

    return n;
}

// формирование рёбер
LinkInfo* GraphGenerator::MakeEdges(int n) const {
    LinkInfo *edges = new LinkInfo[n]; // создаём список смежности

    for (int v = 0; v < n; v++) {
        int index = Vertex2Index(v);
        int x = index % nx;
        int y = index / nx;

        edges[v].count = 0;

        // соседняя сверху, если не нижнетреугольная ячейка
        if (y > 0 && !IsDownVertex(v))
            edges[v].vertices[edges[v].count++] = Index2Vertex((y - 1) * nx + x);

        // соседняя слева
        if (x > 0 || (x == 0 && IsUpVertex(v)))
            edges[v].vertices[edges[v].count++] = v - 1;

        edges[v].vertices[edges[v].count++] = v;

        // соседняя справа
        if (x < nx - 1 || (x == nx - 1 && IsDownVertex(v)))
            edges[v].vertices[edges[v].count++] = v + 1;

        // соседняя снизу, если не верхнетреугольная ячейка
        if (y < ny - 1 && !IsUpVertex(v)) {
            int vertex = Index2Vertex((y + 1) * nx + x);

            if (IsTriangleVertex(vertex))
                vertex++;

            edges[v].vertices[edges[v].count++] = vertex;
        }
    }

    return edges;
}

// вывод рёбер
void GraphGenerator::PrintEdges(LinkInfo *edges, int n) const {
    std::cout << std::endl;
    std::cout << "Edges list: " << std::endl;

    for (int i = 0; i < n; i++) {
        std::cout << i << " -> [ ";

        for (int j = 0; j < edges[i].count; j++)
            std::cout << edges[i].vertices[j] << " ";

        std::cout << "]" << std::endl;
    }
}

// вывод массива
void GraphGenerator::PrintArray(int *array, int n, const char *message) const {
    std::cout << message << ": [ ";

    for (int i = 0; i < n; i++)
        std::cout << array[i] << " ";

    std::cout << "]" << std::endl;
}

// вывод сводной информации
void GraphGenerator::PrintInfo(int n, int *ia, int *ja, const ms &time) const {
    std::cout << "+--------------------------------------+" << std::endl;
    std::cout << "|             Generate part            |" << std::endl;
    std::cout << "+--------------------+-----------------+" << std::endl;
    std::cout << "|           Vertices | " << std::setw(15) << n << " |" << std::endl;
    std::cout << "|              Edges | " << std::setw(15) << ia[n] << " |" << std::endl;
    std::cout << "| Portrait non zeros | " << std::setw(15) << GetNotZeroCount(ja, ia[n]) << " |" << std::endl;
    std::cout << "|   Elapsed time, ms | " << std::setw(15) << time.count() << " |" << std::endl;
    std::cout << "+--------------------+-----------------+" << std::endl;
    std::cout << std::endl;
}

// получение количества ненулевых элементов
int GraphGenerator::GetNotZeroCount(int *array, int n) const {
    int count = 0;

    for (int i = 0; i < n; i++)
        if (array[i])
            count++;

    return count;
}

GraphGenerator::GraphGenerator(int nx, int ny, int k1, int k2, int px, int py, bool debug) {
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
void GraphGenerator::GenerateOwnVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, int *g2l, int *l2g, int *part) {
    for (int i = i_start; i < i_end; i++) {
        for (int j = j_start; j < j_end; j++) {
            int vertex = Index2Vertex(i * nx + j);

            g2l[local] = local;
            l2g[local] = vertex;
            part[local++] = id;

            if (IsTriangleVertex(vertex)) {
                g2l[local] = local;
                l2g[local] = vertex + 1;
                part[local++] = id;                 
            }
        }
    }
}

// формирование HALO вершин
void GraphGenerator::GenerateHaloVertices(int id, int i_start, int i_end, int j_start, int j_end, int &local, int *g2l, int *l2g, int *part) {
    if (i_start > 0) {
        for (int j = j_start; j < j_end; j++) {
            g2l[local] = local;
            l2g[local] = Index2Vertex((i_start - 1) * nx + j);
            part[local++] = id - px;
        }
    }

    for (int i = i_start; i < i_end; i++) {
        if (j_start > 0) {
            int vertex = Index2Vertex(i * nx + j_start - 1);

            if (IsTriangleVertex(vertex))
                vertex++;

            g2l[local] = local;
            l2g[local] = vertex;
            part[local++] = id - 1;
        }

        if (j_end < nx) {
            g2l[local] = local;
            l2g[local] = Index2Vertex(i * nx + j_end);
            part[local++] = id + 1;
        }
    }

    if (i_end < ny) {
        for (int j = j_start; j < j_end; j++) {
            int vertex = Index2Vertex(i_end * nx + j);

            if (IsTriangleVertex(vertex))
                vertex++;

            g2l[local] = local;
            l2g[local] = vertex;
            part[local++] = id + px;
        }
    }
}

int GraphGenerator::Generate(int &n, int *&ia, int *&ja, int *&g2l, int *&l2g, int *&part, bool showInfo) {
    n = GetVerticesCount(); // получаем количество вершин

    for (int id = 0; id < px * py; id++) {
        int idx = id % px;
        int idy = id / px;

        // границы области процесса по вертикали
        int i_start = Process2StartRow(idy);
        int i_end = Process2StartRow(idy + 1);

        // границы области процесса по горизонтали
        int j_start = Process2StartColumn(idx);
        int j_end = Process2StartColumn(idx + 1);

        int ownVertices = GetOwnVerticesCount(i_start, i_end, j_start, j_end); // количество собственных вершин в области
        int haloVertices = GetHaloVerices(i_start, i_end, j_start, j_end); // количество HALO вершин в области

        if (debug) {
            std::cout << "P" << id << ": " << std::endl;
            std::cout << "    rows: [" << i_start << ", " << i_end << ")" << std::endl;
            std::cout << "    columns: [" << j_start << ", " << j_end << "), " << std::endl;
            std::cout << "    OWN vertices: " << ownVertices << std::endl;
            std::cout << "    HALO vertices: " << haloVertices << std::endl;
        }

        int total = ownVertices + haloVertices;

        g2l = new int[total];
        l2g = new int[total];
        part = new int[total];

        int local = 0;
        GenerateOwnVertices(id, i_start, i_end, j_start, j_end, local, g2l, l2g, part);
        GenerateHaloVertices(id, i_start, i_end, j_start, j_end, local, g2l, l2g, part);        

        if (debug) {
            PrintArray(g2l, total, "    G2L");
            PrintArray(l2g, total, "    L2G");
            PrintArray(part, total, "    Part");
            std::cout << std::endl;
        }
    }

    return 0; // возвращаем время
}
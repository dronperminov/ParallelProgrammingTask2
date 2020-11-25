const RECTANGLE_COLOR = '#2196f3'//'#009688'
const UP_TRIANGLE_COLOR = '#2196f3'//'#ffbcd4'
const DOWN_TRIANGLE_COLOR = '#2196f3'//'#2196f3'

const EDGE_COLOR = '#ff5722'
const EDGE_LINE_WIDTH = 3

const MIN_CELL_SIZE = 80
const MIN_RADIUS = 12
const MAX_RADIUS = 20

const VARIANT_A1 = 'A1'
const VARIANT_A2 = 'A2'
const VARIANT_B1 = 'B1'
const VARIANT_B2 = 'B2'

function Grid(canvas, nx, ny, k1, k2, px, py, eps, variant, result) {
    this.canvas = canvas
    this.ctx = canvas.getContext('2d')

    this.nx = nx
    this.ny = ny
    this.k1 = k1
    this.k2 = k2
    this.px = px
    this.py = py
    this.eps = eps
    this.variant = variant
    this.result = result

    this.padding = variant == VARIANT_A1 || variant == VARIANT_A2 ? 25 : 5

    let size = (window.innerWidth < 768 ? window.innerWidth : window.innerWidth / 2)
    this.cellSize = (size - 2 * this.padding) / nx

    if (window.innerWidth < 768 && this.cellSize < MIN_CELL_SIZE)
        this.cellSize = MIN_CELL_SIZE

    this.radius = Math.min(Math.max(this.cellSize / 8, MIN_RADIUS), MAX_RADIUS)

    canvas.width = this.padding * 2 + this.cellSize * nx
    canvas.height = this.padding * 2 + this.cellSize * ny
}

Grid.prototype.DrawLine = function(x1, y1, x2, y2) {
    this.ctx.beginPath()
    this.ctx.moveTo(x1, y1)
    this.ctx.lineTo(x2, y2)
    this.ctx.stroke()
}

// проверка, что вершина треугольная
Grid.prototype.IsTriangleVertex = function(v) {
    return v % (this.k1 + 2 * this.k2) >= this.k1
}

// проверка, что треугольная вершина сверху
Grid.prototype.IsUpVertex = function(v) {
    let pos = v % (this.k1 + 2 * this.k2)

    if (this.variant == VARIANT_B1)
        return pos >= this.k1 && !((pos - this.k1) & 1)

    return pos >= this.k1 && (pos - this.k1) & 1
}

// проверка, что треугольная вершина сверху
Grid.prototype.IsDownVertex = function(v) {
    let pos = v % (this.k1 + 2 * this.k2)

    if (this.variant == VARIANT_B1)
        return pos >= this.k1 && (pos - this.k1) & 1

    return pos >= this.k1 && !((pos - this.k1) & 1)
}

// получение координаты вершины на сетке
Grid.prototype.Vertex2Index = function(id) {
    let div = Math.floor(id / (this.k1 + 2 * this.k2))
    let mod = id % (this.k1 + 2 * this.k2)
    let pos = div * (this.k1 + this.k2)

    if (mod < this.k1)
        pos += mod
    else
        pos += Math.floor((mod + this.k1) / 2)

    return pos
}

// получение вершины по индексу
Grid.prototype.Index2Vertex = function(index) {
    let div = Math.floor(index / (this.k1 + this.k2))
    let mod = index % (this.k1 + this.k2)

    let vertex = div * (this.k1 + 2 * this.k2)

    if (mod < this.k1)
        vertex += mod
    else
        vertex += mod * 2 - this.k1

    return vertex
}

// номер процесса в строку начала области
Grid.prototype.Process2StartRow = function(idy) {
    return idy * Math.floor(this.ny / this.py) + Math.min(idy, this.ny % this.py)
}

// номер процесса в столбце начала области
Grid.prototype.Process2StartColumn = function(idx) {
    return idx * Math.floor(this.nx / this.px) + Math.min(idx, this.nx % this.px)
}

// количество вершин в области
Grid.prototype.GetOwnVerticesCountInArea = function(i_start, i_end, j_start, j_end) {
    let vertices = 0

    for (let i = i_start; i < i_end; i++) {
        for (let j = j_start; j < j_end; j++) {
            let vertex = this.Index2Vertex(i * this.nx + j)

            if (this.IsTriangleVertex(vertex)) {
                vertices += 2
            }
            else {
                vertices++
            }
        }
    }

    return vertices
}

// количество HALO вершин в области
Grid.prototype.GetHaloVericesInArea = function(i_start, i_end, j_start, j_end) {
    let vertices = 0

    if (i_start > 0)
        vertices += j_end - j_start

    if (j_start > 0)
        vertices += i_end - i_start

    if (i_end < this.ny)
        vertices += j_end - j_start

    if (j_end < this.nx)
        vertices += i_end - i_start

    return vertices
}

Grid.prototype.GetAreaInfo = function(id) {
    let idx = id % this.px;
    let idy = Math.floor(id / this.px);

    let i_start = this.Process2StartRow(idy);
    let i_end = this.Process2StartRow(idy + 1);

    // границы области процесса по горизонтали
    let j_start = this.Process2StartColumn(idx);
    let j_end = this.Process2StartColumn(idx + 1);

    let ownVertices = this.GetOwnVerticesCountInArea(i_start, i_end, j_start, j_end); // количество собственных вершин в области
    let haloVertices = this.GetHaloVericesInArea(i_start, i_end, j_start, j_end); // количество HALO вершин в области

    g2l = []
    l2g = []
    part = []

    let local = 0

    for (let i = i_start; i < i_end; i++) {
        for (let j = j_start; j < j_end; j++) {
            let vertex = this.Index2Vertex(i * this.nx + j);

            g2l[local] = local;
            l2g[local] = vertex;
            part[local++] = id;

            if (this.IsTriangleVertex(vertex)) {
                g2l[local] = local;
                l2g[local] = vertex + 1;
                part[local++] = id;                 
            }
        }
    }

    if (i_start > 0) {
        for (let j = j_start; j < j_end; j++) {
            let vertex = this.Index2Vertex((i_start - 1) * this.nx + j);

            if (this.variant == VARIANT_B1) {
                if (this.IsTriangleVertex(vertex))
                    vertex++;
            }

            g2l[local] = local;
            l2g[local] = vertex;
            part[local++] = id - this.px;
        }
    }

    for (let i = i_start; i < i_end; i++) {
        if (j_start > 0) {
            let vertex = this.Index2Vertex(i * this.nx + j_start - 1);

            if (this.IsTriangleVertex(vertex))
                vertex++;

            g2l[local] = local;
            l2g[local] = vertex;
            part[local++] = id - 1;
        }

        if (j_end < this.nx) {
            let vertex = this.Index2Vertex(i * this.nx + j_end);

            g2l[local] = local;
            l2g[local] = vertex;
            part[local++] = id + 1;
        }
    }

    if (i_end < this.ny) {
        for (let j = j_start; j < j_end; j++) {
            let vertex = this.Index2Vertex(i_end * this.nx + j);

            if (this.variant == VARIANT_B2) {
                if (this.IsTriangleVertex(vertex))
                    vertex++;
            }

            g2l[local] = local;
            l2g[local] = vertex;
            part[local++] = id + this.px;
        }
    }

    return { ownVertices: ownVertices, haloVertices: haloVertices, g2l: g2l, l2g: l2g, part: part }
}

// формирование вершин для варианта A
Grid.prototype.MakeVericesVariantA = function() {
    for (let i = 0; i <= this.ny; i++) {
        for (let j = 0; j <= this.nx; j++) {
            let x = this.padding + j * this.cellSize;
            let y = this.padding + i * this.cellSize;

            this.vertices.push({ x: x, y: y, id: i * (this.nx + 1) + j })
        }
    }
}

// формирование вершин
Grid.prototype.MakeVerices = function() {
    let index = 0
    let vertex = 0
    let rectangles = 0
    let triangles = 0

    this.vertices = []

    while (index < this.nx * this.ny) {
        for (let k = 0; k < this.k1 && index < this.nx * this.ny; k++) {
            let x = this.padding + (index % this.nx) * this.cellSize;
            let y = this.padding + Math.floor(index / this.nx) * this.cellSize;

            if (this.variant == VARIANT_B1 || this.variant == VARIANT_B2)
                this.vertices.push({ x: x + this.cellSize / 2, y: y + this.cellSize / 2, id: vertex++ })

            rectangles++
            index++
        }

        for (let k = 0; k < this.k2 && index < this.nx * this.ny; k++) {
            let x = this.padding + (index % this.nx) * this.cellSize
            let y = this.padding + Math.floor(index / this.nx) * this.cellSize

            if (this.variant == VARIANT_B1) {
                this.vertices.push({ x: x + this.cellSize / 3, y: y + this.cellSize / 3, id: vertex++ })
                this.vertices.push({ x: x + this.cellSize * 2 / 3, y: y + this.cellSize * 2 / 3, id: vertex++ })
            }
            else if (this.variant == VARIANT_B2) {
                this.vertices.push({ x: x + this.cellSize / 3, y: y + this.cellSize * 2 / 3, id: vertex++ })
                this.vertices.push({ x: x + this.cellSize * 2 / 3, y: y + this.cellSize / 3, id: vertex++ })
            }

            triangles += 2
            index++
        }
    }

    if (this.variant == VARIANT_A1 || this.variant == VARIANT_A2)
        this.MakeVericesVariantA()

    this.result.innerHTML = "<b>Вариант:</b> " + this.variant
    this.result.innerHTML += "<br><b>Размер сетки:</b> " + this.nx + "x" + this.ny
    this.result.innerHTML += "<br><b>Параметры разбиения</b> " + this.k1 + ", " + this.k2
    this.result.innerHTML += "<br><b>Точность решения</b> " + this.eps
    this.result.innerHTML += "<br><br><b>Прямоугольников:</b> " + rectangles
    this.result.innerHTML += "<br><b>Треугольников:</b> " + triangles
    this.result.innerHTML += "<br><b>Всего вершин:</b> " + this.vertices.length
    this.result.innerHTML += "<br>"
}

// формирование рёбер для заданной вершины для варианта B1
Grid.prototype.MakeEdgesForVertexB1 = function(v, x, y) {
    if (y > 0 && !this.IsDownVertex(v)) {
        let vertex = this.Index2Vertex((y - 1) * this.nx + x);

        if (this.IsTriangleVertex(vertex))
            vertex++;

        this.edges[v].push(vertex)
    }

    // соседняя слева
    if (x > 0 || (x == 0 && this.IsDownVertex(v)))
        this.edges[v].push(v - 1)

    this.edges[v].push(v)

    // соседняя справа
    if (x < this.nx - 1 || (x == this.nx - 1 && this.IsUpVertex(v)))
        this.edges[v].push(v + 1)

    // соседняя снизу, если не верхнетреугольная ячейка
    if (y < this.ny - 1 && !this.IsUpVertex(v))
        this.edges[v].push(this.Index2Vertex((y + 1) * this.nx + x))
}

// формирование рёбер для заданной вершины для варианта B2
Grid.prototype.MakeEdgesForVertexB2 = function(v, x, y) {
    // соседняя сверху, если не нижнетреугольная ячейка
    if (y > 0 && !this.IsDownVertex(v))
        this.edges[v].push(this.Index2Vertex((y - 1) * this.nx + x))

    // соседняя слева
    if (x > 0 || (x == 0 && this.IsUpVertex(v)))
        this.edges[v].push(v - 1)

    this.edges[v].push(v)

    // соседняя справа
    if (x < this.nx - 1 || (x == this.nx - 1 && this.IsDownVertex(v)))
        this.edges[v].push(v + 1)

    // соседняя снизу, если не верхнетреугольная ячейка
    if (y < this.ny - 1 && !this.IsUpVertex(v)) {
        let vertex = this.Index2Vertex((y + 1) * this.nx + x);

        if (this.IsTriangleVertex(vertex))
            vertex++;

        this.edges[v].push(vertex)
    }
}

// формирование рёбер для заданной вершины для вариантов A
Grid.prototype.MakeEdgesForVertexA = function(v) {
    let dx = [ 0, -1, 0, 1, 0 ]
    let dy = [ -1, 0, 0, 0, 1 ]
    let vx = v % (this.nx + 1)
    let vy = Math.floor(v / (this.nx + 1))

    for (let k = 0; k < 5; k++) {
        let x = vx + dx[k]
        let y = vy + dy[k]

        if (x < 0 || y < 0 || x > this.nx || y > this.ny)
            continue

        this.edges[v].push(y * (this.nx + 1) + x)
    }
}

// формирование рёбер для заданной вершины для вариантов B
Grid.prototype.MakeEdgesForVertexB = function(v) {
    let index = this.Vertex2Index(v)
    let x = index % this.nx
    let y = Math.floor(index / this.nx)

    if (this.variant == VARIANT_B1) {
        this.MakeEdgesForVertexB1(v, x, y)
    }
    else if (this.variant == VARIANT_B2) {
        this.MakeEdgesForVertexB2(v, x, y)
    }
}

// добавление диагональных элементов для варианта А
Grid.prototype.AddDiagonalsForA = function() {
    for (let index = this.k1; index < this.nx * this.ny; index += this.k1 + this.k2) {
        for (let k = 0; k < this.k2 && index + k < this.nx * this.ny; k++) {
            let i = Math.floor((index + k) / this.nx)
            let j = (index + k) % this.nx

            if (this.variant == VARIANT_A1) {
                this.edges[i * (this.nx + 1) + j + 1].push((i + 1) * (this.nx + 1) + j)
                this.edges[(i + 1) * (this.nx + 1) + j].push(i * (this.nx + 1) + j + 1)
            }
            else {
                this.edges[i * (this.nx + 1) + j].push((i + 1) * (this.nx + 1) + j + 1)
                this.edges[(i + 1) * (this.nx + 1) + j + 1].push(i * (this.nx + 1) + j)
            }
        }
    }

    for (let v = 0; v < this.vertices.length; v++)
        this.edges[v].sort(function(a, b) { return a - b })
}

// формирование рбер
Grid.prototype.MakeEdges = function() {
    this.edges = []

    for (let v = 0; v < this.vertices.length; v++) {
        this.edges[v] = []

        if (this.variant == VARIANT_A1 || this.variant == VARIANT_A2) {
            this.MakeEdgesForVertexA(v)
        }
        else if (this.variant == VARIANT_B1 || this.variant == VARIANT_B2) {
            this.MakeEdgesForVertexB(v)
        }
    }

    if (this.variant == VARIANT_A1 || this.variant == VARIANT_A2)
        this.AddDiagonalsForA()
}

// формирование массива IA
Grid.prototype.MakeIA = function() {
    let ia = []
    ia[0] = 0

    for (let i = 0; i < this.vertices.length; i++)
        ia[i + 1] = ia[i] + this.edges[i].length

    return ia
}

// формирование массива JA
Grid.prototype.MakeJA = function() {
    let ja = []
    let index = 0

    for (let i = 0; i < this.vertices.length; i++)
        for (let j = 0; j < this.edges[i].length; j++)
            ja[index++] = this.edges[i][j]

    return ja
}

// формирование списка смежности
Grid.prototype.MakeList = function() {
    let list = "<br><b>Список смежности:</b><br>"

    for (let i = 0; i < this.edges.length; i++)
        list += "<b>" + i + "</b>&rarr;[" + this.edges[i].join(", ") + "]<br>"

    return list
}

Grid.prototype.MakeSolve = function(ia, ja, a, b) {
    let solver = new Solver(ia.length - 1, ia, ja, a, b, this.eps)
    let solve = solver.Solve()

    let html = "<br><b>Метод сопряжённых градиентов:</b>"

    html += "<ul>"
    for (let i = 0; i < solve.iterations.length; i++)
        html += "<li>итерация" + solve.iterations[i].iteration + ", ||b-Ax||: " + solve.iterations[i].norm + ", rho: " + solve.iterations[i].rho + "</li>"
    html += "</ul>"

    if (document.getElementById("solve-box").checked)
        html += "<b>x</b>: [" + solve.x.join(", ") + "]<br>"

    return html
}

Grid.prototype.Round = function(x) {
    return Math.round(x * 100000) / 100000
}

Grid.prototype.MakeFill = function(ia, ja, a, b) {
    let html = "<br><b>Коэффициенты матрицы и правой части</b><br>"

    for (let i = 0; i < ia.length - 1; i++) {
        let row = []

        for (let j = ia[i]; j < ia[i + 1]; j++)
            row.push(this.Round(a[j]))

        html += i + "&rarr;[" + row.join(", ") + "] = " + this.Round(b[i]) + "<br>"
    }

    return html
}

Grid.prototype.Fa = function(i, j) {
    return Math.cos(i*j + i + j)
}

Grid.prototype.Fb = function(i) {
    return Math.sin(i)
}

// заполнение массисво
Grid.prototype.Fill = function(ia, ja) {
    let a = []
    let b = []

    for (let i = 0; i < ia.length; i++) {
        let sum = 0
        let diagIndex = -1;

        for (let index = ia[i]; index < ia[i + 1]; index++) {
            let j = ja[index]

            if (i != j) {
                a[index] = this.Fa(i, j) // a_ij
                sum += Math.abs(a[index]) // наразиваем сумму внедиагональных элементов
            }
            else {
                diagIndex = index;
            }
        }

        a[diagIndex] = 1.234 * sum
        b[i] = this.Fb(i)
    }

    return { a: a, b: b }
}

// отрисовка вершины
Grid.prototype.DrawVertex = function(vertex) {
    this.ctx.strokeStyle = EDGE_COLOR
    this.ctx.lineWidth = EDGE_LINE_WIDTH

    if (this.IsTriangleVertex(vertex.id)) {
        this.ctx.fillStyle = this.IsUpVertex(vertex.id) ? UP_TRIANGLE_COLOR : DOWN_TRIANGLE_COLOR
    }
    else {
        this.ctx.fillStyle = RECTANGLE_COLOR
    }

    this.ctx.beginPath()
    this.ctx.arc(vertex.x, vertex.y, this.radius, 0, Math.PI * 2)
    this.ctx.fill()
    this.ctx.stroke()

    this.ctx.fillStyle = '#fff'
    this.ctx.textAlign = 'center'
    this.ctx.textBaseline = 'middle'
    this.ctx.font = this.radius + 'px serif'
    this.ctx.fillText(vertex.id, vertex.x, vertex.y)
}

// отрисовка сетки
Grid.prototype.DrawGrid = function() {
    this.ctx.fillStyle = '#fff'
    this.ctx.fillRect(0, 0, 2 * this.padding + this.cellSize * this.nx, 2 * this.padding + this.cellSize * this.ny)
    this.ctx.strokeStyle = '#000'
    this.ctx.fillStyle = '#03a9f4'

    let p = this.px * this.py
    let colors = []

    for (let id = 0; id < p; id++) {
        let idx = id % this.px;
        let idy = Math.floor(id / this.px);

        let i_start = this.Process2StartRow(idy);
        let i_end = this.Process2StartRow(idy + 1);

        // границы области процесса по горизонтали
        let j_start = this.Process2StartColumn(idx);
        let j_end = this.Process2StartColumn(idx + 1);

        let ownVertices = this.GetOwnVerticesCountInArea(i_start, i_end, j_start, j_end)
        let haloVertices = this.GetHaloVericesInArea(i_start, i_end, j_start, j_end)

        let x = this.padding + j_start * this.cellSize
        let y = this.padding + i_start * this.cellSize

        colors[id] = 'hsl(' + (360 * id / p) + ', 100%,90%)'
        this.ctx.fillStyle = colors[id]
        this.ctx.fillRect(x, y, this.cellSize * (j_end - j_start), this.cellSize * (i_end - i_start))
    }

    for (let i = 0; i <= this.ny; i++)
        this.DrawLine(this.padding, this.padding + i * this.cellSize, this.padding + this.nx * this.cellSize, this.padding + i * this.cellSize)

    for (let i = 0; i <= this.nx; i++)
        this.DrawLine(this.padding + i * this.cellSize, this.padding, this.padding + i * this.cellSize, this.padding + this.ny * this.cellSize)

    for (let index = this.k1; index < this.nx * this.ny; index += this.k1 + this.k2) {
        for (let k = 0; k < this.k2 && index + k < this.nx * this.ny; k++) {
            let x = this.padding + ((index + k) % this.nx) * this.cellSize
            let y = this.padding + Math.floor((index + k) / this.nx) * this.cellSize

            if (this.variant == VARIANT_A1 || this.variant == VARIANT_B1) {
                this.DrawLine(x + this.cellSize, y, x, y + this.cellSize)
            }
            else {
                this.DrawLine(x, y, x + this.cellSize, y + this.cellSize)
            }
        }
    }
}

// отрисовка рёбер
Grid.prototype.DrawEdges = function() {
    for (let i = 0; i < this.edges.length; i++) {
        for (let j = 0; j < this.edges[i].length; j++) {
            let x1 = this.vertices[i].x
            let y1 = this.vertices[i].y

            let x2 = this.vertices[this.edges[i][j]].x
            let y2 = this.vertices[this.edges[i][j]].y

            this.ctx.strokeStyle = EDGE_COLOR
            this.ctx.lineWidth = EDGE_LINE_WIDTH
            this.DrawLine(x1, y1, x2, y2)
        }
    }
}

// отрисовка вершин
Grid.prototype.DrawVertices = function() {
    for (let i = 0; i < this.vertices.length; i++)
        this.DrawVertex(this.vertices[i])
}

Grid.prototype.AddResults = function() {
    let ia = this.MakeIA()
    let ja = this.MakeJA()
    let filled = this.Fill(ia, ja)

    for (let id = 0; id < this.px * this.py; id++) {
        let info = this.GetAreaInfo(id)
        let color = 'hsl(' + (360 * id / (this.px * this.py)) + ', 100%,90%)'

        this.result.innerHTML += "<h3 style='margin-bottom: 0; background:" + color + "'>Процесс P" + id + ":</h3><ul style='margin: 0; padding: 0'>"
        this.result.innerHTML += "<li><b>OWN</b>: " + info.ownVertices + "</li>"
        this.result.innerHTML += "<li><b>HALO</b>: " + info.haloVertices + "</li>"
        this.result.innerHTML += "<li><b>G2L</b>: [" + info.g2l.join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>L2G</b>: [" + info.l2g.join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>Part</b>: [" + info.part.join(", ") + "]</li>"
        this.result.innerHTML += "</ul></p>"
    }

    // if (document.getElementById("edge-list-box").checked)
    //     this.result.innerHTML += this.MakeList()

    // if (document.getElementById("portrait-box").checked) {
    //     this.result.innerHTML += "<br><b>IA:</b> [" + ia.join(", ") + "]"
    //     this.result.innerHTML += "<br><b>JA:</b> [" + ja.join(", ") + "]"
    //     this.result.innerHTML += "<br>"
    // }

    // if (document.getElementById("matrix-box").checked)
    //     this.result.innerHTML += this.MakeFill(ia, ja, filled.a, filled.b)

    // this.result.innerHTML += this.MakeSolve(ia, ja, filled.a, filled.b)
}

// отрисовка
Grid.prototype.Draw = function() {
    this.MakeVerices()
    this.MakeEdges()

    this.DrawGrid()
    this.DrawEdges()
    this.DrawVertices()

    for (let id = 0; id < this.px * this.py; id++) {
        let info = this.GetAreaInfo(id)

        let x = this.padding + this.Process2StartColumn(id % this.px) * this.cellSize
        let y = this.padding + this.Process2StartRow(Math.floor(id / this.px)) * this.cellSize

        this.ctx.fillStyle = '#000'
        this.ctx.textAlign = 'left'
        this.ctx.font = '10px arial'
        this.ctx.fillText("OWN:" + info.ownVertices + ", HALO:" + info.haloVertices + ", N:" + (info.ownVertices + info.haloVertices), x + 10, y + 12)
        this.ctx.fillText("G2L: [" + info.g2l.join(" ") + "]", x + 10, y + 24)
        this.ctx.fillText("L2G: [" + info.l2g.join(" ") + "]", x + 10, y + 36)
        this.ctx.fillText("PART: [" + info.l2g.join(" ") + "]", x + 10, y + 48)
    }

    this.AddResults()
}
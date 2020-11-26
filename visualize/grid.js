const RECTANGLE_COLOR = '#2196f3'//'#009688'
const UP_TRIANGLE_COLOR = '#2196f3'//'#ffbcd4'
const DOWN_TRIANGLE_COLOR = '#2196f3'//'#2196f3'

const EDGE_COLOR = '#ff5722'
const EDGE_LINE_WIDTH = 3

const MIN_CELL_SIZE = 80
const MIN_RADIUS = 12
const MAX_RADIUS = 20

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

    this.padding = 5

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
    let processCount = this.px * this.py

    let idx = id % this.px;
    let idy = Math.floor(id / this.px);

    let i_start = this.Process2StartRow(idy);
    let i_end = this.Process2StartRow(idy + 1);

    // границы области процесса по горизонтали
    let j_start = this.Process2StartColumn(idx);
    let j_end = this.Process2StartColumn(idx + 1);

    let ownVertices = this.GetOwnVerticesCountInArea(i_start, i_end, j_start, j_end); // количество собственных вершин в области
    let haloVertices = this.GetHaloVericesInArea(i_start, i_end, j_start, j_end); // количество HALO вершин в области
    let totalVertices = ownVertices + haloVertices

    l2g = []
    part = []

    let local = 0

    for (let i = i_start; i < i_end; i++) {
        for (let j = j_start; j < j_end; j++) {
            let vertex = this.Index2Vertex(i * this.nx + j);

            l2g[local] = vertex;
            part[local++] = id;

            if (this.IsTriangleVertex(vertex)) {
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

            l2g[local] = vertex;
            part[local++] = id - this.px;
        }
    }

    for (let i = i_start; i < i_end; i++) {
        if (j_start > 0) {
            let vertex = this.Index2Vertex(i * this.nx + j_start - 1);

            if (this.IsTriangleVertex(vertex))
                vertex++;

            l2g[local] = vertex;
            part[local++] = id - 1;
        }

        if (j_end < this.nx) {
            let vertex = this.Index2Vertex(i * this.nx + j_end);

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

            l2g[local] = vertex;
            part[local++] = id + this.px;
        }
    }

    let global2local = {}

    for (let i = 0; i < totalVertices; i++)
        global2local[l2g[i]] = i

    edges = []
    ia = []
    ia[0] = 0;

    for (let i = 0; i < ownVertices; i++) {
        edges[i] = this.MakeEdgesForVertex(l2g[i])
        ia[i + 1] = ia[i] + edges[i].length;
    }

    ja = [];
    jag = []

    for (let i = 0; i < ownVertices; i++) {
        for (let j = 0; j < edges[i].length; j++) {
            ja[ia[i] + j] = global2local[edges[i][j]];
            jag[ia[i] + j] = edges[i][j];
        }
    }

    a = []
    b = []

    for (let i = 0; i < ownVertices; i++) {
        let sum = 0
        let diagIndex = -1;

        for (let index = ia[i]; index < ia[i + 1]; index++) {
            let j = ja[index];

            if (i != j) {
                a[index] = this.Fa(l2g[i], l2g[j]); // a_ij
                sum += Math.abs(a[index]); // наразиваем сумму внедиагональных элементов
            }
            else {
                diagIndex = index;
            }
        }

        a[diagIndex] = 1.234 * sum;
    }

    for (let i = 0; i < totalVertices; i++)
        b[i] = this.Fb(l2g[i]);

    sendToProcess = []
    recvFromProcess = []

    for (let i = 0; i < processCount; i++) {
        sendToProcess[i] = []
        recvFromProcess[i] = []
    }

    for (let i = 0; i < ownVertices; i++) {
        for (let index = ia[i]; index < ia[i + 1]; index++) {
            if (ja[index] >= ownVertices) {
                sendToProcess[part[ja[index]]].push(i);
                recvFromProcess[part[ja[index]]].push(ja[index]);
            }
        }
    }

    neighbours = []
    sendOffset = [0]
    recvOffset = [0]
    send = []
    recv = []

    for (let p = 0; p < processCount; p++) {
        if (sendToProcess[p].length == 0)
            continue

        neighbours.push(p)
        for (let i = 0; i < sendToProcess[p].length; i++) {
            send.push(sendToProcess[p][i]);
            recv.push(recvFromProcess[p][i]);
        }

        sendOffset.push(send.length);
        recvOffset.push(recv.length);
    }

    return {
        ownVertices: ownVertices,
        haloVertices: haloVertices,
        totalVertices: totalVertices,
        l2g: l2g,
        part: part,
        global2local: global2local,
        edges: edges,
        ia: ia,
        ja: ja,
        jag: jag,
        a: a,
        b: b,

        sendToProcess: sendToProcess,
        recvFromProcess: recvFromProcess,
        neighbours: neighbours,
        sendOffset: sendOffset,
        recvOffset: recvOffset,
        send: send,
        recv: recv
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

    this.result.innerHTML = "<b>Вариант:</b> " + this.variant
    this.result.innerHTML += "<br><b>Размер сетки:</b> " + this.nx + "x" + this.ny
    this.result.innerHTML += "<br><b>Параметры разбиения</b> " + this.k1 + ", " + this.k2
    this.result.innerHTML += "<br><b>Точность решения</b> " + this.eps
    // this.result.innerHTML += "<br><br><b>Прямоугольников:</b> " + rectangles
    // this.result.innerHTML += "<br><b>Треугольников:</b> " + triangles
    // this.result.innerHTML += "<br><b>Всего вершин:</b> " + this.vertices.length
    this.result.innerHTML += "<br>"
}

// формирование рёбер для заданной вершины для варианта B1
Grid.prototype.MakeEdgesForVertexB1 = function(v, x, y) {
    let edges = []

    if (y > 0 && !this.IsDownVertex(v)) {
        let vertex = this.Index2Vertex((y - 1) * this.nx + x);

        if (this.IsTriangleVertex(vertex))
            vertex++;

        this.edges[v].push(vertex)
        edges.push(vertex)
    }

    // соседняя слева
    if (x > 0 || (x == 0 && this.IsDownVertex(v))) {
        this.edges[v].push(v - 1)
        edges.push(v - 1)
    }

    this.edges[v].push(v)
    edges.push(v)

    // соседняя справа
    if (x < this.nx - 1 || (x == this.nx - 1 && this.IsUpVertex(v))) {
        this.edges[v].push(v + 1)
        edges.push(v + 1)
    }

    // соседняя снизу, если не верхнетреугольная ячейка
    if (y < this.ny - 1 && !this.IsUpVertex(v)) {
        this.edges[v].push(this.Index2Vertex((y + 1) * this.nx + x))
        edges.push(this.Index2Vertex((y + 1) * this.nx + x))
    }

    return edges
}

// формирование рёбер для заданной вершины для варианта B2
Grid.prototype.MakeEdgesForVertexB2 = function(v, x, y) {
    let edges = []

    // соседняя сверху, если не нижнетреугольная ячейка
    if (y > 0 && !this.IsDownVertex(v)) {
        this.edges[v].push(this.Index2Vertex((y - 1) * this.nx + x))
        edges.push(this.Index2Vertex((y - 1) * this.nx + x))
    }

    // соседняя слева
    if (x > 0 || (x == 0 && this.IsUpVertex(v))) {
        this.edges[v].push(v - 1)
        edges.push(v - 1)
    }

    this.edges[v].push(v)
    edges.push(v)

    // соседняя справа
    if (x < this.nx - 1 || (x == this.nx - 1 && this.IsDownVertex(v))) {
        this.edges[v].push(v + 1)
        edges.push(v + 1)
    }

    // соседняя снизу, если не верхнетреугольная ячейка
    if (y < this.ny - 1 && !this.IsUpVertex(v)) {
        let vertex = this.Index2Vertex((y + 1) * this.nx + x);

        if (this.IsTriangleVertex(vertex))
            vertex++;

        this.edges[v].push(vertex)
        edges.push(vertex)
    }

    return edges
}

// формирование рёбер для заданной вершины для вариантов B
Grid.prototype.MakeEdgesForVertex = function(v) {
    let index = this.Vertex2Index(v)
    let x = index % this.nx
    let y = Math.floor(index / this.nx)

    if (this.variant == VARIANT_B1) {
        return this.MakeEdgesForVertexB1(v, x, y)
    }
    else if (this.variant == VARIANT_B2) {
        return this.MakeEdgesForVertexB2(v, x, y)
    }
}

// формирование рбер
Grid.prototype.MakeEdges = function() {
    this.edges = []

    for (let v = 0; v < this.vertices.length; v++) {
        this.edges[v] = []
        this.MakeEdgesForVertex(v)
    }
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

Grid.prototype.MakeGlobal = function(vec, l2g) {
    let v = []

    for (let i = 0; i < vec.length; i++)
        v[i] = l2g[vec[i]]

    return v
}

// формирование списка смежности
Grid.prototype.MakeList = function(edges, l2g, ownVertices) {
    let list = "<b>Список смежности:</b><br>"

    for (let i = 0; i < ownVertices; i++)
        list += "<b style='margin-left: 22px'>" + i + " (" + l2g[i] + ")</b>&rarr;[" + edges[i].join(", ") + "]<br>"

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

Grid.prototype.MakeFill = function(ia, ja, a, b, l2g) {
    let html = "<b>Коэффициенты матрицы</b><br>"

    for (let i = 0; i < ia.length - 1; i++) {
        let row = []

        for (let j = ia[i]; j < ia[i + 1]; j++)
            row.push(this.Round(a[j]))

        html += "<b style='margin-left: 22px'>" + i + " (" + l2g[i] + ")</b>&rarr;[" + row.join(", ") + "]<br>"
    }


    br = []
    for (let i = 0; i < b.length; i++)
        br[i] = this.Round(b[i])

    html += "</li><br><li><b>Вектор правой части</b>: [" + br.join(", ") + "]"
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

            if (this.variant == VARIANT_B1) {
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
    let processCount = this.px * this.py

    for (let id = 0; id < processCount; id++) {
        let info = this.GetAreaInfo(id)
        let color = 'hsl(' + (360 * id / (this.px * this.py)) + ', 100%,90%)'

        this.result.innerHTML += "<h3 style='margin-bottom: 0; background:" + color + "'>Процесс P" + id + ":</h3>"
        this.result.innerHTML += "<ul style='margin: 0; padding: 0'>"
        this.result.innerHTML += "<li><b>OWN (No)</b>: " + info.ownVertices + "</li>"
        this.result.innerHTML += "<li><b>HALO</b>: " + info.haloVertices + "</li>"
        this.result.innerHTML += "<li><b>TOTAL (N)</b>: " + info.totalVertices + "</li>"

        this.result.innerHTML += "<br><li><b>L2G</b>: [" + info.l2g.join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>Part</b>: [" + info.part.join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>IA</b>: [" + info.ia.join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>JA</b>: [" + info.ja.join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>JA (GLOBAL)</b>: [" + info.jag.join(", ") + "]</li>"

        this.result.innerHTML += "<br><li>" + this.MakeList(info.edges, info.l2g, info.ownVertices) + "</li>"

        this.result.innerHTML += "<br><li>" + this.MakeFill(info.ia, info.ja, info.a, info.b, info.l2g) + "</li>"

        for (let i = 0; i < processCount; i++) {
            let sendToProcess = info.sendToProcess[i]
            let recvFromProcess = info.recvFromProcess[i]

            if (sendToProcess.length > 0) {
                this.result.innerHTML += "<br><li><b>SendToProcess" + i + " (GLOBAL)</b>: [" + this.MakeGlobal(sendToProcess, info.l2g).join(", ") + "]</li>"
                this.result.innerHTML += "<li><b>RecvFromProcess" + i + " (GLOBAL)</b>: [" + this.MakeGlobal(recvFromProcess, info.l2g).join(", ") + "]</li>"
            }
        }

        this.result.innerHTML += "<br><li><b>Neighbours</b>: [" + info.neighbours.join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>Send</b>: [" + this.MakeGlobal(info.send, info.l2g).join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>SendOffset (GLOBAL)</b>: [" + info.sendOffset.join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>Recv</b>: [" + this.MakeGlobal(info.recv, info.l2g).join(", ") + "]</li>"
        this.result.innerHTML += "<li><b>RecvOffset (GLOBAL)</b>: [" + info.recvOffset.join(", ") + "]</li>"
    }

    // if (document.getElementById("edge-list-box").checked)

    // if (document.getElementById("portrait-box").checked) {
    //     this.result.innerHTML += "<br><b>IA:</b> [" + ia.join(", ") + "]"
    //     this.result.innerHTML += "<br><b>JA:</b> [" + ja.join(", ") + "]"
    //     this.result.innerHTML += "<br>"
    // }

    // this.result.innerHTML += this.MakeSolve(ia, ja, filled.a, filled.b)
}

// отрисовка
Grid.prototype.Draw = function() {
    this.MakeVerices()
    this.MakeEdges()

    this.DrawGrid()
    this.DrawEdges()
    this.DrawVertices()
    this.AddResults()
}
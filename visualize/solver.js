function Solver(n, ia, ja, a, b, eps) {
    this.n = n
    this.ia = ia
    this.ja = ja

    this.a = a
    this.b = b
    this.eps = eps
}

Solver.prototype.Dot = function(x, y, n) {
    let dot = 0

    for (let i = 0; i < n; i++)
        dot += x[i] * y[i]

    return dot
}

// линейная комбинация x = ax + by
Solver.prototype.LinearCombination = function(a, x, b, y, n) {
    for (let i = 0; i < n; i++) {
        x[i] = x[i] * a + y[i] * b
    }
}

// произведение матрицы на вектор
Solver.prototype.MatrixVectorMultiplication = function(ia, ja, a, n, x, result) {
    for (let i = 0; i < n; i++) {
        let sum = 0

        for (let index = ia[i]; index < ia[i + 1]; index++)
            sum += a[index] * x[ja[index]]

        result[i] = sum
    }
}

// покомпонентное произведение векторов
Solver.prototype.VectorVectorMultiplication = function(x, y, result, n) {
    for (let i = 0; i < n; i++) {
        result[i] = x[i] * y[i]
    }
}

// получение нормы невязки
Solver.prototype.GetResidualNorm = function(x, r) {
    this.MatrixVectorMultiplication(this.ia, this.ja, this.a, this.n, x, r); // r = Ax
    this.LinearCombination(-1, r, 1, this.b, this.n); // r = -Ax + b
    
    return Math.sqrt(this.Dot(r, r, this.n)); // ||b - Ax||
}

// инициализация начальных значений
Solver.prototype.InitValues = function(x, r, m) {
    for (let i = 0; i < this.n; i++) {
        x[i] = 0;
        r[i] = this.b[i]; // b - A*O = b

        let diagIndex = this.ia[i];

        while (this.ja[diagIndex] != i)
            diagIndex++;

        m[i] = 1.0 / this.a[diagIndex];
    }
}

// решение системы
Solver.prototype.Solve = function() {
    x = [] // начальное решение
    r = [] // начальная невязка
    m = [] // вектор из обратных элементов диагонали матрицы A

    z_k = []
    p_k = []
    q_k = []
    debug_r = [] // вектор для вычисления невязки в debug режиме

    this.InitValues(x, r, m); // инициализируем значения

    for (let i = 0; i < this.n; i++) {
        z_k[i] = 0
        p_k[i] = 0
        q_k[i] = 0
    }

    let rho_prev = 0 // предыдущее ро
    let iterations = 1 // текущая итерация
    let isConverge = false // сошёлся ли алгоритм
    let iterationsInfo = []

    do {
        this.VectorVectorMultiplication(m, r, z_k, this.n); // z_k = M^-1 * r_k
        let rho_k = this.Dot(r, z_k, this.n); // rho_k = <r_0, z_k>

        iterationsInfo.push({ iteration: iterations, norm: this.GetResidualNorm(x, debug_r), rho: rho_k })

        if (iterations == 1) {
            this.LinearCombination(0, p_k, 1, z_k, this.n); // p_k = z_k
        }
        else {
            this.LinearCombination(rho_k / rho_prev, p_k, 1, z_k, this.n); // p_k = z_k + beta * p_k
        }

        this.MatrixVectorMultiplication(this.ia, this.ja, this.a, this.n, p_k, q_k); // q_k = A * p_k
        let alpha = rho_k / this.Dot(p_k, q_k, this.n); // rho_k / <p_k, q_k>
        this.LinearCombination(1, x, alpha, p_k, this.n); // x = x + alpha * p_k
        this.LinearCombination(1, r, -alpha, q_k, this.n); // r_k = r_k - alpha * q_k

        if (rho_k < this.eps) {
            isConverge = true;
        }
        else {
            iterations++;
        }

        rho_prev = rho_k;
    }
    while (!isConverge);

    return { iterations: iterationsInfo, x: x }
}
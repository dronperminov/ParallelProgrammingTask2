#include "ArgumentParser.h"

// проверка, что строка является целым числом
bool ArgumentParser::IsInteger(const char *s) const {
    for (int i = 0; s[i]; i++)
        if (s[i] < '0' || s[i] > '9')
            return false;

    return s[0] != '\0'; // целое число, если строка не пуста
}

// проверка, что строка является вещественным числом
bool ArgumentParser::IsReal(const char *s) const {
    bool point = false;
    bool exponent = false;

    for (int i = 0; s[i]; i++) {
        if (s[i] == '.') {
            if (point || exponent)
                return false;

            point = true;
        }
        else if (s[i] == 'e') {
            if (exponent || i == 0)
                return false;

            exponent = true;
        }
        else if (s[i] == '-') {
            if (i == 0 || s[i - 1] != 'e')
                return false;
        }
        else if (s[i] < '0' || s[i] > '9') {
            return false;
        }
    }

    return s[0] != '\0'; // вещественное число, если строка не пуста
}

// парсинг отладки
bool ArgumentParser::ParseDebug(char *arg) {
    if (!strcmp(arg, "f")) {
        debug = FULL_DEBUG;
        return true;
    }

    if (!strcmp(arg, "s")) {
        debug = SOLVE_DEBUG;
        return true;
    }

    if (!strcmp(arg, "n")) {
        debug = NO_DEBUG;
        return true;
    }

    std::cout << "Error: debug parameter is invalid" << std::endl;
    return false;
}

// парсинг из файла
bool ArgumentParser::ParseFromFile(const char *path) {
    std::ifstream f(path); // открываем файл на чтение

    if (!f) { // печаль, если файл не открылся
        std::cout << "Error: unable to open input file" << std::endl;
        return false;
    }

    if (!(f >> nx >> ny >> k1 >> k2 >> px >> py >> eps)) {
        std::cout << "Error: unable to read from file" << std::endl;
        f.close(); // закрываем файл
        return false;
    }

    f.close(); // закрываем файл

    if (nx < 1) {
        std::cout << "Error: Nx parameter in file is invalid (" << nx << ")" << std::endl;
        return false;
    }

    if (ny < 1) {
        std::cout << "Error: Ny parameter in file is invalid (" << ny << ")" << std::endl;
        return false;
    }

    if (k1 < 0) {
        std::cout << "Error: k1 parameter in file is invalid (" << k1 << ")" << std::endl;
        return false;
    }

    if (k2 < 0) {
        std::cout << "Error: k2 parameter in file is invalid (" << k2 << ")" << std::endl;
        return false;
    }

    if (px < 1) {
        std::cout << "Error: Px parameter in file is invalid (" << px << ")" << std::endl;
        return false;
    }

    if (py < 1) {
        std::cout << "Error: Py parameter in file is invalid (" << py << ")" << std::endl;
        return false;
    }

    if (eps < 1e-15) {
        std::cout << "Error: eps parameter in file is invalid (" << eps << ")" << std::endl;
        return false;
    }

    if (k1 + k2 == 0) {
        std::cout << "Error: k1 and k2 == 0" << std::endl;
        return false;
    }

    return true;
}

// парсинг из аргументов
bool ArgumentParser::ParseFromArgv(int argc, char **argv) {
    if (!IsInteger(argv[1])) {
        std::cout << "Error: Nx parameter is not integer (" << argv[1] << ")" << std::endl;
        return false;
    }

    if (!IsInteger(argv[2])) {
        std::cout << "Error: Ny parameter is not integer (" << argv[2] << ")" << std::endl;
        return false;
    }

    if (!IsInteger(argv[3])) {
        std::cout << "Error: k1 parameter is not integer (" << argv[3] << ")" << std::endl;
        return false;
    }

    if (!IsInteger(argv[4])) {
        std::cout << "Error: k2 parameter is not integer (" << argv[4] << ")" << std::endl;
        return false;
    }

    if (!IsInteger(argv[5])) {
        std::cout << "Error: Px parameter is not integer (" << argv[5] << ")" << std::endl;
        return false;
    }

    if (!IsInteger(argv[6])) {
        std::cout << "Error: Py parameter is not integer (" << argv[6] << ")" << std::endl;
        return false;
    }

    if (!IsReal(argv[7])) {
        std::cout << "Error: eps parameter is not real (" << argv[5] << ")" << std::endl;
        return false;
    }

    // парсим в настоящие аргументы
    nx = atoi(argv[1]);
    ny = atoi(argv[2]);
    k1 = atoi(argv[3]);
    k2 = atoi(argv[4]);
    px = atoi(argv[5]);
    py = atoi(argv[6]);
    eps = atof(argv[7]);
    
    if (nx == 0 || ny == 0) {
        std::cout << "Error: invalid value of Nx or Ny " << std::endl;
        return false;
    }

    if (k1 + k2 == 0) {
        std::cout << "Error: k1 and k2 == 0" << std::endl;
        return false;
    }

    if (px == 0 || py == 0) {
        std::cout << "Error: invalid value of Px or Py " << std::endl;
        return false;
    }

    if (eps < 1e-15) {
        std::cout << "Error: eps parameter is invalid (" << eps << ")" << std::endl;
        return false;
    }

    return true;
}

bool ArgumentParser::ParseArgs(int argc, char **argv) {
    if (argc != 2 && argc != 3 && argc != 8 && argc != 9) { // если некорректное количество аргументов
        std::cout << "Error: invalid arguments. Try ./main --help for usage";
        return false;
    }

    bool isCorrect = true; // всё ли корректно

    // если запуск вида ./main path [debug]
    if (argc == 2 || argc == 3) {
        isCorrect = ParseFromFile(argv[1]);
    }
    else { // запуск вида ./main nx ny k1 k2 eps T [debug]
        isCorrect = ParseFromArgv(argc, argv);
    }

    if (!isCorrect)
        return false;

    if (argc == 3) {
        isCorrect = ParseDebug(argv[2]);
    }
    else if (argc == 9) {
        isCorrect = ParseDebug(argv[8]);
    }
    else {
        debug = NO_DEBUG;
    }

    return isCorrect;
}

int ArgumentParser::GetNx() const {
    return nx;
}

int ArgumentParser::GetNy() const {
    return ny;
}

int ArgumentParser::GetK1() const {
    return k1;
}

int ArgumentParser::GetK2() const {
    return k2;
}

int ArgumentParser::GetPx() const {
    return px;
}

int ArgumentParser::GetPy() const {
    return py;
}

double ArgumentParser::GetEps() const {
    return eps;
}

int ArgumentParser::GetDebug() const {
    return debug;
}

// вывод аргументов
void ArgumentParser::PrintArguments() const {
    std::cout << "Parsed arguments:" << std::endl;
    std::cout << "Nx: " << nx << std::endl;
    std::cout << "Ny: " << ny << std::endl;
    std::cout << "k1: " << k1 << std::endl;
    std::cout << "k2: " << k2 << std::endl;
    std::cout << "Px: " << px << std::endl;
    std::cout << "Py: " << py << std::endl;
    std::cout << "eps: " << eps << std::endl;
}

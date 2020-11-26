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
        params.debug = FULL_DEBUG;
        return true;
    }

    if (!strcmp(arg, "s")) {
        params.debug = SOLVE_DEBUG;
        return true;
    }

    if (!strcmp(arg, "n")) {
        params.debug = NO_DEBUG;
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

    if (!(f >> params.nx >> params.ny >> params.k1 >> params.k2 >> params.px >> params.py >> params.eps)) {
        std::cout << "Error: unable to read from file" << std::endl;
        f.close(); // закрываем файл
        return false;
    }

    f.close(); // закрываем файл

    if (params.nx < 1) {
        std::cout << "Error: Nx parameter in file is invalid (" << params.nx << ")" << std::endl;
        return false;
    }

    if (params.ny < 1) {
        std::cout << "Error: Ny parameter in file is invalid (" << params.ny << ")" << std::endl;
        return false;
    }

    if (params.k1 < 0) {
        std::cout << "Error: k1 parameter in file is invalid (" << params.k1 << ")" << std::endl;
        return false;
    }

    if (params.k2 < 0) {
        std::cout << "Error: k2 parameter in file is invalid (" << params.k2 << ")" << std::endl;
        return false;
    }

    if (params.px < 1) {
        std::cout << "Error: Px parameter in file is invalid (" << params.px << ")" << std::endl;
        return false;
    }

    if (params.py < 1) {
        std::cout << "Error: Py parameter in file is invalid (" << params.py << ")" << std::endl;
        return false;
    }

    if (params.eps < 1e-15) {
        std::cout << "Error: eps parameter in file is invalid (" << params.eps << ")" << std::endl;
        return false;
    }

    if (params.k1 + params.k2 == 0) {
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
    params.nx = atoi(argv[1]);
    params.ny = atoi(argv[2]);
    params.k1 = atoi(argv[3]);
    params.k2 = atoi(argv[4]);
    params.px = atoi(argv[5]);
    params.py = atoi(argv[6]);
    params.eps = atof(argv[7]);
    
    if (params.nx == 0 || params.ny == 0) {
        std::cout << "Error: invalid value of Nx or Ny " << std::endl;
        return false;
    }

    if (params.k1 + params.k2 == 0) {
        std::cout << "Error: k1 and k2 == 0" << std::endl;
        return false;
    }

    if (params.px == 0 || params.py == 0) {
        std::cout << "Error: invalid value of Px or Py " << std::endl;
        return false;
    }

    if (params.eps < 1e-15) {
        std::cout << "Error: eps parameter is invalid (" << params.eps << ")" << std::endl;
        return false;
    }

    return true;
}

bool ArgumentParser::ParseArgs(int argc, char **argv, int processCount) {
    if (argc != 2 && argc != 3 && argc != 8 && argc != 9) { // если некорректное количество аргументов
        std::cout << "Error: invalid arguments. Try ./main --help for usage";
        return false;
    }

    bool isCorrect = true; // всё ли корректно

    // если запуск вида ./main path [debug]
    if (argc == 2 || argc == 3) {
        isCorrect = ParseFromFile(argv[1]);
    }
    else { // запуск вида ./main nx ny k1 k2 px py eps [debug]
        isCorrect = ParseFromArgv(argc, argv);
    }

    if (!isCorrect)
        return false;

    if (params.px * params.py != processCount) {
        std::cout << "Error: process count != px*py (" << params.px << "x" << params.py << " != " << processCount << ")" << std::endl;
        return false;
    }

    if (argc == 3) {
        isCorrect = ParseDebug(argv[2]);
    }
    else if (argc == 9) {
        isCorrect = ParseDebug(argv[8]);
    }
    else {
        params.debug = NO_DEBUG;
    }

    return isCorrect;
}

TaskParams ArgumentParser::GetParams() const {
    return params;
}

// вывод аргументов
void ArgumentParser::PrintArguments() const {
    std::cout << "Parsed arguments:" << std::endl;
    std::cout << "Nx: " << params.nx << std::endl;
    std::cout << "Ny: " << params.ny << std::endl;
    std::cout << "k1: " << params.k1 << std::endl;
    std::cout << "k2: " << params.k2 << std::endl;
    std::cout << "Px: " << params.px << std::endl;
    std::cout << "Py: " << params.py << std::endl;
    std::cout << "eps: " << params.eps << std::endl;
}

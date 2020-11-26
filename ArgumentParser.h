#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include "Types.h"

class ArgumentParser {
    TaskParams params;

    bool IsInteger(const char *s) const; // проверка, что строка является целым числом
    bool IsReal(const char *s) const; // проверка, что строка является вещественным числом

    bool ParseDebug(char *arg); // парсинг отладки
    bool ParseFromFile(const char *path); // парсинг из файла
    bool ParseFromArgv(int argc, char **argv); // парсинг из аргументов
public:
    bool ParseArgs(int argc, char **argv);

    TaskParams GetParams() const; // получение аргументов
    void PrintArguments() const; // вывод аргументов
};

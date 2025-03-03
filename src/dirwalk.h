#ifndef DIRWALK_H
#define DIRWALK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <locale.h>
#include <errno.h>
#include <unistd.h>

// Максимальная длина пути
#define PATH_MAX 4096

// Структура для хранения коллекции файлов
typedef struct {
    char **items;    // Массив строк (путей к файлам/директориям)
    size_t count;    // Текущее количество элементов
    size_t capacity; // Текущая емкость массива
} FileCollection;

// Структура для хранения опций фильтрации и сортировки
typedef struct {
    int showLinks;  // Показывать символические ссылки
    int showDirs;   // Показывать директории
    int showFiles;  // Показывать файлы
    int sort;       // Сортировать результаты
} FilterOptions;

// Прототипы функций
void fail(const char *message);
void parseArgs(int argc, char *argv[], FilterOptions *options, const char **startDir);
void addFile(FileCollection *files, const char *path);
void clearFileCollection(FileCollection *files);
int compareFileNames(const void *a, const void *b);
int matchesFilter(const struct stat *fileInfo, const FilterOptions *options);
void scanDir(const char *basePath, const FilterOptions *options, FileCollection *files);

#endif // DIRWALK_H

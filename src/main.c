#define _XOPEN_SOURCE 700 // Включение расширенных возможностей POSIX для совместимости

#include "dirwalk.h"

int main(int argc, char *argv[]) {
    // Установка локали для корректной сортировки
    setlocale(LC_COLLATE, "");

    // Инициализация коллекции файлов и опций фильтрации
    FileCollection files = {0};
    FilterOptions options = {0};
    const char *startDir;

    // Парсинг аргументов командной строки
    parseArgs(argc, argv, &options, &startDir);

    // Получение информации о начальной директории
    struct stat fileInfo;
    if (lstat(startDir, &fileInfo) < 0) {
        fail(strerror(errno)); // Обработка ошибки
    }

    // Если начальная директория соответствует фильтру, добавляем её
    if (matchesFilter(&fileInfo, &options)) {
        addFile(&files, startDir);
    }

    // Если начальная директория является каталогом, сканируем её
    if (S_ISDIR(fileInfo.st_mode)) {
        scanDir(startDir, &options, &files);
    }

    // Если включена сортировка, сортируем коллекцию файлов
    if (options.sort) {
        qsort(files.items, files.count, sizeof(char *), compareFileNames);
    }

    // Вывод коллекции файлов
    for (size_t i = 0; i < files.count; i++) {
        printf("%s\n", files.items[i]);
    }

    // Очистка коллекции файлов
    clearFileCollection(&files);

    return 0;
}

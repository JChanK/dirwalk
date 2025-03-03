#define _XOPEN_SOURCE 700  // Включение расширенных возможностей POSIX для совместимости
#include "dirwalk.h"  

// Завершение программы с ошибкой
void fail(const char *message) {
    fprintf(stderr, "Ошибка: %s\n", message);
    exit(EXIT_FAILURE);
}

// Анализ аргументов командной строки
void parseArgs(int argc, char *argv[], FilterOptions *options, const char **startDir) {
    int opt;
    while ((opt = getopt(argc, argv, "ldfs")) != -1) {
        switch (opt) {
            case 'l': options->showLinks = 1; break; // Показывать символические ссылки
            case 'd': options->showDirs = 1; break;  // Показывать директории
            case 'f': options->showFiles = 1; break; // Показывать файлы
            case 's': options->sort = 1; break;      // Сортировать результаты
            default:
                fprintf(stderr, "Использование: %s [опции] [директория]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    *startDir = (optind < argc) ? argv[optind] : "."; // Установка начальной директории
}

// Добавление файла в коллекцию
void addFile(FileCollection *files, const char *path) {
    // Проверка, нужно ли увеличивать емкость коллекции
    if (files->count == files->capacity) {
        // Увеличиваем емкость вдвое или устанавливаем начальную емкость
        size_t newCapacity = (files->capacity == 0) ? 16 : files->capacity * 2;
        // Перераспределение памяти
        char **tmp = realloc(files->items, newCapacity * sizeof(char*));
        if (!tmp) { // Проверка на успешное перераспределение
            fail("Ошибка выделения памяти");
        }
        files->items = tmp; // Обновляем указатель на массив
        files->capacity = newCapacity; // Обновляем емкость
    }
    // Копируем строку пути и добавляем в коллекцию
    files->items[files->count] = strdup(path);
    if (!files->items[files->count]) { // Проверка на успешное дублирование строки
        fail("Ошибка дублирования строки");
    }
    files->count++; // Увеличиваем счетчик элементов
}

// Очистка коллекции файлов
void clearFileCollection(FileCollection *files) {
    if (files->items) {
        for (size_t i = 0; i < files->count; i++) {
            free(files->items[i]); // Освобождаем каждый элемент
        }
        free(files->items); // Освобождаем массив
        files->items = NULL; // Обнуляем указатель
    }
    files->count = 0;
    files->capacity = 0;
}

// Функция сравнения для сортировки
int compareFileNames(const void *a, const void *b) {
    const char * const *pathA = a;
    const char * const *pathB = b;
    return strcoll(*pathA, *pathB); // Сравнение строк с учетом локали
}

// Проверка, соответствует ли файл фильтру
int matchesFilter(const struct stat *fileInfo, const FilterOptions *options) {
    // Если данные некорректны, считаем, что файл не подходит
    if (!fileInfo || !options) return 0;

    // Проверяем, задан ли хотя бы один фильтр
    int anyFilter = options->showLinks || options->showDirs || options->showFiles;

    // Проверка на соответствие типу файла
    if (options->showLinks && S_ISLNK(fileInfo->st_mode)) return 1; // Символическая ссылка
    if (options->showDirs && S_ISDIR(fileInfo->st_mode)) return 1; // Директория
    if (options->showFiles && S_ISREG(fileInfo->st_mode)) return 1; // Обычный файл

    // Если не указаны специфичные фильтры, разрешаем все типы
    if (!anyFilter) return 1;

    return 0; // Файл не соответствует фильтрам
}

// Сканирование директории
void scanDir(const char *basePath, const FilterOptions *options, FileCollection *files) {
    DIR *dir = opendir(basePath); // Открываем директорию
    if (!dir) {
        fprintf(stderr, "Ошибка при открытии каталога '%s': %s\n", basePath, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем "." и ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[PATH_MAX]; // Массив для хранения полного пути
        snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name); // Формируем полный путь

        struct stat fileInfo;
        // Используем lstat, чтобы не следовать символическим ссылкам
        if (lstat(path, &fileInfo) < 0) {
            fprintf(stderr, "Ошибка при lstat '%s': %s\n", path, strerror(errno));
            continue;
        }

        // Если объект удовлетворяет фильтру, добавляем его
        if (matchesFilter(&fileInfo, options)) {
            addFile(files, path);
        }

        // Если это каталог, за исключением символических ссылок, сканируем рекурсивно
        if (S_ISDIR(fileInfo.st_mode)) {
            scanDir(path, options, files);
        }
    }
    closedir(dir); // Закрываем директорию
}

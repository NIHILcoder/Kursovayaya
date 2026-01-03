/**
 * @file repository.c
 * @brief База данных репозитория
 * @author Костопраев Дмитрий Евгеньевич
 * @date --.--.----
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#define MAX_STR 50
#define MAX_LONG_STR 100
#define INITIAL_CAPACITY 10
#define MAX_FILENAME 256

typedef enum {
    DIRECTION_BACKEND = 0, DIRECTION_FRONTEND, DIRECTION_MOBILE,
    DIRECTION_DEVOPS, DIRECTION_DATA_SCIENCE, DIRECTION_COUNT
} Direction;

typedef enum {
    COMPAT_WINDOWS = 0, COMPAT_LINUX, COMPAT_MACOS,
    COMPAT_CROSS_PLATFORM, COMPAT_COUNT
} Compatibility;

typedef struct { int day, month, year; } Date;

typedef struct {
    Direction direction;
    char site[MAX_LONG_STR];
    char name[MAX_LONG_STR];
    int size;
    Date release_date;
    int dependencies;
    Compatibility compatibility;
} Repository;

typedef struct { int* indices; int count; } SearchResult;

/* Прототипы */
int load_from_file(Repository** records, int* count, const char* filename);
int save_to_file(Repository* records, int count, const char* filename);
int add_record(Repository** records, int* count);
int print_record(Repository* record, int index);
int print_records(Repository* records, int count);
SearchResult search_by_direction(Repository* records, int count, Direction direction);
SearchResult search_combined(Repository* records, int count, Date target_date, int target_size);
int sort_records(Repository* records, int count);
int show_menu();
int read_int();
int read_string(char* buffer, int size);
int free_search_result(SearchResult* result);
Date read_date();
int validate_date(Date date);
int compare_dates(Date d1, Date d2);
const char* direction_to_string(Direction dir);
const char* compatibility_to_string(Compatibility compat);
Direction read_direction();
Compatibility read_compatibility();
int string_to_direction(const char* str, Direction* result);
int string_to_compatibility(const char* str, Compatibility* result);

static const char* dir_names[] = { "Backend", "Frontend", "Mobile", "DevOps", "DataScience" };
static const char* compat_names[] = { "Windows", "Linux", "macOS", "CrossPlatform" };

const char* direction_to_string(Direction dir) {
    return (dir >= 0 && dir < DIRECTION_COUNT) ? dir_names[dir] : "Unknown";
}

const char* compatibility_to_string(Compatibility compat) {
    return (compat >= 0 && compat < COMPAT_COUNT) ? compat_names[compat] : "Unknown";
}

int string_to_direction(const char* str, Direction* result) {
    int i;
    for (i = 0; i < DIRECTION_COUNT; i++) {
        if (strcmp(str, dir_names[i]) == 0) { *result = (Direction)i; return 1; }
    }
    fprintf(stderr, "Ошибка: неизвестное направление '%s'\n", str);
    return 0;
}

int string_to_compatibility(const char* str, Compatibility* result) {
    int i;
    for (i = 0; i < COMPAT_COUNT; i++) {
        if (strcmp(str, compat_names[i]) == 0) { *result = (Compatibility)i; return 1; }
    }
    fprintf(stderr, "Ошибка: неизвестная совместимость '%s'\n", str);
    return 0;
}

int main() {
    Repository* records = NULL;
    int count = 0, running = 1, choice, i;
    Direction search_direction;
    Date search_date;
    int target_size;
    char filename[MAX_FILENAME];
    SearchResult result;

    setlocale(LC_ALL, "");
    printf("=== База данных репозитория ===\n\n");

    while (running) {
        choice = show_menu();
        switch (choice) {
        case 1:
            printf("Введите имя файла для загрузки:  ");
            read_string(filename, MAX_FILENAME);
            if (load_from_file(&records, &count, filename))
                printf("Данные успешно загружены (%d записей)\n", count);
            break;
        case 2:
            if (count == 0) printf("\nБаза данных пуста.\n");
            else { printf("\n=== Список всех записей (%d) ===\n", count); print_records(records, count); }
            break;
        case 3:
            if (count == 0) { printf("\nБаза данных пуста\n"); break; }
            printf("\n--- Поиск по направлению ---\n");
            search_direction = read_direction();
            result = search_by_direction(records, count, search_direction);
            printf("\n=== Результаты поиска ===\nНаправление: %s\n\n", direction_to_string(search_direction));
            if (result.count == 0) printf("Записи не найдены\n");
            else {
                for (i = 0; i < result.count; i++) print_record(&records[result.indices[i]], result.indices[i] + 1);
                printf("\nНайдено:  %d\n", result.count);
            }
            free_search_result(&result);
            break;
        case 4:
            if (count == 0) { printf("\nБаза данных пуста\n"); break; }
            printf("\n--- Комбинированный поиск (дата релиза И размер) ---\n");
            printf("Введите дату релиза:\n");
            search_date = read_date();
            printf("Размер (Кб): ");
            target_size = read_int();
            result = search_combined(records, count, search_date, target_size);
            printf("\n=== Результаты ===\nУсловия:  дата = %02d.%02d.%04d И размер = %d Кб\n\n",
                search_date.day, search_date.month, search_date.year, target_size);
            if (result.count == 0) printf("Записи не найдены\n");
            else {
                for (i = 0; i < result.count; i++) print_record(&records[result.indices[i]], result.indices[i] + 1);
                printf("Найдено:  %d\n", result.count);
            }
            free_search_result(&result);
            break;
        case 5:
            if (count == 0) { printf("\nБаза данных пуста\n"); break; }
            printf("\n--- Сортировка:  Название -> Направление -> Дата релиза (убыв.) ---\n");
            if (sort_records(records, count)) { printf("Сортировка выполнена!\n\n"); print_records(records, count); }
            break;
        case 6:
            if (add_record(&records, &count)) printf("\nЗапись добавлена!\n");
            break;
        case 7:
            if (count == 0) { fprintf(stderr, "Нет данных для сохранения\n"); break; }
            printf("Введите имя файла:  ");
            read_string(filename, MAX_FILENAME);
            if (save_to_file(records, count, filename)) printf("Сохранено в '%s'\n", filename);
            break;
        case 8:
            running = 0;
            printf("\nДо свидания!\n");
            break;
        default:
            printf("Неверный выбор\n");
        }
    }
    if (records != NULL) free(records);
    return 0;
}

int show_menu() {
    printf("\n--- МЕНЮ ---\n");
    printf("1. Загрузить из файла\n2. Просмотреть записи\n3. Поиск по направлению\n");
    printf("4. Комбинированный поиск\n5. Сортировка\n6. Добавить запись\n7. Сохранить\n8. Выход\n");
    printf("Выбор (1-8): ");
    return read_int();
}

int read_int() {
    int value, result;
    result = scanf("%d", &value);
    while (result != 1) { while (getchar() != '\n'); fprintf(stderr, "Ошибка!  Число:  "); result = scanf("%d", &value); }
    while (getchar() != '\n');
    return value;
}

int read_string(char* buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) { buffer[strcspn(buffer, "\n")] = '\0'; return 1; }
    return 0;
}

Date read_date() {
    Date date;
    do {
        printf("День (1-31): "); date.day = read_int();
        printf("Месяц (1-12): "); date.month = read_int();
        printf("Год (1900-2100): "); date.year = read_int();
        if (!validate_date(date)) fprintf(stderr, "Ошибка:  некорректная дата\n");
    } while (!validate_date(date));
    return date;
}

int validate_date(Date date) {
    int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (date.year < 1900 || date.year > 2100 || date.month < 1 || date.month > 12) return 0;
    if ((date.year % 4 == 0 && date.year % 100 != 0) || (date.year % 400 == 0)) days[1] = 29;
    return (date.day >= 1 && date.day <= days[date.month - 1]);
}

int compare_dates(Date d1, Date d2) {
    if (d1.year != d2.year) return d1.year - d2.year;
    if (d1.month != d2.month) return d1.month - d2.month;
    return d1.day - d2.day;
}

Direction read_direction() {
    int choice, i;
    printf("\nВыберите направление:\n");
    for (i = 0; i < DIRECTION_COUNT; i++) printf("%d. %s\n", i + 1, dir_names[i]);
    printf("Выбор (1-%d): ", DIRECTION_COUNT);
    do {
        choice = read_int(); if (choice < 1 || choice > DIRECTION_COUNT) fprintf(stderr, "Ошибка:  1-%d:  ", DIRECTION_COUNT);
    } while (choice < 1 || choice > DIRECTION_COUNT);
    return (Direction)(choice - 1);
}

Compatibility read_compatibility() {
    int choice, i;
    printf("\nВыберите совместимость:\n");
    for (i = 0; i < COMPAT_COUNT; i++) printf("%d. %s\n", i + 1, compat_names[i]);
    printf("Выбор (1-%d): ", COMPAT_COUNT);
    do {
        choice = read_int(); if (choice < 1 || choice > COMPAT_COUNT) fprintf(stderr, "Ошибка: 1-%d: ", COMPAT_COUNT);
    } while (choice < 1 || choice > COMPAT_COUNT);
    return (Compatibility)(choice - 1);
}

int load_from_file(Repository** records, int* count, const char* filename) {
    FILE* file;
    int capacity, read_count, line_number = 0;
    Repository* temp;
    char dir_str[MAX_STR], compat_str[MAX_STR];
    Repository current;

    file = fopen(filename, "r");
    if (file == NULL) { perror("Ошибка открытия файла"); return 0; }
    if (*records != NULL) free(*records);
    capacity = INITIAL_CAPACITY;
    *records = (Repository*)malloc(capacity * sizeof(Repository));
    if (*records == NULL) { fclose(file); perror("Ошибка памяти"); return 0; }
    *count = 0;

    while (1) {
        read_count = fscanf(file, "%49[^\n]\n%99[^\n]\n%99[^\n]\n%d\n%d %d %d\n%d\n%49[^\n]\n",
            dir_str, current.site, current.name, &current.size,
            &current.release_date.day, &current.release_date.month, &current.release_date.year,
            &current.dependencies, compat_str);
        if (read_count != 9) break;
        line_number++;
        if (!string_to_direction(dir_str, &current.direction) ||
            !string_to_compatibility(compat_str, &current.compatibility) ||
            current.size <= 0 || current.dependencies < 0 || !validate_date(current.release_date)) {
            fprintf(stderr, "Ошибка в записи %d\n", line_number);
            fclose(file); free(*records); *records = NULL; *count = 0; return 0;
        }
        (*records)[*count] = current;
        (*count)++;
        if (*count >= capacity) {
            capacity *= 2;
            temp = (Repository*)realloc(*records, capacity * sizeof(Repository));
            if (temp == NULL) { fclose(file); free(*records); *records = NULL; *count = 0; return 0; }
            *records = temp;
        }
    }
    fclose(file);
    if (*count == 0) {
        fprintf(stderr, "Файл пуст\n");
        free(*records);
        *records = NULL;
        return 0;
    }
    return 1;
}

int save_to_file(Repository* records, int count, const char* filename) {
    FILE* file;
    int i;
    file = fopen(filename, "w");
    if (file == NULL) { perror("Ошибка создания файла"); return 0; }
    for (i = 0; i < count; i++)
        fprintf(file, "%s\n%s\n%s\n%d\n%d %d %d\n%d\n%s\n",
            direction_to_string(records[i].direction), records[i].site, records[i].name,
            records[i].size, records[i].release_date.day, records[i].release_date.month,
            records[i].release_date.year, records[i].dependencies,
            compatibility_to_string(records[i].compatibility));
    fclose(file);
    return 1;
}

int add_record(Repository** records, int* count) {
    Repository* temp;
    int value;
    temp = (Repository*)realloc(*records, (*count + 1) * sizeof(Repository));
    if (temp == NULL) { perror("Ошибка памяти"); return 0; }
    *records = temp;
    printf("\n--- Добавление записи ---\n");
    (*records)[*count].direction = read_direction();
    printf("\nСайт: "); read_string((*records)[*count].site, MAX_LONG_STR);
    printf("Название: "); read_string((*records)[*count].name, MAX_LONG_STR);
    printf("Размер (Кб, > 0): "); value = read_int();
    while (value <= 0) { fprintf(stderr, "Ошибка:  > 0\n"); printf("Размер:  "); value = read_int(); }
    (*records)[*count].size = value;
    printf("\nДата релиза:\n"); (*records)[*count].release_date = read_date();
    printf("\nЗависимости (>= 0): "); value = read_int();
    while (value < 0) { fprintf(stderr, "Ошибка: >= 0\n"); printf("Зависимости: "); value = read_int(); }
    (*records)[*count].dependencies = value;
    (*records)[*count].compatibility = read_compatibility();
    (*count)++;
    return 1;
}

int print_record(Repository* record, int index) {
    printf("\n--- Запись %d ---\n", index);
    printf("Направление: %s\nСайт: %s\nНазвание: %s\nРазмер: %d Кб\n",
        direction_to_string(record->direction), record->site, record->name, record->size);
    printf("Дата релиза: %02d.%02d.%04d\nЗависимости: %d\nСовместимость:  %s\n",
        record->release_date.day, record->release_date.month, record->release_date.year,
        record->dependencies, compatibility_to_string(record->compatibility));
    return 1;
}

int print_records(Repository* records, int count) {
    int i;
    for (i = 0; i < count; i++) print_record(&records[i], i + 1);
    return 1;
}

int free_search_result(SearchResult* result) {
    if (result->indices != NULL) { free(result->indices); result->indices = NULL; }
    result->count = 0;
    return 1;
}

SearchResult search_by_direction(Repository* records, int count, Direction direction) {
    SearchResult result = { NULL, 0 };
    int i, capacity = 10;
    int* temp;
    result.indices = (int*)malloc(capacity * sizeof(int));
    if (result.indices == NULL) return result;
    for (i = 0; i < count; i++) {
        if (records[i].direction == direction) {
            if (result.count >= capacity) {
                capacity *= 2;
                temp = (int*)realloc(result.indices, capacity * sizeof(int));
                if (temp == NULL) {
                    free(result.indices);
                    result.indices = NULL;
                    result.count = 0;
                    return result;
                }
                result.indices = temp;
            }
            result.indices[result.count++] = i;
        }
    }
    return result;
}

/* Комбинированный поиск:  дата релиза == target_date И размер == target_size */
SearchResult search_combined(Repository* records, int count, Date target_date, int target_size) {
    SearchResult result = { NULL, 0 };
    int i, capacity = 10;
    int* temp;
    result.indices = (int*)malloc(capacity * sizeof(int));
    if (result.indices == NULL) return result;
    for (i = 0; i < count; i++) {
        if (compare_dates(records[i].release_date, target_date) == 0 &&
            records[i].size == target_size) {
            if (result.count >= capacity) {
                capacity *= 2;
                temp = (int*)realloc(result.indices, capacity * sizeof(int));
                if (temp == NULL) {
                    free(result.indices);
                    result.indices = NULL;
                    result.count = 0;
                    return result;
                }
                result.indices = temp;
            }
            result.indices[result.count++] = i;
        }
    }
    return result;
}

/* Фиксированная сортировка: Название (возр.) -> Направление (возр.) -> Дата релиза (убыв.) */
int sort_records(Repository* records, int count) {
    int i, j, cmp_name, cmp_dir, cmp_date;
    Repository temp;
    for (i = 0; i < count - 1; i++) {
        for (j = i + 1; j < count; j++) {
            cmp_name = strcmp(records[i].name, records[j].name);
            if (cmp_name > 0 ||
                (cmp_name == 0 && records[i].direction > records[j].direction) ||
                (cmp_name == 0 && records[i].direction == records[j].direction &&
                    (cmp_date = compare_dates(records[i].release_date, records[j].release_date)) < 0)) {
                temp = records[i]; records[i] = records[j]; records[j] = temp;
            }
        }
    }
    return 1;
}
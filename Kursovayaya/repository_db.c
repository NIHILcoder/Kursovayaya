/**
 * @file repository_db.c
 * @brief База данных репозитория
 * @author Костопраев Дмитрий Евгеньевич
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "repository.h"

const char* dir_names[] = {
    "Backend", "Frontend", "Mobile", "DevOps", "DataScience"
};

const char* compat_names[] = {
    "Windows", "Linux", "macOS", "CrossPlatform"
};

int db_init(RepositoryDB* db)
{
    if (db == NULL) {
        fprintf(stderr, "Ошибка: передан NULL-указатель в db_init\n");
        return 0;
    }
    
    db->records = (Repository*)malloc(INITIAL_CAPACITY * sizeof(Repository));
    if (db->records == NULL) {
        fprintf(stderr, "Ошибка выделения памяти при инициализации БД\n");
        return 0;
    }
    
    db->count = 0;
    db->capacity = INITIAL_CAPACITY;
    return 1;
}

int db_free(RepositoryDB* db)
{
    if (db == NULL) {
        return 0;
    }
    
    if (db->records != NULL) {
        free(db->records);
        db->records = NULL;
    }
    
    db->count = 0;
    db->capacity = 0;
    return 1;
}

const char* direction_to_string(Direction dir)
{
    if (dir >= 0 && dir < DIRECTION_COUNT) {
        return dir_names[dir];
    }
    return "Unknown";
}

const char* compatibility_to_string(Compatibility compat)
{
    if (compat >= 0 && compat < COMPAT_COUNT) {
        return compat_names[compat];
    }
    return "Unknown";
}

int string_to_direction(const char* str, Direction* result)
{
    int i;
    
    if (str == NULL || result == NULL) {
        return 0;
    }
    
    for (i = 0; i < DIRECTION_COUNT; i++) {
        if (strcmp(str, dir_names[i]) == 0) {
            *result = (Direction)i;
            return 1;
        }
    }
    
    fprintf(stderr, "Ошибка: неизвестное направление '%s'\n", str);
    return 0;
}

int string_to_compatibility(const char* str, Compatibility* result)
{
    int i;
    
    if (str == NULL || result == NULL) {
        return 0;
    }
    
    for (i = 0; i < COMPAT_COUNT; i++) {
        if (strcmp(str, compat_names[i]) == 0) {
            *result = (Compatibility)i;
            return 1;
        }
    }
    
    fprintf(stderr, "Ошибка: неизвестная совместимость '%s'\n", str);
    return 0;
}

int validate_date(Date date)
{
    int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int is_leap;
    
    if (date.year < 1900 || date.year > 2100) {
        return 0;
    }
    
    if (date.month < 1 || date.month > 12) {
        return 0;
    }
    
    is_leap = ((date.year % 4 == 0 && date.year % 100 != 0) || 
               (date.year % 400 == 0));
    if (is_leap) {
        days_in_month[1] = 29;
    }
    
    return (date.day >= 1 && date.day <= days_in_month[date.month - 1]);
}

int compare_dates(Date d1, Date d2)
{
    if (d1.year != d2.year) {
        return d1.year - d2.year;
    }
    if (d1.month != d2.month) {
        return d1.month - d2.month;
    }
    return d1.day - d2.day;
}

static int db_grow_capacity(RepositoryDB* db)
{
    int new_capacity;
    Repository* temp;
    
    new_capacity = db->capacity * 2;
    temp = (Repository*)realloc(db->records, new_capacity * sizeof(Repository));
    
    if (temp == NULL) {
        fprintf(stderr, "Ошибка выделения памяти при расширении БД\n");
        return 0;
    }
    
    db->records = temp;
    db->capacity = new_capacity;
    return 1;
}

int db_load_from_file(RepositoryDB* db, const char* filename)
{
    FILE* file;
    int read_count;
    int line_number = 0;
    char dir_str[MAX_STR];
    char compat_str[MAX_STR];
    Repository current;
    
    if (db == NULL || filename == NULL) {
        fprintf(stderr, "Ошибка: некорректные параметры в db_load_from_file\n");
        return 0;
    }
    
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return 0;
    }
    
    db_free(db);
    if (!db_init(db)) {
        fclose(file);
        return 0;
    }
    
    while (1) {
        read_count = fscanf(file, "%49[^\n]\n%99[^\n]\n%99[^\n]\n%d\n%d %d %d\n%d\n%49[^\n]\n",
            dir_str, current.site, current.name, &current.size,
            &current.release_date.day, &current.release_date.month, 
            &current.release_date.year, &current.dependencies, compat_str);
        
        if (read_count != 9) {
            break;
        }
        
        line_number++;
        
        if (!string_to_direction(dir_str, &current.direction)) {
            fprintf(stderr, "Ошибка в записи %d: некорректное направление\n", line_number);
            fclose(file);
            db_free(db);
            return 0;
        }
        
        if (!string_to_compatibility(compat_str, &current.compatibility)) {
            fprintf(stderr, "Ошибка в записи %d: некорректная совместимость\n", line_number);
            fclose(file);
            db_free(db);
            return 0;
        }
        
        if (current.size <= 0) {
            fprintf(stderr, "Ошибка в записи %d: размер должен быть > 0\n", line_number);
            fclose(file);
            db_free(db);
            return 0;
        }
        
        if (current.dependencies < 0) {
            fprintf(stderr, "Ошибка в записи %d: зависимости должны быть >= 0\n", line_number);
            fclose(file);
            db_free(db);
            return 0;
        }
        
        if (!validate_date(current.release_date)) {
            fprintf(stderr, "Ошибка в записи %d: некорректная дата\n", line_number);
            fclose(file);
            db_free(db);
            return 0;
        }
        
        if (!db_add_record(db, &current)) {
            fclose(file);
            db_free(db);
            return 0;
        }
    }
    
    fclose(file);
    
    if (db->count == 0) {
        fprintf(stderr, "Файл пуст или имеет неверный формат\n");
        db_free(db);
        return 0;
    }
    
    return 1;
}

int db_save_to_file(RepositoryDB* db, const char* filename)
{
    FILE* file;
    int i;
    
    if (db == NULL || filename == NULL) {
        fprintf(stderr, "Ошибка: некорректные параметры в db_save_to_file\n");
        return 0;
    }
    
    if (db->count == 0) {
        fprintf(stderr, "Нет данных для сохранения\n");
        return 0;
    }
    
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Ошибка создания файла");
        return 0;
    }
    
    for (i = 0; i < db->count; i++) {
        fprintf(file, "%s\n%s\n%s\n%d\n%d %d %d\n%d\n%s\n",
            direction_to_string(db->records[i].direction),
            db->records[i].site,
            db->records[i].name,
            db->records[i].size,
            db->records[i].release_date.day,
            db->records[i].release_date.month,
            db->records[i].release_date.year,
            db->records[i].dependencies,
            compatibility_to_string(db->records[i].compatibility));
    }
    
    fclose(file);
    return 1;
}

int db_add_record(RepositoryDB* db, Repository* record)
{
    if (db == NULL || record == NULL) {
        fprintf(stderr, "Ошибка: некорректные параметры в db_add_record\n");
        return 0;
    }
    
    if (db->count >= db->capacity) {
        if (!db_grow_capacity(db)) {
            return 0;
        }
    }
    
    db->records[db->count] = *record;
    db->count++;
    return 1;
}

int search_result_free(SearchResult* result)
{
    if (result == NULL) {
        return 0;
    }
    
    if (result->indices != NULL) {
        free(result->indices);
        result->indices = NULL;
    }
    result->count = 0;
    return 1;
}

SearchResult db_search_by_direction(RepositoryDB* db, Direction direction)
{
    SearchResult result = { NULL, 0 };
    int i;
    int capacity = INITIAL_CAPACITY;
    int* temp;
    
    if (db == NULL || db->count == 0) {
        return result;
    }
    
    result.indices = (int*)malloc(capacity * sizeof(int));
    if (result.indices == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для результата поиска\n");
        return result;
    }
    
    for (i = 0; i < db->count; i++) {
        if (db->records[i].direction == direction) {
            if (result.count >= capacity) {
                capacity *= 2;
                temp = (int*)realloc(result.indices, capacity * sizeof(int));
                if (temp == NULL) {
                    fprintf(stderr, "Ошибка расширения результата поиска\n");
                    free(result.indices);
                    result.indices = NULL;
                    result.count = 0;
                    return result;
                }
                result.indices = temp;
            }
            result.indices[result.count] = i;
            result.count++;
        }
    }
    
    return result;
}

/* Комбинированный поиск: дата релиза == target_date И размер == target_size */
SearchResult db_search_combined(RepositoryDB* db, Date target_date, int target_size)
{
    SearchResult result = { NULL, 0 };
    int i;
    int capacity = INITIAL_CAPACITY;
    int* temp;
    
    if (db == NULL || db->count == 0) {
        return result;
    }
    
    result.indices = (int*)malloc(capacity * sizeof(int));
    if (result.indices == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для результата поиска\n");
        return result;
    }
    
    for (i = 0; i < db->count; i++) {
        if (compare_dates(db->records[i].release_date, target_date) == 0 &&
            db->records[i].size == target_size) {
            if (result.count >= capacity) {
                capacity *= 2;
                temp = (int*)realloc(result.indices, capacity * sizeof(int));
                if (temp == NULL) {
                    fprintf(stderr, "Ошибка расширения результата поиска\n");
                    free(result.indices);
                    result.indices = NULL;
                    result.count = 0;
                    return result;
                }
                result.indices = temp;
            }
            result.indices[result.count] = i;
            result.count++;
        }
    }
    
    return result;
}

static int compare_records(Repository* a, Repository* b)
{
    int cmp_name;
    int cmp_date;
    
    cmp_name = strcmp(a->name, b->name);
    if (cmp_name != 0) {
        return cmp_name;
    }
    
    if (a->direction != b->direction) {
        return (int)a->direction - (int)b->direction;
    }
    
    cmp_date = compare_dates(a->release_date, b->release_date);
    return -cmp_date;
}

/* Пузырьковая сортировка: Название (возр.) -> Направление (возр.) -> Дата релиза (убыв.) */
int db_sort_bubble(RepositoryDB* db)
{
    int i, j;
    int swapped;
    Repository temp;
    
    if (db == NULL) {
        fprintf(stderr, "Ошибка: некорректный параметр в db_sort_bubble\n");
        return 0;
    }
    
    if (db->count < 2) {
        return 1;
    }
    
    for (i = 0; i < db->count - 1; i++) {
        swapped = 0;
        
        for (j = 0; j < db->count - 1 - i; j++) {
            if (compare_records(&db->records[j], &db->records[j + 1]) > 0) {
                temp = db->records[j];
                db->records[j] = db->records[j + 1];
                db->records[j + 1] = temp;
                swapped = 1;
            }
        }
        
        if (!swapped) {
            break;
        }
    }
    
    return 1;
}

int db_print_record(Repository* record, int index)
{
    if (record == NULL) {
        fprintf(stderr, "Ошибка: некорректный параметр в db_print_record\n");
        return 0;
    }
    
    printf("\n--- Запись %d ---\n", index);
    printf("Направление: %s\nСайт: %s\nНазвание: %s\nРазмер: %d Кб\n",
        direction_to_string(record->direction), record->site, record->name, record->size);
    printf("Дата релиза: %02d.%02d.%04d\nЗависимости: %d\nСовместимость: %s\n",
        record->release_date.day, record->release_date.month, record->release_date.year,
        record->dependencies, compatibility_to_string(record->compatibility));
    
    return 1;
}

int db_print_all(RepositoryDB* db)
{
    int i;
    
    if (db == NULL) {
        fprintf(stderr, "Ошибка: некорректный параметр в db_print_all\n");
        return 0;
    }
    
    if (db->count == 0) {
        printf("\nБаза данных пуста.\n");
        return 1;
    }
    
    printf("\n=== Список всех записей (%d) ===\n", db->count);
    
    for (i = 0; i < db->count; i++) {
        db_print_record(&db->records[i], i + 1);
    }
    
    return 1;
}

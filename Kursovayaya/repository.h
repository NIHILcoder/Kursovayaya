/**
 * @file repository.h
 * @brief База данных репозитория
 * @author Костопраев Дмитрий Евгеньевич
 */

#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <stdio.h>

#define MAX_STR 50
#define MAX_LONG_STR 100
#define INITIAL_CAPACITY 10
#define MAX_FILENAME 256

typedef enum {
    DIRECTION_BACKEND = 0,
    DIRECTION_FRONTEND,
    DIRECTION_MOBILE,
    DIRECTION_DEVOPS,
    DIRECTION_DATA_SCIENCE,
    DIRECTION_COUNT
} Direction;

typedef enum {
    COMPAT_WINDOWS = 0,
    COMPAT_LINUX,
    COMPAT_MACOS,
    COMPAT_CROSS_PLATFORM,
    COMPAT_COUNT
} Compatibility;

typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct {
    Direction direction;
    char site[MAX_LONG_STR];
    char name[MAX_LONG_STR];
    int size;
    Date release_date;
    int dependencies;
    Compatibility compatibility;
} Repository;

typedef struct {
    Repository* records;
    int count;
    int capacity;
} RepositoryDB;

typedef struct {
    int* indices;
    int count;
} SearchResult;

extern const char* dir_names[];
extern const char* compat_names[];

/* repository_db.c */
int db_init(RepositoryDB* db);
int db_free(RepositoryDB* db);
int db_load_from_file(RepositoryDB* db, const char* filename);
int db_save_to_file(RepositoryDB* db, const char* filename);
int db_add_record(RepositoryDB* db, Repository* record);
SearchResult db_search_by_direction(RepositoryDB* db, Direction direction);
SearchResult db_search_combined(RepositoryDB* db, Date target_date, int target_size);
int search_result_free(SearchResult* result);
int db_sort_bubble(RepositoryDB* db);
int db_print_record(Repository* record, int index);
int db_print_all(RepositoryDB* db);
const char* direction_to_string(Direction dir);
const char* compatibility_to_string(Compatibility compat);
int string_to_direction(const char* str, Direction* result);
int string_to_compatibility(const char* str, Compatibility* result);
int validate_date(Date date);
int compare_dates(Date d1, Date d2);

/* io.c */
int show_menu();
int read_int();
int read_string(char* buffer, int size);
Date read_date();
Direction read_direction();
Compatibility read_compatibility();
int read_repository_record(Repository* record);

#endif

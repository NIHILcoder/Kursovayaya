/**
 * @file main.c
 * @brief База данных репозитория
 * @author Костопраев Дмитрий Евгеньевич
 * @date --.--.----
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "repository.h"

static int handle_load(RepositoryDB* db)
{
    char filename[MAX_FILENAME];
    
    printf("Введите имя файла для загрузки: ");
    if (!read_string(filename, MAX_FILENAME)) {
        fprintf(stderr, "Ошибка чтения имени файла\n");
        return 0;
    }
    
    if (db_load_from_file(db, filename)) {
        printf("Данные успешно загружены (%d записей)\n", db->count);
        return 1;
    }
    
    return 0;
}

static int handle_search_direction(RepositoryDB* db)
{
    Direction search_direction;
    SearchResult result;
    int i;
    
    if (db->count == 0) {
        printf("\nБаза данных пуста\n");
        return 0;
    }
    
    printf("\n--- Поиск по направлению ---\n");
    search_direction = read_direction();
    
    result = db_search_by_direction(db, search_direction);
    
    printf("\n=== Результаты поиска ===\nНаправление: %s\n\n", direction_to_string(search_direction));
    
    if (result.count == 0) {
        printf("Записи не найдены\n");
    } else {
        for (i = 0; i < result.count; i++) {
            db_print_record(&db->records[result.indices[i]], result.indices[i] + 1);
        }
        printf("\nНайдено: %d\n", result.count);
    }
    
    search_result_free(&result);
    return 1;
}

static int handle_search_combined(RepositoryDB* db)
{
    Date search_date;
    int target_size;
    SearchResult result;
    int i;
    
    if (db->count == 0) {
        printf("\nБаза данных пуста\n");
        return 0;
    }
    
    printf("\n--- Комбинированный поиск (дата релиза И размер) ---\n");
    
    printf("Введите дату релиза:\n");
    search_date = read_date();
    
    printf("Размер (Кб): ");
    target_size = read_int();
    
    result = db_search_combined(db, search_date, target_size);
    
    printf("\n=== Результаты ===\nУсловия: дата = %02d.%02d.%04d И размер = %d Кб\n\n",
        search_date.day, search_date.month, search_date.year, target_size);
    
    if (result.count == 0) {
        printf("Записи не найдены\n");
    } else {
        for (i = 0; i < result.count; i++) {
            db_print_record(&db->records[result.indices[i]], result.indices[i] + 1);
        }
        printf("Найдено: %d\n", result.count);
    }
    
    search_result_free(&result);
    return 1;
}

static int handle_sort(RepositoryDB* db)
{
    if (db->count == 0) {
        printf("\nБаза данных пуста\n");
        return 0;
    }
    
    printf("\n--- Сортировка: Название -> Направление -> Дата релиза (убыв.) ---\n");
    
    if (db_sort_bubble(db)) {
        printf("Сортировка выполнена!\n\n");
        db_print_all(db);
        return 1;
    }
    
    return 0;
}

static int handle_add_record(RepositoryDB* db)
{
    Repository new_record;
    
    if (!read_repository_record(&new_record)) {
        fprintf(stderr, "Ошибка ввода записи\n");
        return 0;
    }
    
    if (db_add_record(db, &new_record)) {
        printf("\nЗапись добавлена!\n");
        return 1;
    }
    
    return 0;
}

static int handle_save(RepositoryDB* db)
{
    char filename[MAX_FILENAME];
    
    if (db->count == 0) {
        fprintf(stderr, "Нет данных для сохранения\n");
        return 0;
    }
    
    printf("Введите имя файла: ");
    if (!read_string(filename, MAX_FILENAME)) {
        fprintf(stderr, "Ошибка чтения имени файла\n");
        return 0;
    }
    
    if (db_save_to_file(db, filename)) {
        printf("Сохранено в '%s'\n", filename);
        return 1;
    }
    
    return 0;
}

int main()
{
    RepositoryDB db;
    int running = 1;
    int choice;
    
    setlocale(LC_ALL, "");
    
    printf("=== База данных репозитория ===\n\n");
    
    if (!db_init(&db)) {
        fprintf(stderr, "Критическая ошибка: не удалось инициализировать БД\n");
        return 1;
    }
    
    while (running) {
        choice = show_menu();
        
        switch (choice) {
            case 1:
                handle_load(&db);
                break;
                
            case 2:
                db_print_all(&db);
                break;
                
            case 3:
                handle_search_direction(&db);
                break;
                
            case 4:
                handle_search_combined(&db);
                break;
                
            case 5:
                handle_sort(&db);
                break;
                
            case 6:
                handle_add_record(&db);
                break;
                
            case 7:
                handle_save(&db);
                break;
                
            case 8:
                running = 0;
                printf("\nДо свидания!\n");
                break;
                
            default:
                printf("Неверный выбор\n");
                break;
        }
    }
    
    db_free(&db);
    
    return 0;
}

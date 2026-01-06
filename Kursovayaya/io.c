/**
 * @file io.c
 * @brief База данных репозитория - функции ввода-вывода
 * @author Костопраев Дмитрий Евгеньевич
 */

#include <stdio.h>
#include <string.h>
#include "repository.h"

int show_menu()
{
    printf("\n--- МЕНЮ ---\n");
    printf("1. Загрузить из файла\n2. Просмотреть записи\n3. Поиск по направлению\n");
    printf("4. Комбинированный поиск\n5. Сортировка\n6. Добавить запись\n7. Сохранить\n8. Выход\n");
    printf("Выбор (1-8): ");
    
    return read_int();
}

int read_int()
{
    int value;
    int result;
    
    result = scanf("%d", &value);
    
    while (result != 1) {
        while (getchar() != '\n');
        fprintf(stderr, "Ошибка! Число: ");
        result = scanf("%d", &value);
    }
    
    while (getchar() != '\n');
    
    return value;
}

int read_string(char* buffer, int size)
{
    if (buffer == NULL || size <= 0) {
        fprintf(stderr, "Ошибка: некорректные параметры в read_string\n");
        return 0;
    }
    
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        return 1;
    }
    
    return 0;
}

Date read_date()
{
    Date date;
    int valid = 0;
    
    while (!valid) {
        printf("День (1-31): ");
        date.day = read_int();
        
        printf("Месяц (1-12): ");
        date.month = read_int();
        
        printf("Год (1900-2100): ");
        date.year = read_int();
        
        if (validate_date(date)) {
            valid = 1;
        } else {
            fprintf(stderr, "Ошибка: некорректная дата\n");
        }
    }
    
    return date;
}

Direction read_direction()
{
    int choice;
    int i;
    
    printf("\nВыберите направление:\n");
    for (i = 0; i < DIRECTION_COUNT; i++) {
        printf("%d. %s\n", i + 1, dir_names[i]);
    }
    printf("Выбор (1-%d): ", DIRECTION_COUNT);
    
    choice = read_int();
    
    while (choice < 1 || choice > DIRECTION_COUNT) {
        fprintf(stderr, "Ошибка: 1-%d: ", DIRECTION_COUNT);
        choice = read_int();
    }
    
    return (Direction)(choice - 1);
}

Compatibility read_compatibility()
{
    int choice;
    int i;
    
    printf("\nВыберите совместимость:\n");
    for (i = 0; i < COMPAT_COUNT; i++) {
        printf("%d. %s\n", i + 1, compat_names[i]);
    }
    printf("Выбор (1-%d): ", COMPAT_COUNT);
    
    choice = read_int();
    
    while (choice < 1 || choice > COMPAT_COUNT) {
        fprintf(stderr, "Ошибка: 1-%d: ", COMPAT_COUNT);
        choice = read_int();
    }
    
    return (Compatibility)(choice - 1);
}

int read_repository_record(Repository* record)
{
    int value;
    
    if (record == NULL) {
        fprintf(stderr, "Ошибка: некорректный параметр в read_repository_record\n");
        return 0;
    }
    
    printf("\n--- Добавление записи ---\n");
    
    record->direction = read_direction();
    
    printf("\nСайт: ");
    if (!read_string(record->site, MAX_LONG_STR)) {
        fprintf(stderr, "Ошибка чтения сайта\n");
        return 0;
    }
    
    printf("Название: ");
    if (!read_string(record->name, MAX_LONG_STR)) {
        fprintf(stderr, "Ошибка чтения названия\n");
        return 0;
    }
    
    printf("Размер (Кб, > 0): ");
    value = read_int();
    while (value <= 0) {
        fprintf(stderr, "Ошибка: > 0\n");
        printf("Размер: ");
        value = read_int();
    }
    record->size = value;
    
    printf("\nДата релиза:\n");
    record->release_date = read_date();
    
    printf("\nЗависимости (>= 0): ");
    value = read_int();
    while (value < 0) {
        fprintf(stderr, "Ошибка: >= 0\n");
        printf("Зависимости: ");
        value = read_int();
    }
    record->dependencies = value;
    
    record->compatibility = read_compatibility();
    
    return 1;
}

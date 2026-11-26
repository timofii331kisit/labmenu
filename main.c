/*
    labmenu - системна консольна утиліта на основі лабораторних робіт №2-8.
    Варіант 10: відомості про пам'ять системи (GlobalMemoryStatusEx).

    Без аргументів - меню. З аргументами - виконання однієї лабораторної:
        labmenu lab3 [мінімум МБ]
    Код повернення програми дорівнює коду повернення лабораторної.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "labs.h"

typedef int (*LabFunc)(int, char *[]);

typedef struct {
    const char *name;       /* ім'я для командного рядка */
    const char *title;      /* назва в меню */
    const char *prompt;     /* що питати в користувача; NULL - параметрів немає */
    LabFunc     run;
} LabEntry;

 LAB_COUNT 7

static const LabEntry lab_table[LAB_COUNT] = {
    { "lab2", "ЛР2: сумарний розмір файлів у теці",           "Шлях до теки",                                   lab2_run },
    { "lab3", "ЛР3: завантаження пам'яті",                    "Мінімум вільної пам'яті, МБ (Enter = 1024)",     lab3_run },
    { "lab4", "ЛР4: вільні літери дисків (Z -> A)",           "Скільки літер знайти, 1-26 (Enter = 3)",         lab4_run },
    { "lab5", "ЛР5: найбільші файли каталогу",                "N і тека через пробіл (Enter = 5 і поточна)",    lab5_run },
    { "lab6", "ЛР6: файли каталогу за класами розміру",       "Маска (Enter = *)",                              lab6_run },
    { "lab7", "ЛР7: знімок процесів у CSV",                   "Скільки процесів показати, 1-50 (Enter = 10)",   lab7_run },
    { "lab8", "ЛР8: лічильник запусків під блокуванням",      "Затримка між читанням і записом, мс 0-10000 (Enter = 3000)", lab8_run },
};

static void print_menu(void)
{
    printf("\n===== labmenu =====\n");
    for (int i = 0; i < LAB_COUNT; i++) {
        printf("%d. %s\n", i + 1, lab_table[i].title);
    }
    printf("0. Вихід\n");
    printf("Ваш вибір: ");
}

/* Читає рядок із stdin без символу кінця рядка. FALSE - кінець введення. */
static BOOL read_line(char *buf, size_t size)
{
    if (fgets(buf, (int)size, stdin) == NULL) {
        return FALSE;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
    return TRUE;
}

/* Виконує пункт меню: питає параметри, будує argv, викликає лабораторну. */
static int run_from_menu(const LabEntry *lab)
{
    char line[512];
    char *argv[8];
    int argc = 0;
    argv[argc++] = (char *)lab->name;

    if (lab->prompt != NULL) {
        printf("%s: ", lab->prompt);
        if (!read_line(line, sizeof line)) {
            return 2;
        }
        /* розбір рядка на слова за пробілами (не більше 6 параметрів) */
        char *ctx = NULL;
        for (char *tok = strtok_s(line, " \t", &ctx); tok != NULL && argc < 7;
             tok = strtok_s(NULL, " \t", &ctx)) {
            argv[argc++] = tok;
        }
    }
    argv[argc] = NULL;

    printf("\n--- %s ---\n", lab->title);
    int rc = lab->run(argc, argv);
    printf("--- Код повернення: %d ---\n", rc);
    return rc;
}

int main(int argc, char *argv[])
{
    /* Українські літери: UTF-8 для виводу й введення (файл зібрано з /utf-8) */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    /* Режим з аргументами: labmenu labN <параметри> */
    if (argc > 1) {
        for (int i = 0; i < LAB_COUNT; i++) {
            if (_stricmp(argv[1], lab_table[i].name) == 0) {
                return lab_table[i].run(argc - 1, argv + 1);
            }
        }
        fprintf(stderr, "Невідома лабораторна \"%s\". Використання: labmenu [lab2..lab8] [параметри]\n", argv[1]);
        return 2;
    }

    /* Режим меню */
    char line[64];
    for (;;) {
        print_menu();
        if (!read_line(line, sizeof line)) {
            break;                                      /* кінець введення */
        }
        char *end = NULL;
        long choice = strtol(line, &end, 10);
        if (end == line || *end != '\0' || choice < 0 || choice > LAB_COUNT) {
            printf("Невірний вибір, введіть число від 0 до %d\n", LAB_COUNT);
            continue;
        }
        if (choice == 0) {
            break;
        }
        run_from_menu(&lab_table[choice - 1]);
    }
    return 0;
}

/*
    ЛР7, варіант 10. Знімок процесів у CSV.
    Програма записує таблицю процесів у processes.csv (ім'я;PID;потоки),
    читає її назад, сортує за кількістю потоків і друкує найбільші.
    Аргумент: скільки процесів показати, 1..50, типово 10.
    Звіт - на екран і у файл top.txt.

    Коди повернення:
        0 - норма
        1 - є процес з понад 100 потоками
        2 - неправильний аргумент
        3 - файл не відкрився або помилка функції WinAPI
*/

#define _CRT_SECURE_NO_WARNINGS
#ifdef UNICODE
#undef UNICODE                  /* потрібна ANSI-структура PROCESSENTRY32 із char szExeFile */
#endif
#ifdef _UNICODE
#undef _UNICODE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tlhelp32.h>

#define MAX_PROC 512    /* місткість масивів; на перевірочному ПК процесів було 511-513 */
#define NAME_LEN 64

static char  g_name[MAX_PROC][NAME_LEN];
static DWORD g_pid[MAX_PROC];
static DWORD g_thr[MAX_PROC];

/* Обмін рядків i і j у ВСІХ трьох масивах */
static void swap_rows(int i, int j)
{
    char tmp[NAME_LEN];
    DWORD t;
    strcpy(tmp, g_name[i]);
    strcpy(g_name[i], g_name[j]);
    strcpy(g_name[j], tmp);
    t = g_pid[i]; g_pid[i] = g_pid[j]; g_pid[j] = t;
    t = g_thr[i]; g_thr[i] = g_thr[j]; g_thr[j] = t;
}

/* Сортування за кількістю потоків за спаданням, бульбашка */
static void sort_by_threads(int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (g_thr[j] < g_thr[j + 1]) {
                swap_rows(j, j + 1);
            }
        }
    }
}

static unsigned long total_threads(int n)
{
    unsigned long s = 0;
    for (int i = 0; i < n; i++) {
        s += g_thr[i];
    }
    return s;
}

/* Номер рядка (з 1) першого процесу з заданим іменем; -1, якщо не знайдено */
static int find_row(int n, const char *name)
{
    for (int i = 0; i < n; i++) {
        if (_stricmp(g_name[i], name) == 0) {
            return i + 1;
        }
    }
    return -1;
}

/* Звіт: однаково пишеться і на екран (stdout), і у файл */
static void report(FILE *out, int n, int show, int svc_row)
{
    fprintf(out, "processes: %d, threads: %lu\n", n, total_threads(n));
    for (int i = 0; i < show && i < n; i++) {
        fprintf(out, "%-6lu %4lu  [%s]\n", g_pid[i], g_thr[i], g_name[i]);   /* ім'я - у дужках, щоб бачити межі */
    }
    fprintf(out, "svchost.exe row: %d\n", svc_row);
}

int lab7_run(int argc, char *argv[])
{
    /* 1. Аргумент */
    int show = 10;
    if (argc > 1) {
        char *end = NULL;
        long v = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || v < 1 || v > 50) {
            fprintf(stderr, "argument must be an integer in range 1..50\n");
            return 2;
        }
        show = (int)v;
    }

    /* 2. Дані від Windows -> processes.csv, режим "w" */
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateToolhelp32Snapshot failed, error %lu\n", GetLastError());
        return 3;
    }
    FILE *f = fopen("processes.csv", "w");
    if (f == NULL) {
        perror("processes.csv");
        CloseHandle(snap);
        return 3;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof pe;                              /* розмір - до першого виклику */
    if (!Process32First(snap, &pe)) {
        fprintf(stderr, "Process32First failed, error %lu\n", GetLastError());
        fclose(f);
        CloseHandle(snap);
        return 3;
    }
    do {
        fprintf(f, "%.63s;%lu;%lu\n", pe.szExeFile, pe.th32ProcessID, pe.cntThreads);
    } while (Process32Next(snap, &pe));
    fclose(f);
    CloseHandle(snap);

    /* 3. Файл -> масиви. Керування циклом - результат fscanf (має бути 3 поля).
          ПАСТКА: пробіл перед %63[^;] обов'язковий. Набір символів не пропускає '\n',
          що лишився після числа в попередньому рядку, тому без пробілу з другого запису
          кожне ім'я починається з '\n', а svchost.exe не знаходиться (-1). */
    f = fopen("processes.csv", "r");
    if (f == NULL) {
        perror("processes.csv");
        return 3;
    }
    int n = 0;
    int lost = 0;
    char name[NAME_LEN];
    DWORD pid, thr;
    while (fscanf(f, " %63[^;];%lu;%lu", name, &pid, &thr) == 3) {
        if (n >= MAX_PROC) {                            /* перевірка місткості ПЕРЕД записом */
            lost++;
            continue;
        }
        strcpy(g_name[n], name);
        g_pid[n] = pid;
        g_thr[n] = thr;
        n++;
    }
    fclose(f);
    if (lost > 0) {
        fprintf(stderr, "arrays are full (%d): %d processes did not fit\n", MAX_PROC, lost);
    }
    if (n == 0) {
        fprintf(stderr, "no processes were read\n");
        return 3;
    }

    /* 4. Обробка: сортування, пошук svchost.exe */
    sort_by_threads(n);
    int svc_row = find_row(n, "svchost.exe");

    /* 5. Звіт на екран і у файл top.txt */
    report(stdout, n, show, svc_row);
    FILE *out = fopen("top.txt", "w");
    if (out == NULL) {
        perror("top.txt");
        return 3;
    }
    report(out, n, show, svc_row);
    fclose(out);

    /* 6. Код повернення: після сортування g_thr[0] - найбільша кількість потоків */
    return g_thr[0] > 100 ? 1 : 0;
}

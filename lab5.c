/*
    ЛР5, варіант 10. N найбільших файлів каталогу, середній і медіанний розмір.
    Аргументи: N (1..CAP, типово 5) і тека (типово поточна).

    Коди повернення:
        0 - норма
        1 - файлів менше, ніж N (або файлів немає)
        2 - неправильний аргумент
        3 - помилка функції WinAPI
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define CAP 256     /* місткість масиву розмірів; для перевірки переповнення тимчасово поставте 3 */

/* Сортування за спаданням, бульбашка */
static void sort_desc(ULONGLONG a[], int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (a[j] < a[j + 1]) {
                ULONGLONG t = a[j];
                a[j] = a[j + 1];
                a[j + 1] = t;
            }
        }
    }
}

static ULONGLONG total_size(const ULONGLONG a[], int n)
{
    ULONGLONG s = 0;
    for (int i = 0; i < n; i++) {
        s += a[i];
    }
    return s;
}

/* Середнє; для n == 0 повертає 0 (без ділення на нуль) */
static ULONGLONG average(const ULONGLONG a[], int n)
{
    return n > 0 ? total_size(a, n) / (ULONGLONG)n : 0;
}

/* Медіана відсортованого масиву */
static ULONGLONG median(const ULONGLONG a[], int n)
{
    if (n == 0) {
        return 0;
    }
    if (n % 2 == 1) {
        return a[n / 2];
    }
    return (a[n / 2 - 1] + a[n / 2]) / 2;
}

static ULONGLONG file_size(const WIN32_FIND_DATAA *fd)
{
    return ((ULONGLONG)fd->nFileSizeHigh << 32) | fd->nFileSizeLow;
}

int lab5_run(int argc, char *argv[])
{
    /* 1. Аргументи */
    int top = 5;
    if (argc > 1) {
        char *end = NULL;
        long v = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || v < 1 || v > CAP) {
            fprintf(stderr, "N must be an integer in range 1..%d\n", CAP);
            return 2;
        }
        top = (int)v;
    }
    const char *dir = (argc > 2) ? argv[2] : ".";
    if (strlen(dir) > MAX_PATH - 4) {
        fprintf(stderr, "path is too long\n");
        return 2;
    }
    char pattern[MAX_PATH];
    sprintf_s(pattern, sizeof pattern, "%s\\*", dir);

    /* 2. Заповнення масиву розмірів: не більше CAP елементів */
    ULONGLONG size[CAP];
    int n = 0;
    int skipped = 0;
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "FindFirstFileA failed, error %lu\n", GetLastError());
        return 3;
    }
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        if (n >= CAP) {                                 /* перевірка місткості ПЕРЕД записом */
            skipped++;
            continue;
        }
        size[n++] = file_size(&fd);
    } while (FindNextFileA(h, &fd));
    DWORD err = GetLastError();
    FindClose(h);
    if (err != ERROR_NO_MORE_FILES) {
        fprintf(stderr, "FindNextFileA failed, error %lu\n", err);
        return 3;
    }
    if (skipped > 0) {
        fprintf(stderr, "array is full (%d): %d files were not counted\n", CAP, skipped);
    }

    /* 3. Порожній каталог обробляється окремо, щоб не ділити на нуль */
    if (n == 0) {
        printf("no files in %s\n", dir);
        return 1;
    }

    /* 4. Обробка */
    sort_desc(size, n);
    int shown = top < n ? top : n;
    printf("files: %d, total %llu bytes\n", n, total_size(size, n));
    printf("average %llu bytes, median %llu bytes\n", average(size, n), median(size, n));

    /* Імена N найбільших - другим переглядом: файли, розмір яких не менший за size[shown - 1].
       ПАСТКА: якщо на межі кілька файлів однакового розміру, рядків буде більше за N. */
    ULONGLONG threshold = size[shown - 1];
    printf("largest %d:\n", shown);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "FindFirstFileA failed, error %lu\n", GetLastError());
        return 3;
    }
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && file_size(&fd) >= threshold) {
            printf("%12llu  %s\n", file_size(&fd), fd.cFileName);
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);

    /* 5. Код повернення */
    return n < top ? 1 : 0;
}

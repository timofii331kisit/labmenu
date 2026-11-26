/*
    ЛР6, варіант 10. Файли каталогу за класами розміру.
    Аргумент: маска файлів (типово *), 1..100 символів.

    Коди повернення:
        0 - норма
        1 - файлів немає
        2 - неправильний аргумент
        3 - помилка функції WinAPI
*/

#include <stdio.h>
#include <string.h>
#include <windows.h>

#define CLASSES 6   /* < 1 КБ, < 10 КБ, < 100 КБ, < 1 МБ, < 10 МБ, решта */

static const char *const class_name[CLASSES] = {
    "< 1 KB", "< 10 KB", "< 100 KB", "< 1 MB", "< 10 MB", ">= 10 MB"
};

/* Сума стовпця c таблиці t */
static ULONGLONG column_sum(ULONGLONG t[][2], int c)
{
    ULONGLONG s = 0;
    for (int k = 0; k < CLASSES; k++) {
        s += t[k][c];
    }
    return s;
}

/* Номер рядка з найбільшою кількістю файлів */
static int busiest_row(ULONGLONG t[][2])
{
    int best = 0;
    for (int k = 1; k < CLASSES; k++) {
        if (t[k][0] > t[best][0]) {
            best = k;
        }
    }
    return best;
}

int lab6_run(int argc, char *argv[])
{
    /* 1. Аргумент: маска; перевірка довжини */
    const char *mask = (argc > 1) ? argv[1] : "*";
    size_t len = strlen(mask);
    if (len < 1 || len > 100) {
        fprintf(stderr, "mask must be 1..100 characters\n");
        return 2;
    }

    static const ULONGLONG lim[CLASSES - 1] = { 1024, 10240, 102400, 1048576, 10485760 };
    ULONGLONG t[CLASSES][2] = { { 0 } };    /* стовпець 0 - кількість файлів, 1 - сумарний розмір, КБ */

    /* 2. Перебір файлів */
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(mask, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        DWORD e = GetLastError();
        if (e == ERROR_FILE_NOT_FOUND) {
            printf("no files match \"%s\"\n", mask);
            return 1;
        }
        fprintf(stderr, "FindFirstFileA failed, error %lu\n", e);
        return 3;
    }
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        ULONGLONG size = ((ULONGLONG)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
        int k = 0;
        /* ПАСТКА: без умови k < CLASSES - 1 файл від 10 МБ виводить k за межі масиву lim
           і запис t[k] пошкоджує пам'ять (у Debug - повідомлення /RTC про масив t). */
        while (k < CLASSES - 1 && size >= lim[k]) {
            k++;
        }
        t[k][0]++;
        t[k][1] += size / 1024;
    } while (FindNextFileA(h, &fd));
    DWORD err = GetLastError();
    FindClose(h);
    if (err != ERROR_NO_MORE_FILES) {
        fprintf(stderr, "FindNextFileA failed, error %lu\n", err);
        return 3;
    }

    /* 3. Таблиця: частка і середній розмір; порожній клас - без ділення на нуль */
    ULONGLONG files = column_sum(t, 0);
    ULONGLONG kb = column_sum(t, 1);
    printf("%-10s %6s %9s %10s %10s\n", "class", "files", "share %", "total KB", "avg KB");
    for (int k = 0; k < CLASSES; k++) {
        double share = files > 0 ? 100.0 * (double)t[k][0] / (double)files : 0.0;
        double avg = t[k][0] > 0 ? (double)t[k][1] / (double)t[k][0] : 0.0;
        printf("%-10s %6llu %9.1f %10llu %10.1f\n", class_name[k], t[k][0], share, t[k][1], avg);
    }
    printf("total: %llu files, %llu KB\n", files, kb);
    if (files == 0) {
        return 1;
    }
    printf("busiest class: %s (%llu files)\n", class_name[busiest_row(t)], t[busiest_row(t)][0]);
    return 0;
}

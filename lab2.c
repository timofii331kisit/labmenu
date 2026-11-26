/*
    ЛР2, варіант 10. Сумарний розмір файлів у теці.
    Аргумент: шлях до теки. Підтеки не обходяться (свідоме спрощення).

    Коди повернення:
        0 - норма
        1 - у теці немає жодного файлу
        2 - немає аргументу або шлях задовгий
        3 - помилка функції WinAPI (теку не знайдено, відмова в доступі)
*/

#include <stdio.h>
#include <string.h>
#include <windows.h>

int lab2_run(int argc, char *argv[])
{
    if (argc < 2 || argv[1][0] == '\0') {
        fprintf(stderr, "usage: lab2 <folder>\n");
        return 2;
    }
    if (strlen(argv[1]) > MAX_PATH - 4) {
        fprintf(stderr, "path is too long\n");
        return 2;
    }

    /* шлях + маска: якщо шлях уже закінчується на \ або /, роздільник не додається */
    char pattern[MAX_PATH];
    size_t len = strlen(argv[1]);
    char last = argv[1][len - 1];
    const char *sep = (last == '\\' || last == '/') ? "" : "\\";
    sprintf_s(pattern, sizeof pattern, "%s%s*", argv[1], sep);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "FindFirstFileA failed, error %lu\n", GetLastError());
        return 3;
    }

    ULARGE_INTEGER total;
    total.QuadPart = 0;                                 /* 64-бітний акумулятор */
    int files = 0;

    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;                                   /* каталоги (., .., підтеки) пропускаємо */
        }
        ULARGE_INTEGER size;
        size.LowPart  = fd.nFileSizeLow;                /* розмір файлу - у двох 32-бітних половинах */
        size.HighPart = fd.nFileSizeHigh;
        total.QuadPart += size.QuadPart;
        files++;
    } while (FindNextFileA(hFind, &fd));

    DWORD err = GetLastError();                         /* причина завершення циклу */
    FindClose(hFind);
    if (err != ERROR_NO_MORE_FILES) {
        fprintf(stderr, "FindNextFileA failed, error %lu\n", err);
        return 3;
    }

    printf("%llu bytes in %d files\n", total.QuadPart, files);
    return files > 0 ? 0 : 1;
}

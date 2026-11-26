/*
    ЛР4, варіант 10. Вільні літери дисків, перебір від Z: до A:.
    Аргумент: скільки вільних літер знайти, 1..26, типово 3.

    Коди повернення:
        0 - норма (знайдено потрібну кількість)
        1 - знайдено менше літер, ніж потрібно
        2 - неправильний аргумент
        3 - GetLogicalDrives повернула 0
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int lab4_run(int argc, char *argv[])
{
    /* 1. Аргумент */
    int need = 3;
    if (argc > 1) {
        char *end = NULL;
        long v = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || v < 1 || v > 26) {
            fprintf(stderr, "argument must be an integer in range 1..26\n");
            return 2;
        }
        need = (int)v;
    }

    /* 2. Маска зайнятих літер: біт i = 1, якщо літера 'A' + i зайнята */
    DWORD mask = GetLogicalDrives();
    if (mask == 0) {
        fprintf(stderr, "GetLogicalDrives failed, error %lu\n", GetLastError());
        return 3;
    }

    /* 3. Цикл від Z (біт 25) до A (біт 0).
          ПАСТКА: лічильник знакового типу int. Цикл for (DWORD i = 25; i >= 0; i--)
          не завершується: умова i >= 0 завжди істинна, а після 0 значення стає 4294967295. */
    int found = 0;
    int checked = 0;
    int stopped_by_break = 0;
    for (int i = 25; i >= 0; i--) {
        checked++;
        if (mask & (1u << i)) {
            continue;                                   /* літера зайнята */
        }
        printf("free: %c:\n", 'A' + i);
        found++;
        if (found == need) {
            stopped_by_break = 1;                       /* знайдено досить - достроковий вихід */
            break;
        }
    }

    /* 4. Підсумок і причина завершення циклу */
    printf("checked %d letters, found %d of %d\n", checked, found, need);
    printf("loop ended: %s\n", stopped_by_break ? "break (enough letters found)"
                                                : "condition (all letters checked)");

    /* 5. Код повернення */
    return found < need ? 1 : 0;
}

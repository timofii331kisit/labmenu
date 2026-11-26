/*
    ЛР3, варіант 10. Завантаження пам'яті (GlobalMemoryStatusEx).
    Аргумент: мінімум вільної фізичної пам'яті в МБ, 0..1048576, типово 1024.

    Коди повернення:
        0 - норма
        1 - вільної пам'яті менше за мінімум
        2 - неправильний аргумент
        3 - GlobalMemoryStatusEx повернула FALSE
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int lab3_run(int argc, char *argv[])
{
    /* 1. Аргумент: спочатку знаковий long long, перевірка діапазону, потім беззнаковий тип.
          Від'ємне число не повинно стати величезним беззнаковим. */
    long long arg = 1024;
    if (argc > 1) {
        char *end = NULL;
        arg = strtoll(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0') {
            fprintf(stderr, "argument must be an integer\n");
            return 2;
        }
    }
    if (arg < 0 || arg > 1048576) {
        fprintf(stderr, "argument must be in range 0..1048576 MB\n");
        return 2;
    }
    DWORDLONG limitMB = (DWORDLONG)arg;

    /* 2. Виклик WinAPI. dwLength заповнюється ДО виклику, інакше помилка 87. */
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    if (!GlobalMemoryStatusEx(&ms)) {
        fprintf(stderr, "GlobalMemoryStatusEx failed, error %lu\n", GetLastError());
        return 3;
    }

    /* 3. Лінійна частина: байти -> мегабайти; зайнятий обсяг.
          Віднімання безпечне, бо вільної пам'яті не може бути більше, ніж усієї. */
    DWORDLONG totalMB = ms.ullTotalPhys / (1024 * 1024);
    DWORDLONG availMB = ms.ullAvailPhys / (1024 * 1024);
    DWORDLONG usedMB  = totalMB - availMB;
    printf("total %llu MB, free %llu MB, used %llu MB\n", totalMB, availMB, usedMB);
    printf("load  %lu %%\n", ms.dwMemoryLoad);

    /* 4. if - else if за діапазонами завантаження */
    if (ms.dwMemoryLoad < 50) {
        printf("range: below 50 %%\n");
    } else if (ms.dwMemoryLoad < 75) {
        printf("range: 50..74 %%\n");
    } else if (ms.dwMemoryLoad < 90) {
        printf("range: 75..89 %%\n");
    } else {
        printf("range: 90 %% and above\n");
    }

    /* 5. switch за чвертю завантаження (0..4); case 3 і case 4 мають однакову пораду */
    switch (ms.dwMemoryLoad / 25) {
    case 0:
        printf("advice: plenty of memory\n");
        break;
    case 1:
        printf("advice: normal\n");
        break;
    case 2:
        printf("advice: close unused programs\n");
        break;
    case 3:
    case 4:
        printf("advice: memory is almost exhausted, close programs now\n");
        break;
    default:
        printf("advice: unexpected value\n");
        break;
    }

    /* 6. Порівняння з мінімумом.
          ПАСТКА: вираз availMB - limitMB < 0 для беззнакових чисел завжди хибний
          (різниця "обертається" у величезне число), тому порівнюємо напряму. */
    if (availMB < limitMB) {
        printf("free memory %llu MB is below the minimum %llu MB\n", availMB, limitMB);
        return 1;
    }
    printf("free memory %llu MB is not below the minimum %llu MB\n", availMB, limitMB);
    return 0;
}

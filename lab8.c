/*
    ЛР8, варіант 10. Лічильник запусків під блокуванням.
    Програма збільшує лічильник у counter.bin на 1. Дві копії, запущені одночасно,
    не втрачають жодного запуску: ділянку файлу блокує LockFileEx.
    Аргумент: затримка між читанням і записом, мс, 0..10000, типово 3000.

    Коди повернення:
        0 - норма
        1 - довелося чекати блокування понад 100 мс
        2 - неправильний аргумент
        3 - помилка файлу або WinAPI
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int lab8_run(int argc, char *argv[])
{
    /* 1. Аргумент: затримка, мс */
    long delay = 3000;
    if (argc > 1) {
        char *end = NULL;
        delay = strtol(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0' || delay < 0 || delay > 10000) {
            fprintf(stderr, "delay must be an integer in range 0..10000 ms\n");
            return 2;
        }
    }

    /* 2. Відкриття з повним спільним доступом: інакше друга копія отримала б помилку 32.
          OPEN_ALWAYS: файл створюється, якщо його ще немає; якщо файл уже є, виклик теж
          успішний, а GetLastError повертає 183 (ERROR_ALREADY_EXISTS) - це норма, не помилка. */
    HANDLE h = CreateFileA("counter.bin", GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                           OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateFileA failed, error %lu\n", GetLastError());
        return 3;
    }

    /* 3. Блокування ділянки 0..3. Без LOCKFILE_FAIL_IMMEDIATELY виклик ЧЕКАЄ, доки
          першу копію не розблокують. Скільки чекали - різниця двох GetTickCount64. */
    OVERLAPPED ov = { 0 };                              /* Offset = 0: ділянка з початку файлу */
    ULONGLONG t0 = GetTickCount64();
    if (!LockFileEx(h, LOCKFILE_EXCLUSIVE_LOCK, 0, sizeof(int), 0, &ov)) {
        fprintf(stderr, "LockFileEx failed, error %lu\n", GetLastError());
        CloseHandle(h);
        return 3;
    }
    ULONGLONG waited = GetTickCount64() - t0;

    /* 4. Читання лічильника. Новий файл: ReadFile успішна, але got == 0 -> лічильник 0.
          ПАСТКА: без блокування обидві копії прочитали б те саме X і обидві записали б X + 1:
          один запуск втрачено, і жодна копія про це не дізнається. */
    LARGE_INTEGER zero;
    zero.QuadPart = 0;
    int n = 0;
    DWORD got = 0;
    DWORD put = 0;
    int rc = 0;

    if (!SetFilePointerEx(h, zero, NULL, FILE_BEGIN)) {
        fprintf(stderr, "SetFilePointerEx failed, error %lu\n", GetLastError());
        rc = 3;
    } else if (!ReadFile(h, &n, sizeof n, &got, NULL)) {
        fprintf(stderr, "ReadFile failed, error %lu\n", GetLastError());
        rc = 3;
    } else {
        if (got != sizeof n) {
            n = 0;                                      /* файл порожній (або обірваний): починаємо з 0 */
        }
        printf("locked after %llu ms, read %d\n", waited, n);

        /* 5. Затримка: час, щоб запустити другу копію */
        Sleep((DWORD)delay);
        n++;

        /* 6. Запис. Після ReadFile позиція = 4, тому знову ставимо її на початок. */
        if (!SetFilePointerEx(h, zero, NULL, FILE_BEGIN)) {
            fprintf(stderr, "SetFilePointerEx failed, error %lu\n", GetLastError());
            rc = 3;
        } else if (!WriteFile(h, &n, sizeof n, &put, NULL) || put != sizeof n) {
            fprintf(stderr, "WriteFile failed, error %lu\n", GetLastError());
            rc = 3;
        } else {
            printf("written %d\n", n);
        }
    }

    /* 7. Розблокування і закриття (виконується і після помилки) */
    if (!UnlockFileEx(h, 0, sizeof(int), 0, &ov)) {
        fprintf(stderr, "UnlockFileEx failed, error %lu\n", GetLastError());
        rc = 3;
    }
    CloseHandle(h);

    if (rc != 0) {
        return rc;
    }
    return waited > 100 ? 1 : 0;
}

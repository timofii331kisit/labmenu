/*
    labs.h - оголошення точок входу всіх лабораторних робіт labmenu.
    Кожна функція labN_run має сигнатуру main: приймає argc/argv,
    argv[0] - ім'я лабораторної, далі - параметри задачі.
    Повертає код повернення лабораторної (0 - норма).
*/
#ifndef LABS_H
#define LABS_H

int lab2_run(int argc, char *argv[]);   /* ЛР2: сумарний розмір файлів у теці        */
int lab3_run(int argc, char *argv[]);   /* ЛР3: завантаження пам'яті                 */
int lab4_run(int argc, char *argv[]);   /* ЛР4: вільні літери дисків (Z -> A)        */
int lab5_run(int argc, char *argv[]);   /* ЛР5: найбільші файли каталогу             */
int lab6_run(int argc, char *argv[]);   /* ЛР6: файли каталогу за класами розміру    */
int lab7_run(int argc, char *argv[]);   /* ЛР7: знімок процесів у CSV                */
int lab8_run(int argc, char *argv[]);   /* ЛР8: лічильник запусків під блокуванням   */

#endif /* LABS_H */

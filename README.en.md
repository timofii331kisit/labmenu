# labmenu

**Languages:** [Русский](README.md) | English | [Українська](README.uk.md)

A Windows console utility written in C that combines lab assignments 2–8 (variant 10) into a single program. Each lab talks to the system through the WinAPI: files and folders, memory, drives, processes, and file locking.

It runs with no arguments (interactive menu) or with arguments (a single lab, handy for scripts). The program's exit code equals the exit code of the selected lab.

## Features

| Command | What it does |
|---------|--------------|
| `lab2` | Total size of the files in a folder (subfolders are not traversed) |
| `lab3` | RAM load (`GlobalMemoryStatusEx`): total / free / used, load percentage, advice, comparison with a given minimum |
| `lab4` | Free drive letters, scanned from `Z:` down to `A:` |
| `lab5` | The N largest files in a folder, plus average and median size |
| `lab6` | Files grouped by size class (< 1 KB, < 10 KB, < 100 KB, < 1 MB, < 10 MB, ≥ 10 MB): count, share, total and average size |
| `lab7` | Process snapshot: written to `processes.csv`, read back, sorted by thread count, report printed and saved to `top.txt` |
| `lab8` | Launch counter in `counter.bin` protected by `LockFileEx`: two copies started at the same time never lose a launch |

## Requirements

- Windows 7 or later (WinAPI, `GetTickCount64`, `LockFileEx`).
- To build: Visual Studio (project `labmenu.slnx` / `labmenu.vcxproj`) or MinGW-w64.
- Source files are UTF-8. The menu messages are in Ukrainian.

## Building

**Visual Studio.** Open `labmenu.slnx`, pick a configuration (Debug/Release, x64/x86), and build. The project targets the `v145` toolset; if it is not installed, switch to one you have (for example `v143`) in the project properties. Language standard: C17.

**MinGW-w64** (for example, cross-compiling from Linux):

```bash
x86_64-w64-mingw32-gcc -std=c17 -O2 -static \
    main.c lab2.c lab3.c lab4.c lab5.c lab6.c lab7.c lab8.c \
    -o labmenu.exe -lpsapi
```

## Usage

### Menu mode

Run `labmenu.exe` with no arguments, choose an item from 1 to 7 (0 exits), and enter the parameters. Empty input means the default value.

```
===== labmenu =====
1. ЛР2: сумарний розмір файлів у теці
2. ЛР3: завантаження пам'яті
...
0. Вихід
```

> In the menu, parameters are split on spaces, so a path containing spaces cannot be entered as a whole. For such paths, use command-line mode with quotes.

### Command-line mode

```
labmenu.exe <labN> [parameters]
```

| Command | Parameters | Default | Example |
|---------|------------|---------|---------|
| `lab2` | folder path | required | `labmenu.exe lab2 "C:\Users\me\Documents"` |
| `lab3` | minimum free memory, MB (0–1048576) | 1024 | `labmenu.exe lab3 2048` |
| `lab4` | how many free letters to find (1–26) | 3 | `labmenu.exe lab4 5` |
| `lab5` | N (1–256) and folder | 5 and the current folder | `labmenu.exe lab5 10 "C:\Windows"` |
| `lab6` | file mask (1–100 characters) | `*` | `labmenu.exe lab6 "C:\Windows\*.exe"` |
| `lab7` | how many processes to show (1–50) | 10 | `labmenu.exe lab7 20` |
| `lab8` | delay between read and write, ms (0–10000) | 3000 | `labmenu.exe lab8 5000` |

## Exit codes

Common to all labs: `0` means success, `2` means an invalid argument, `3` means a file or WinAPI error. Code `1` is specific to each lab:

| Command | Meaning of code `1` |
|---------|---------------------|
| `lab2` | the folder contains no files |
| `lab3` | free memory is below the given minimum |
| `lab4` | fewer letters were found than requested |
| `lab5` | fewer files than N (or no files at all) |
| `lab6` | no files match the mask |
| `lab7` | some process has more than 100 threads |
| `lab8` | waiting for the lock took longer than 100 ms |

For `lab4`, code `3` means `GetLogicalDrives` returned 0. In command-line mode you can check the code with `echo %ERRORLEVEL%`.

## Files created

The labs write files to the **current folder**:

- `processes.csv` and `top.txt` by `lab7`;
- `counter.bin` by `lab8` (4 bytes, the counter value).

## Testing lab8 (file locking)

Open two console windows in the same folder and start this in both, almost at the same time:

```
labmenu.exe lab8 5000
```

The first copy reads `0`, waits, and writes `1`. The second one waits for the lock to be released, reads `1`, and writes `2`. Without locking, both would write `1` and one launch would be lost.

## Project layout

```
labmenu/
├── main.c            menu and command-line parsing
├── labs.h            declarations of lab2_run … lab8_run
├── lab2.c … lab8.c   the lab assignments
├── labmenu.vcxproj   Visual Studio project
└── labmenu.slnx      Visual Studio solution
```

Each lab is a function `labN_run(int argc, char *argv[])` with the same signature as `main`: `argv[0]` is the lab name, followed by the parameters.

## Known limitations

- `lab2` does not traverse subfolders (a deliberate simplification).
- `lab5` counts at most 256 files per folder; extra files are skipped with a warning. If several files of equal size tie at the top-N boundary, more than N lines are printed.
- `lab7` handles at most 512 processes.
- Paths longer than `MAX_PATH` (260 characters) are not supported.

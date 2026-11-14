# 🧬 Stereocode

[![Tests](https://github.com/srcML/stereocode/actions/workflows/tests.yml/badge.svg)](https://github.com/srcML/stereocode/actions/workflows/tests.yml) ![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus) [![Release](https://img.shields.io/github/v/release/srcML/stereocode)](https://github.com/srcML/stereocode/releases/latest) [![License](https://img.shields.io/github/license/srcML/stereocode)](LICENSE)

**Stereocode** classifies methods, free functions, and types (e.g., classes) of **C, C++, C#, Java, Python, JavaScript, and TypeScript** code by their **stereotypes**.

A stereotype is a short label for the role of a code element (e.g., a method or a class), such as `get` for a method that returns a field, `command` for a method that changes the state of the object, or `entity` for a class that keeps its data together with the methods that work on the data.

> [!TIP]
> The [wiki](https://github.com/srcML/stereocode/wiki) defines every method, class, and free-function stereotype with its rules and examples.
>
> [Developer Notes](https://github.com/srcML/stereocode/wiki/Developer-Notes) lists the developers, the history, and the citation of Stereocode.

## 📦 Requirements

- **srcML 1.2** with its development package ([srcml.org](https://www.srcml.org/))
- **libxml2** with its development package (`libxml2-dev`)
- **CMake 3.20** or newer and a **C++17** compiler (Clang, GCC, or MSVC)

## 🔧 Installation

```bash
git clone https://github.com/srcML/stereocode.git  # download the source code
cd stereocode                                      # go into the source folder
cmake -B build -DCMAKE_BUILD_TYPE=Release          # configure a release build
cmake --build build                                # compile build/stereocode
cmake --install build                              # copy stereocode to the system (e.g., /usr/local/bin)
```

## 🐳 Docker

The image [`mightyali98/stereocode`](https://hub.docker.com/r/mightyali98/stereocode) on Docker Hub holds srcML and Stereocode for Intel and ARM machines, ready to run without the requirements or the build. GitHub Actions rebuilds the image with each release of Stereocode and every week with the newest srcML release and Ubuntu packages.

```bash
docker pull mightyali98/stereocode                                  # get or update the image
alias run='docker run --rm -v "$PWD:/data" mightyali98/stereocode'  # run commands in the image
```

The commands of the usage example then run in the image, on the files of the current folder.

```bash
run srcml --register-ext h=C++ my_project -o my_project.xml  # turn the code into a srcML archive
run stereocode my_project.xml -f -r                          # add stereotypes and write CSV reports
```

The tags `latest` and `2.0.0` follow the weekly builds. A tag such as `2.0.0-srcml1.2.0` keeps the first build of one release of Stereocode with one release of srcML.

## 🚀 Usage

Stereocode reads a [srcML](https://www.srcml.org/) archive and annotates it with stereotype information as `st:stereotype` attributes.

For example, these two commands classify a C++ project in the folder `my_project`.

```bash
srcml --register-ext h=C++ my_project -o my_project.xml  # turn the code into a srcML archive
build/stereocode my_project.xml -f -r                    # add stereotypes and write CSV reports
```

In the output archive `my_project.stereotypes.xml`, code elements (e.g., classes and functions) carry their stereotypes, as in this example.

```xml
<class st:stereotype="entity"> ... </class>
<function st:stereotype="get"> ... </function>
```

> [!NOTE]
> srcML parses `.h` files as C by default. A C++ class in such a header then gets no class markup. The option `--register-ext h=C++` of `srcml` parses these headers as C++.

### Options

| Option | Description |
|---|---|
| `-o`, `--output-file` | Output archive (default `<input>.stereotypes.xml`) |
| `-f`, `--free-function` | Stereotypes free functions and static methods |
| `-t`, `--struct` | Stereotypes structs (C, C++, and C#) |
| `-i`, `--interface` | Stereotypes interfaces (C#, Java, and TypeScript) |
| `-u`, `--union` | Stereotypes unions (C and C++) |
| `-e`, `--enum` | Stereotypes enums (Java) |
| `-r`, `--csv-report` | Writes a CSV report of the stereotypes |
| `-x`, `--txt-report` | Writes a text report of the stereotypes |
| `-b`, `--verbose` | Writes a detailed CSV report (e.g., the calls of each function) and prints the settings in use |
| `-m`, `--comment` | Inserts a `// @stereotype <labels>` comment (`# @stereotype <labels>` in Python) before each stereotyped element |
| `-d`, `--no-srcml-output`, `--srcmlOutput` | Writes no output archive, only the reports of `-r`, `-x`, or `-b` |
| `-j`, `--threads` | Number of threads (default all available, at most 256) |
| `-l`, `--large-class` | Method count above which a type can be a `large-class` (default mean plus standard deviation over all types) |
| `-p`, `--primitive-file` | File of extra types that count as primitive (e.g., a typedef of `int`) |
| `-c`, `--call-file` | File of extra calls to ignore (e.g., a logging function) |
| `-s`, `--specifier-file` | File of extra specifiers to strip from types (e.g., an export macro) |
| `-q`, `--query-file` | File of extra methods whose calls leave the object unchanged (e.g., `size`) |
| `-v`, `--version` | Prints the version |
| `-h`, `--help` | Prints the options |

⚡ **Speed.** Stereocode takes about 2.5 s to annotate a srcML archive of 100 MB on 24 threads.

### List files

The options `-p`, `-c`, `-s`, and `-q` each read a text file that adds entries to a built-in list, one entry per line. A line `[C++]`, `[C#]`, `[Java]`, `[Python]`, or `[JavaScript]` limits the entries after it to that language, while the entries before the first such line hold for every language. The line `[JavaScript]` also covers TypeScript, which srcML marks as JavaScript. A line that starts with `#` is a comment.

| List | Example entries | Description |
|---|---|---|
| Primitive types (`-p`) | `qint64`, `qreal` | Qt typedefs of numbers |
| Ignored calls (`-c`) | `spdlog::info`, `Log.d` | Logging calls of spdlog and Android |
| Type specifiers (`-s`) | `SCI_METHOD` | A Scintilla macro that srcML reads as part of a return type |
| Query methods (`-q`) | `contains`, `isBlank` | Methods whose calls leave the object unchanged |

For example, this file `queries.txt` adds two C++ methods and one Java method for `-q`.

```text
# Methods whose calls leave the object unchanged
[C++]
contains
starts_with
[Java]
isBlank
```

```bash
build/stereocode my_project.xml -q queries.txt   # use the extra query methods
```

## 🧪 Tests

Each test name starts with the folder of its files under `test/`.

- **Unit tests** (`unit`) check the stereotype rules and the helper functions in one doctest program.
- **Integration tests** (`integration`) run `stereocode` on small archives and check the exit code, the messages, and the output files.
- **Stereotype cases** (`stereotypes`) are small C, C++, C#, Java, Python, JavaScript, and TypeScript files whose expected stereotypes are written by hand in `@stereotype` comments.
- **System tests** (`systems`) compare every label of RxJava, PowerShell, and Notepad++ with a table recorded from earlier runs.

The stereotype cases, the system tests, and the integration tests of the option `-m` convert source files to srcML with the `srcml` program. Without this program, the configuration skips these tests and names them in a status message. The system tests run one at a time, also under `ctest -j 8`, since each one runs on every thread.

### Commands

Run these commands inside the build folder (`cd build`).

| Command | Description |
|---|---|
| `ctest -j 8` | Every test |
| `ctest -LE systems` | Every test but the three systems |
| `ctest -L systems` | Only the three systems |
| `ctest -L stereotypes` | Only the stereotype cases and the helper checks |
| `ctest -R stereotypes/java` | The stereotype cases of one language |
| `ctest -L unit` | Only the unit tests |
| `ctest -L integration` | Only the integration tests |
| `ctest -R <name> --output-on-failure` | One test, with its output on a failure |
| `ctest -N` | Lists every test name |

## 📜 License

Stereocode is free software under the GNU General Public License version 3, as `LICENSE` states.

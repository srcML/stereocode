# 🧬 Stereocode

**Stereocode** classifies methods, free functions, and types of **C, C++, C#, and Java** code by their **stereotypes**, such as `get`, `command`, or `entity`. It reads a [srcML](https://www.srcml.org/) archive and writes the archive back with an `st:stereotype` attribute on each classified element.

```xml
<class st:stereotype="entity"> ... </class>
<function st:stereotype="get"> ... </function>
```

> [!TIP]
> The [wiki](https://github.com/srcML/stereocode/wiki) defines every method, class, and free-function stereotype with its rules and examples.

## 📦 Requirements

- **srcML 1.2**, with the program `srcml`, the library `libsrcml`, and the header `srcml.h` ([srcml.org](https://www.srcml.org/))
- **libxml2 headers**, such as the package `libxml2-dev` on Debian and Ubuntu
- **CMake 3.20** or newer and a **C++17** compiler (Clang, GCC, or MSVC)

## 🔧 Installation

```bash
git clone https://github.com/srcML/stereocode.git
cd stereocode
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The build writes the program `build/stereocode`.

## 🚀 Usage

```bash
srcml --register-ext h=C++ my_project -o my_project.xml
build/stereocode my_project.xml -f -r
```

The first command turns the source code into a srcML archive. The second classifies the code of the archive and writes `my_project.stereotypes.xml` and the CSV report `my_project.stereotypes.csv`.

> [!NOTE]
> srcML parses `.h` files as C by default. A C++ class in such a header then gets no class markup. The option `--register-ext h=C++` of `srcml` parses these headers as C++.

| Option | Effect |
|---|---|
| `-o`, `--output-file` | Output archive, `<input>.stereotypes.xml` by default |
| `-f`, `--free-function` | Also classifies free functions and static methods |
| `-t`, `--struct` | Also classifies structs |
| `-i`, `--interface` | Also classifies interfaces (C# and Java) |
| `-u`, `--union` | Also classifies unions (C and C++) |
| `-e`, `--enum` | Also classifies enums (Java) |
| `-r`, `--csv-report` | Writes a CSV report |
| `-x`, `--txt-report` | Writes a text report |
| `-b`, `--verbose` | Writes a verbose CSV report and prints the settings of the run |
| `-m`, `--comment` | Inserts a `// @stereotype <labels>` comment before each classified element |
| `-d`, `--no-srcml-output`, `--srcmlOutput` | Writes only the reports, with `-r`, `-b`, or `-x` |
| `-j`, `--threads` | Number of worker threads, as many as the machine has by default (at most 256) |
| `-l`, `--large-class` | Method count above which a type is large, computed per run by default |
| `-p`, `--primitive-file` | File of extra primitive types |
| `-c`, `--call-file` | File of extra calls to ignore |
| `-s`, `--specifier-file` | File of extra type specifiers to remove |
| `-q`, `--query-file` | File of extra query methods whose calls only read, such as `size` |
| `-v`, `--version` | Prints the version |
| `-h`, `--help` | Prints the options |

The files of `-p`, `-c`, `-s`, and `-q` hold one entry per line. A line `[C++]`, `[C#]`, or `[Java]` limits the entries after it to that language, and `#` starts a comment.

⚡ **Speed.** On 24 threads, a srcML archive of 100 MB takes about 2.5 s.

## 🧪 Tests

Each test name starts with the folder of its files under `test/`.

| Test names | Label | Description |
|---|---|---|
| `unit` | `unit` | Each unit test checks one function of the Stereocode source code, such as a stereotype rule. All unit tests run in one doctest program. |
| `integration/<topic>/<check>` | `integration` | Each integration test runs `stereocode` on a small srcML archive and checks its exit code, messages, and output files. |
| `stereotypes/<language>/<case>` | `stereotypes` | Each small stereotype case is one or a few short C++, C#, or Java files. A comment `// @stereotype <labels>` written by hand gives the expected stereotypes of the element on the next line, with `@defect` for a known bug or `@spec` for an open rule question. |
| `stereotypes/check_stereotypes/<check>` | `stereotypes` | Each self-check runs `check_stereotypes`, the helper program that compares the labels of the cases and the systems, on a hand-written archive. |
| `stereotypes/systems/<system>` | `systems` | Each system test runs `stereocode` on a real system (RxJava, PowerShell, or Notepad++) and compares every label with `<system>.tsv`, a table recorded from an earlier output of `stereocode`. A system test catches any change of a label, while it cannot prove that a label is right. |

| Command | Runs |
|---|---|
| `ctest --test-dir build -j 8` | Every test |
| `ctest --test-dir build -LE systems` | Every test but the three systems |
| `ctest --test-dir build -L systems` | Only the three systems |
| `ctest --test-dir build -L stereotypes` | Only the stereotype cases and the self-checks of the helper |
| `ctest --test-dir build -R stereotypes/java` | The stereotype cases of one language |
| `ctest --test-dir build -L unit` | Only the unit tests |
| `ctest --test-dir build -L integration` | Only the integration tests |
| `ctest --test-dir build -R <name> --output-on-failure` | One test, with its report on a failure |
| `ctest --test-dir build -N` | Lists every test without a run |

After a change of the labels or of the srcML version, `cmake --build build --target update_system_tables` regenerates the tables of `test/stereotypes/systems/`. Review their diff before a commit.

## 📓 Developer Notes

The initial version of this code base was developed by Doleh and documented in his MS Thesis December 2021 at Kent State University [7]. Later, Al-Ramadan re-wrote Stereocode, adding support for additional programming languages (i.e., C# and Java), enabling the stereotyping of complete systems, and incorporating deep static analysis. This work was also documented in his MS Thesis May 2024 at Kent State University [8] and published at ICSME 2024 [1]. Previously, a prototype of **Stereocode** was developed by Collard and Dragan for Dragan's dissertation in December 2010 at KSU [5]. This prototype was further extended for DySDoc 2018 by Decker and Collard [6]. The method and class stereotypes come from the work of Dragan et al. [2][3][4]. 

Developers of Stereocode:
- Ali Al-Ramadan - Kent State University
- Michael L. Collard - University of Akron
- Michael Decker - Bowling Green State University
- Zane Doleh - Kent State University
- Jonathan I. Maletic - Kent State University
- Nick Weber - Kent State University

[1] Al-Ramadan, A. F., Behler, J. A., Decker, M. J., Dragan, N., Collard, M. L., & Maletic, J. I. (2024, October). Stereocode: A Tool for Automatic Identification of Method and Class Stereotypes for Software Systems. In 2024 IEEE International Conference on Software Maintenance and Evolution (ICSME) (pp. 898-902). IEEE. https://doi.org/10.1109/ICSME58944.2024.00099

[2] Dragan, N., Collard, M. L., & Maletic, J. I. (2006, September). Reverse Engineering Method Stereotypes. In 2006 22nd IEEE International Conference on Software Maintenance (ICSM) (pp. 24-34). IEEE. https://doi.org/10.1109/ICSM.2006.54

[3] Dragan, N., Collard, M. L., & Maletic, J. I. (2009, September). Using Method Stereotype Distribution as a Signature Descriptor for Software Systems. In 2009 IEEE International Conference on Software Maintenance (ICSM) (pp. 567-570). IEEE. https://doi.org/10.1109/ICSM.2009.5306394

[4] Dragan, N., Collard, M. L., & Maletic, J. I. (2010, September). Automatic Identification of Class Stereotypes. In 2010 IEEE International Conference on Software Maintenance (ICSM) (pp. 1-10). IEEE. https://doi.org/10.1109/ICSM.2010.5609703

[5] Dragan, N. (2010). The Emergent Laws of Method and Class Stereotypes in Object Oriented Software (Doctoral dissertation). Kent State University.

[6] Guarnera, D., Collard, M. L., Dragan, N., Maletic, J. I., Newman, C., & Decker, M. (2018). Automatically Redocumenting Source Code with Method and Class Stereotypes. In 2018 IEEE Third International Workshop on Dynamic Software Documentation (DySDoc3) (pp. 3-4). IEEE. https://doi.org/10.1109/DySDoc3.2018.00008

[7] Doleh, Z. (2021). Stereocode: A Tool for Automatic Identification of Method Stereotypes (Master's thesis). Kent State University. https://etd.ohiolink.edu/acprod/odb_etd/ws/send_file/send?accession=kent1636394189362288&disposition=inline

[8] Al-Ramadan, A. (2024). An Empirical Investigation of Method and Class Stereotype Distributions Across Software Systems in C++, C#, and Java (Master's thesis). Kent State University. https://etd.ohiolink.edu/acprod/odb_etd/ws/send_file/send?accession=kent1713744395637476&disposition=inline

## 📜 License

Stereocode is free software under the GNU General Public License version 3, as `COPYING.txt` states.

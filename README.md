# 🛠 **Stereocode 1.0**


## 💡 **What does it do?**
**Stereocode** classifies **methods** and **classes** based on their **stereotypes** (behavior) for **C**, **C++**, **Java**, and/or **C#** systems.

In addition, Stereocode stereotypes **free functions** as well as other structures (e.g., struct)
- Please refer to the Wiki for detailed information on stereotypes

**Stereocode**  takes a srcML archive as input, performs static analysis, and annotates each function and class tag in the XML input with an attribute indicating the detected stereotype. For example:

```XML
<class st:stereotype="entity"> ... </class>
<function st:stereotype="get"> ... </function>
```

## 🔧 Installation and Build
1. Prerequisites
- [srcML 1.1+](https://www.srcml.org/) (Including develop version for Linux)
- [cmake 3.20+](https://cmake.org/)
- Clang, GCC, or MSCV with C++17 or higher
- Git and Git LFS (Optional)

2. Clone this repository.
```bash
git clone https://github.com/srcML/stereocode.git

# If you need testing:
git lfs pull
```

3. Build using cmake:

```bash
cmake CMakeLists.txt -B build_path
cd build_path
make
```

## 🚀 Usage

**Stereocode**  is a command-line tool. It can process individual source files or whole systems. 

```bash
Demo: 

# This line converts the PowerShell system to the srcML format
srcml PowerShell.zip -o PowerShell.xml

# Saves the output to PowerShell-output.xml
./stereocode PowerShell.xml -o PowerShell-output.xml

# For more options and help:
./stereocode --help
```

## 📜 Stereocode Options

<span style='color: lightgreen;'>**-o, --output-file:**</span> File name of srcML output archive annotated with stereotypes

<span style='color: lightgreen;'>**-p, --primitive-file:**</span> File name of user primitive types (one per line) </br>
```
Datatype_1
Datatype_2
...
```
These data types will be treated as primitive data types such as an **int**.

<span style='color: lightgreen;'>**-g, --ignore-call-file:**</span> File name of user calls to ignore (one per line) </br>
```
Call_Name_1
Call_Name_2
...
```
Do not use namespaces, parenthesis, or other special characters. Simply list the call name itself. For example, **foo**. </br>
These calls are ignored from analysis. However, usage of fields within these calls (as parameters) are not ignored (considered as accessors to attributes).

<span style='color: lightgreen;'>**-t, --type-specifier-file:**</span> File name of user type specifiers to remove (one per line) </br>
```
specifier_1
specifier_2
...
```
These specifiers, such as **public**, are removed during analysis to enhance the detection of certain elements such as primitive data types and method return types. 

<span style='color: lightgreen;'>**-l, --large-class \[int]:**</span> Method threshold for the large type stereotype (e.g., large-class) (default = 21)

<span style='color: lightgreen;'>**-f, --free-function:**</span> Identify stereotypes for free functions (includes static methods) (C, C++, C#, and Java)

<span style='color: lightgreen;'>**-i, --interface:**</span> Identify stereotypes for interfaces (C# and Java)

<span style='color: lightgreen;'>**-n, --union:**</span> Identify stereotypes for unions (C++)

<span style='color: lightgreen;'>**-m, --enum:**</span> Identify stereotypes for enums (Java)

<span style='color: lightgreen;'>**-s, --struct:**</span> Identify stereotypes for structs (C, C++, C# and Java)

<span style='color: lightgreen;'>**-z, --csv-report:**</span> Output optional CSV file containing stereotypes

<span style='color: lightgreen;'>**-b, --verbose:**</span> Verbose output: primitives, ignorable calls, specifiers, and CSV with metadata

<span style='color: lightgreen;'>**-v, --version:**</span> Display version information

## 📓 Developer Notes

The initial version of this code base was developed by Doleh and documented in his MS Thesis December 2021 at Kent State University. Later, Al-Ramadan re-wrote Stereocode, adding support for additional programming languages (i.e., C# and Java), enabling the stereotyping of complete systems, and incorporating deep static analysis. This work was also documented in his MS Thesis May 2024 at Kent State University and published at ICSME 2024 [1]. Previously, a prototype of **Stereocode** was developed by Collard and Dragan for Dragan's dissertation in December 2010 at KSU. This prototype was further extended for DySDoc 2018 by Decker and Collard. 

Developers of Stereocode:
- Ali Al-Ramadan - Kent State University
- Michael L. Collard - University of Akron
- Michael Decker - Bowling Green State University
- Zane Doleh - Kent State University
- Jonathan I. Maletic - Kent State University
- Nick Weber - Kent State University

[1] Al-Ramadan, A. F., Behler, J. A., Decker, M. J., Dragan, N., Collard, M. L., & Maletic, J. I. (2024, October). Stereocode: A Tool for Automatic Identification of Method and Class Stereotypes for Software Systems. In 2024 IEEE International Conference on Software Maintenance and Evolution (ICSME) (pp. 898-902). IEEE.
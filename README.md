# 🛠 **Stereocode 1.0**


## 💡 **What does it do?**
**Stereocode** classifies **methods** and **classes** based on their **stereotypes** for **C++**, **Java**, and/or **C#** systems.

In addition, Stereocode stereotypes **free functions** as well as other structures (e.g., struct)
- Please refer to the Wiki for detailed information on stereotypes

**Stereocode**  takes a srcML archive as input, performs static analysis, and annotates each function and class tag in the XML input with an attribute indicating the detected stereotype. For example:

```XML
<class st:stereotype="entity"> ... </class>
<function st:stereotype="get"> ... </function>
```

## 🔧 Installation and Build
1. Prerequisites
- [srcML 1.0+](https://www.srcml.org/) (Develop version for Linux)
- [cmake 3.17+](https://cmake.org/)
- GCC, Clang, or MSCV with C++17 or higher

2. Clone or download this repository.

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

<span style='color: lightgreen;'>**-o, --output-file:**</span> File name of output - srcML archive with stereotypes.

<span style='color: lightgreen;'>**-p, --primitive-file:**</span> File name of user supplied primitive types (one per line). </br>
```
Datatype_1
Datatype_2
...
```
These data types will be treated as primitive data types such as an **int**.

<span style='color: lightgreen;'>**-g, --ignore-call-file:**</span> File name of user supplied calls to ignore (one per line). </br>
```
Call_Name_1
Call_Name_2
...
```
Do not use namespaces, parenthesis, or other special characters. Simply list the call name itself. For example, **foo**. </br>
These calls are ignored from analysis. However, usage of fields within these calls (as parameters) are not ignored (considered as accessors to attributes).

<span style='color: lightgreen;'>**-t, --type-modifier-file:**</span> File name of user supplied data type modifiers to remove (one per line). </br>
```
modifier_1
modifier_2
...
```
These modifiers, such as **public**, are removed during analysis to enhance the detection of certain elements such as primitive data types and method return types. 

<span style='color: lightgreen;'>**-l, --large-class \[int]:**</span> Method threshold for the large-class stereotype (default = 21).

<span style='color: lightgreen;'>**-f, --free-function:**</span> Identify stereotypes for free functions (C++, C#, and Java). 

<span style='color: lightgreen;'>**-i, --interface:**</span> Identify stereotypes for interfaces (C# and Java). 

<span style='color: lightgreen;'>**-n, --union:**</span> Identify stereotypes for unions (C++). 

<span style='color: lightgreen;'>**-m, --enum:**</span> Identify stereotypes for enums (Java). 

<span style='color: lightgreen;'>**-s, --struct:**</span> Identify stereotypes for structs (C# and Java). 

<span style='color: lightgreen;'>**-e, --input-overwrite:**</span> Overwrite input with stereotype information. 

<span style='color: lightgreen;'>**-x, --txt-report:**</span> Output optional TXT report file containing stereotype information. 

<span style='color: lightgreen;'>**-z, --csv-report:**</span> Output optional CSV report file containing stereotype information. 

<span style='color: lightgreen;'>**-c, --comment:**</span> Annotates stereotypes as a comment before method and class definitions (/** @stereotype stereotype */). 

<span style='color: lightgreen;'>**-v, --verbose:**</span> Outputs default primitives, ignored calls, type modifiers, and extra report files.

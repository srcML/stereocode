# 🛠 **Stereocode 1.0**


## 💡 **What does it do?**
**Stereocode** classifies **methods** and **classes** based on their **stereotypes**.

> Class and method stereotypes are defined in three papers presented at the **IEEE International Conference on Software Maintenance (ICSM)** in 2006, 2009, and 2010 by Dragan, Collard, and Maletic.

**Stereocode**  takes a srcML archive as input, performs static analysis, and annotates each function and class tag in the XML input with an attribute indicating the detected stereotype. For example:

```XML
<class st:stereotype="entity"> ... </class>
<function st:stereotype="get"> ... </function>
```

This output can be used for further processing or analysis. For instance, the stereotype information can be embedded as documentation in the source code (using comments, **Doxygen**, **Javadoc**, etc.).


## 🔧 Installation and Build
1. Prerequisites
- [srcml 1.1+](https://www.srcml.org/) (Client + Develop)
- [cmake 3.17+](https://cmake.org/)
- GCC, Clang, or MSCV with C++17 or higher

2. Clone or download this repository.

3. Build using cmake:

```bash
cmake CMakeLists.txt -B build_path
cd build_path
make
```


Stereocode is compatible with **srcML v1.0**, but it might not work as intended in certain cases as it is supported in Stereocode using a workaround. </br>

On MAC OS, if you get an error related to "dyld library not loaded", then you need to export the path of libsrcml.dylib as follows: </br> 
```bash
export DYLD_LIBRARY_PATH=/usr/local/lib:$DYLD_LIBRARY_PATH 
```

Or add it to the shell configuration **.zshrc or .bash_profile** file to make it permanent.


## 🚀 Usage

**Stereocode**  is a command-line tool. It can process individual source files or entire systems written in **C++**, **Java**, and/or **C#**. 

Demo: 
```bash
# This line converts the PowerShell system to the srcML format
srcml PowerShell.zip -o PowerShell.xml

# Saves the output to PowerShell-output.xml
./stereocode PowerShell.xml -o PowerShell-output.xml

# For more options and help:
./stereocode --help
```

Note:</br>
Stereocode can stereotype *free functions*. A *free function* could a static method/function, a friend function (C++), or simply a function that does not belong to a class (C++) **(This is still under development, a taxonomy will be available soon)**.

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
These calls are ignored from analysis. However, usage of attributes within these calls (as parameters) are not ignored (considered as accessors to attributes).

<span style='color: lightgreen;'>**-t, --type-modifier-file:**</span> File name of user supplied data type modifiers to remove (one per line). </br>
```
modifier_1
modifier_2
...
```
These modifiers are removed during analysis to enhance the detection of certain elements such as primitive data types and method return types. 

<span style='color: lightgreen;'>**-l, --large-class \[int]:**</span> Method threshold for the large-class stereotype (default = 21).

<span style='color: lightgreen;'>**-i, --interface:**</span> Identify stereotypes for interfaces (C# and Java). 

<span style='color: lightgreen;'>**-n, --union:**</span> Identify stereotypes for unions (C++). 

<span style='color: lightgreen;'>**-m, --enum:**</span> Identify stereotypes for enums (Java). 

<span style='color: lightgreen;'>**-s, --struct:**</span> Identify stereotypes for structs (C# and Java). 

<span style='color: lightgreen;'>**-e, --input-overwrite:**</span> Overwrite input with stereotype information. 

<span style='color: lightgreen;'>**-x, --txt-report:**</span> Output optional TXT report file containing stereotype information. 

<span style='color: lightgreen;'>**-z, --csv-report:**</span> Output optional CSV report file containing stereotype information. 

<span style='color: lightgreen;'>**-c, --comment:**</span> Annotates stereotypes as a comment before method and class definitetions (/** @stereotype stereotype */). 

<span style='color: lightgreen;'>**-v, --verbose:**</span> Outputs default primitives, ignored calls, type modifiers, and extra report files.

## 📓 Developer Notes:

The initial version of this code base was developed by Doleh and documented in his MS Thesis December 2021 at Kent State University. Later, Al-Ramadan updated Stereocode by adding support for additional programming languages (i.e., C# and Java), enabling the stereotyping of complete systems, and incorporating deep static analysis. This work was also documented in MS Thesis May 2024 at Kent State University. Previously, a prototype of **Stereocode** was developed by Collard and Dragan for Dragan's dissertation in December 2010 at KSU. This prototype was further extended for DySDoc 2018 by Decker and Collard. 

Developers of stereocode 1.0:
- Ali Al-Ramadan - Kent State University
- Michael L. Collard - University of Akron
- Michael Decker - Bowling Green State University
- Zane Doleh - Kent State University
- Jonathan I. Maletic - Kent State University
- Nick Weber - Kent State University

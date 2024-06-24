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
- [srcml 1.1+](https://www.srcml.org/)
- [cmake 3.17+](https://cmake.org/)
- C++17 or higher

2. Clone or download this repository.

3. Build using cmake:

```bash
cmake CMakeLists.txt -B build_path
cd build_path
make
```

Note: Stereocode is compatible with srcML v1.0, but might not work as intended in certain rare cases.

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

## 📜 Stereocode Options

<span style='color: lightgreen;'>**-o, --output-file:**</span> File name of output - srcML archive with stereotypes.

<span style='color: lightgreen;'>**-p, --primitive-file:**</span> File name of user supplied primitive types (one per line). 

<span style='color: lightgreen;'>**-g, --ignore-call-file:**</span> File name of user supplied calls to ignore (one per line). 

<span style='color: lightgreen;'>**-t, --type-token-file:**</span> File name of user supplied data type tokens to remove (one per line). 

<span style='color: lightgreen;'>**-i, --enable-interface:**</span> Identify stereotypes for interfaces (C# and Java). 

<span style='color: lightgreen;'>**-s, --enable-struct:**</span> Identify stereotypes for structs (C# and Java). 

<span style='color: lightgreen;'>**-e, --input-overwrite:**</span> Overwrite input with stereotype information. 

<span style='color: lightgreen;'>**-x, --txt-report:**</span> Output optional TXT report file containing stereotype information. 

<span style='color: lightgreen;'>**-z, --csv-report:**</span> Output optional CSV report file containing stereotype information. 

<span style='color: lightgreen;'>**-c, --comment:**</span> Annotates stereotypes as a comment before method and class definitetions (/** @stereotype stereotype */). 

<span style='color: lightgreen;'>**-l, --large-class \[int]:**</span> Method threshold for the large-class stereotype (default = 21).

## 📓 Developer Notes:

The initial version of this code base was developed by Doleh and documented in his MS Thesis December 2021 at Kent State University. Later, Al-Ramadan updated Stereocode by adding support for additional programming languages (i.e., C# and Java), enabling the stereotyping of complete systems, and incorporating deep static analysis. This work was also documented in MS Thesis May 2024 at Kent State University. Previously, a prototype of **Stereocode** was developed by Collard and Dragan for Dragan's dissertation in December 2010 at KSU. This prototype was further extended for DySDoc 2018 by Decker and Collard. 

Developers of stereocode 1.0:
- Ali Al-Ramadan - Kent State University
- Michael L. Collard - University of Akron
- Michael Decker - Bowling Green State University
- Zane Doleh - Kent State University
- Jonathan I. Maletic - Kent State University
- Nick Weber - Kent State University

# 🛠 **Stereocode 1.0**

## 📋 **Prerequisites**
- [srcml 1.0+](https://www.srcml.org/)
- [cmake 3.17+](https://cmake.org/)


## 💡 **What does it do?**
**Stereocode** classifies **methods** and **classes** based on their **stereotypes**.

> Class and method stereotypes are defined in two papers presented at the **IEEE International Conference on Software Maintenance (ICSM)** in 2006 and 2010 by Dragan, Collard, and Maletic.

**Stereocode**  takes a srcML-formatted archive as input, performs static analysis on the code, and annotates the srcML with labels indicating the detected stereotypes. The output is a srcML format with stereotype attributes embedded within class and function tags. An optional report summarizing the stereotypes of classes and their methods can also be generated.

*Users can use this output for further processing or analysis. For instance, the stereotype information can be embedded as documentation in the source code (using comments, **Doxygen**, **Javadoc**, etc.).*

The tool currently supports **C++**, **Java**, and **C#**.
## 🔧 Installation and Build
1. Clone or download this repository.

2. Build using cmake:

```bash
cmake CMakeLists.txt -B build_path
cd build_path
make
```

## 🚀 Usage

**Stereocode**  is a command-line tool.

It can process individual files, multiple files, or entire systems. The input is expected to be in srcML format. 

Demo: 
```bash
# Process CLI11 and store the output in CLI11-C++.stereotypes.xml
./stereocode CLI11-C++.xml

# Save the output in Dapper-output.xml
./stereocode Dapper-CS.xml -o Dapper-output.xml

# Overwrite the input file with stereotype output 
./stereocode PowerShell-CS.xml -v

# Generate an output PowerShell-CS.stereotypes.xml and an optional report PowerShell-CS.xml.report.txt
./stereocode PowerShell-CS.xml -r

# Generate ONLY the report, saved as PowerShell-CS.xml.report.txt.
./stereocode PowerShell-CS.xml -d
```

There are a predefined set of primitive (base) types for each language.  Additional primitive type (system specific) can be supplied by the user <span style='color: lightgreen;'>**(-p --primitives option)**</span> as follows:

```
./stereocode PowerShell-CS.xml -p primitive-file.txt
```
The **primitive-file.txt** should list data types, one per line, without spaces. Example:
```
longlong
intlong
```

For more options and help:
```bash
./stereocode --help
./OrderBy --help
```


## 📜 Stereocode Options



<span style='color: lightgreen;'>**long-o, --output-file:**</span> Specify the output file name. If omitted, the default is input-fname.stereotypes.xml.

<span style='color: lightgreen;'>**-v, --overwrite:**</span> Overwrite the input file with stereotype output. This option takes precedence over -o.

<span style='color: lightgreen;'>**-p, --primitives:**</span> Specify file name of user defined primitive/base types (one per line without spaces).  This is added to initial list of primitive types. 

<span style='color: lightgreen;'>**-r, --report:**</span> Generate a report file named input-fname.report.txt. 

<span style='color: lightgreen;'>**-d, --report-only:**</span> Generate only a report file named input-fname.report.txt. No XML output is created. This option takes precedence over -o and -v.

<span style='color: lightgreen;'>**-c, --large-class \[int]:**</span> Define a parameter to determine the large-class stereotype. Default is 21.


## 📄 Output

By default, **Stereocode** generates an annotated archive for each input, and optionally, a report file. An option to generate only a report file is also available.

The class and function tags are given a stereotype attribute, e.g.::
```XML
<class st:stereotype="entity"> ... </class>
<function st:stereotype="get"> ... </function>
```


## 📓 Developer Notes:

The initial version of this code base was developed by Doleh and documented in his MS Thesis December 2021 at Kent State University. Previously, a prototype of **Stereocode** was developed by Collard and Dragan for Dragan's dissertation in December 2010 at KSU.  This prototype was further extended for DySDoc 2018 by Decker and Collard.

Developers of stereocode 1.0:
- Ali Al-Ramadan - Kent State University
- Michael L. Collard - University of Akron
- Michael Decker - Bowling Green State University
- Zane Doleh - Kent State University
- Jonathan I. Maletic - Kent State University
- Nick Weber - Kent State University

# 🛠 **Stereocode 1.0**


## 💡 **What does it do?**
**Stereocode** classifies **methods** and **classes** based on their **stereotypes**.

> Class and method stereotypes are defined in two papers presented at the **IEEE International Conference on Software Maintenance (ICSM)** in 2006 and 2010 by Dragan, Collard, and Maletic.

**Stereocode**  takes a srcML-formatted archive as input, performs static analysis on the code, and annotates the srcML with labels indicating the detected stereotypes. The output is a srcML archive with stereotype attributes embedded within class and function tags. 

*This output can be used for further processing or analysis. For instance, the stereotype information can be embedded as documentation in the source code (using comments, **Doxygen**, **Javadoc**, etc.).*

The tool supports stereotyping for **C++**, **Java**, and **C#**.

## 🔧 Installation and Build
1. Prerequisites
- [srcml 1.1+](https://www.srcml.org/)
- [cmake 3.17+](https://cmake.org/)

2. Clone or download this repository.

3. Build using cmake:

```bash
cmake CMakeLists.txt -B build_path
cd build_path
make
```

## 🚀 Usage

**Stereocode**  is a command-line tool.

It can process individual source files or entire systems. The input is expected to be in srcML format. 

Demo: 
```bash
# This line converts the PowerShell system to the srcML format
srcml PowerShell.zip -o PowerShell.xml

# Runs Stereocode on PowerShell.xml and stores the output in PowerShell.stereotypes.xml
./stereocode PowerShell.xml

# Saves the output to PowerShell-output.xml
./stereocode PowerShell.xml -o PowerShell-output.xml

# Overwrite the input file with the stereotype output 
./stereocode PowerShell.xml -v

# Generate an output PowerShell.stereotypes.xml and an optional report PowerShell.xml.report.txt
./stereocode PowerShell.xml -r
```

For more options and help:
```bash
./stereocode --help
```


## 📜 Stereocode Options

<span style='color: lightgreen;'>**-o, --output-file:**</span> Specify the output file name - srcML archive with stereotypes.

<span style='color: lightgreen;'>**-p, --primitives:**</span> Specify file name of user defined primitive/base types (one per line without spaces).  This is added to an initial list of primitive types. 

<span style='color: lightgreen;'>**-g, --ignore-calls:**</span> Specify file name of user defined calls to ignore (one per line without spaces).  This is added to an initial list of ignored calls. 

<span style='color: lightgreen;'>**-i, --interface:**</span> Identify stereotypes for interfaces (C# and Java). 

<span style='color: lightgreen;'>**-s, --struct:**</span> Identify stereotypes for structs (C# and Java). 

<span style='color: lightgreen;'>**-c, --large-class \[int]:**</span> Define a parameter to determine the large-class stereotype. Default is 21.

<span style='color: lightgreen;'>**-v, --overwrite:**</span> Overwrite the input file with stereotype output. 

<span style='color: lightgreen;'>**-r, --report:**</span> Generate an optional TXT report file. 

<span style='color: lightgreen;'>**-w, --stereotype-views:**</span> Output optional CSV files capturing different views of method and class stereotypes - *.view_type.csv".


## 📄 Output

By default, **Stereocode** generates an annotated archive for each input.
Stereocode can also generate optional reports summarizing the stereotypes from different views (e.g., class view or method view).

The annotated class and function tags are given a stereotype attribute, e.g.:
```XML
<class st:stereotype="entity"> ... </class>
<function st:stereotype="get"> ... </function>
```


## 📓 Developer Notes:

The initial version of this code base was developed by Doleh and documented in his MS Thesis December 2021 at Kent State University. Previously, a prototype of **Stereocode** was developed by Collard and Dragan for Dragan's dissertation in December 2010 at KSU.  This prototype was further extended for DySDoc 2018 by Decker and Collard. Later, Al-Ramadan enhanced Stereocode by adding support for additional programming languages (i.e., C# and Java), enabling the stereotyping of complete systems, and incorporating deep static analysis.

Developers of stereocode 1.0:
- Ali Al-Ramadan - Kent State University
- Michael L. Collard - University of Akron
- Michael Decker - Bowling Green State University
- Zane Doleh - Kent State University
- Jonathan I. Maletic - Kent State University
- Nick Weber - Kent State University

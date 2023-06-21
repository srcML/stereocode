# Stereocode 1.0

## Prerequisites
- [srcml 1.0](https://www.srcml.org/)
- [cmake 3.17+](https://cmake.org/)


## What does it do?
Stereocode classifies methods and classes with one or more stereotypes.

Class and method stereotypes are defined in two papers appearing in IEEE International Conference on Software Maintenance (ICSM) 2006 and 2010 by Dragan, Collard, and Maletic.

The Stereocode application accepts an archive consisting of .hpp and .cpp file pair(s) in srcML format as input. It performs static analysis on the code and annotates the srcML with labels indicating the stereotypes of the classes and their methods. The output is produced in srcML format, with stereotype attributes embedded in the class and function tags.

Users of stereocode use this output as input to additional processing or analysis.  For example, the stereotype information can be added as documentation (comments, doxgen, javadoc) to the source code.

## Installation and Build
Intall srcml version 1.0 and cmake version 3.17 or later on Linux/Unix/MacOS.

Clone or download this repo.

Build using cmake.

```
cmake CMakeLists.txt -B build_path

# From build_path with created makefile
make
```

## Usage

Stereocode is run on the command line.

Stereocode expects each pair in the srcML archive file to contain only one class definition. For C++, the archive typically will have pairs of .hpp and .cpp. where each pair corresponds to a single class. 


Demo: 
```
# Toolchain

srcml example1.hpp example1.cpp example2.hpp example2.cpp -o Examples.xml
./stereocode Examples.xml

# The .cpp files can be placed first

srcml example1.cpp example1.hpp example2.cpp example2.hpp -o Examples.xml
./stereocode Examples.xml

# Using a single .hpp file

srcml example1.hpp -o example1hpp.xml
./stereocode example1hpp.xml
```

Help
```
./stereocode --help
```


Examples can be found in the `examples` folder

There are a predefined set of primitive (base) types for each language.  Additional primitive type (system specific) can be supplied by the user (--primitives option).


## Options

-o, output-file \[relative path] - Specify a file the name of the output file.  If not specified output is input-fname.annotated.xml

-v, --overwrite - Option to over write input file with stereotype output (off by default).

-p \--primitives[relative path] - Specifiy file name of user defined primitive/base types.  This is added to initial list of primitive types.

-r, --report - Option to output a report file (input-fname.report.txt).  Tab delimited: file path, class name, method header, stereotype.

-c, --large-class \[int] - Specify a parameter in determining the large-class stereotype.  Default is 21. Normally is the average methods/class + one standard deviation for a system.

-d, --debug - Option to turn on some output for debugging. 


## Output

Stereocode outputs an annotated archive for each input archive and optionally a report file.

By default annotated archives have the same name/path as the input archive with `annotated.xml` at the end of the name.  There are options to overwrite the input file or specify a output file name.

The class and function tags are given a stereotype attribute:
```
<class st:stereotype="entity"> ... </class>
<function st:stereotype="get"> ... </function>
```

## Developer Notes:

The initial version of this code base was developed by Doleh and documented in his MS Thesis December 2021 at Kent State University.   Previously, a prototype of stereocode was developed by Collard and Dragan for Dragan's dissertation in December 2010 at KSU.  This prototype was further extended for DySDoc 2018 by Decker and Collard.

Developers of stereocode 1.0:
- Ali Al-Ramadan - Kent State University
- Michael L. Collard - University of Akron
- Michael Decker - Bowling Green State University
- Zane Doleh - Kent State University
- Jonathan I. Maletic - Kent State University
- Nick Weber - Kent State University

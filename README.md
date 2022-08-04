# Stereocode 1.0

## Prerequitsites
- [srcml 1.0](https://www.srcml.org/)
- [cmake 3.18+](https://cmake.org/)


## What does it do?
Stereocode classifies methods and classes with one or more stereotypes.

Class and method stereotypes are defined in two papers appearing in IEEE International Conference on Software Maintenance (ICSM) 2006 and 2010 by Dragan, Collard, and Maletic.

It takes a class (in srcML) as input, conducts static analysis on the code, and annotates the srcML with the stereotype of the class and each method in the class.  The output is in srcML with stereotype attributes on the class and function tags.  

Users of stereocode use this output as input to additional processing or analysis.  For example, the stereotype information can be added as documentation (comments, doxgen, javadoc) to the source code.

## Installation and Build
intall srcml version 1.0 and cmake version 3.18 or later on Linux/Unix/MacOS

clone or download this repo

build using cmake

```
cmake path/CMakeLists.txt

#from location with created makefile
make
```

## Usage

Stereocode is run on the command line

Example: Range.xml is a srcML archive
```
./stereocode -a stereocode_tests/Range.xml
```

Help
```
./stereocode --help
```

Tool chain 
```
srcml foo.hpp foo.cpp -o foo.xml
stereocode -a foo.xml
```


Stereocode expects each srcML archive file to contain only one class definition.  For C++ the archive typically will have two units with the .hpp first and .cpp second.  

Example archives can be found in the `stereocode_tests` folder

There are a predefined set of primitive (base) types for each language.  Additional primitive type (system specific) can be suppled by the user (--primitives option).


## Options

-a, --archive \[relative path] - Specify a single input srcML archive. Archives passed in this options must have one class.

-l, --list-file \[relative path] - This option is to specify a file containing a list paths of archives described above in the -a option. Paths are relative to the stereocode executable file. Each path must be on a new line.

-a or -l options are required to provide input.

-o, output-file \[relative path] - Specify a file the name of the output file.  If not specified output is input-fname.annotated.xml

-v, --overwrite - Option to over write input file with stereotype output (off by default).

-p \--primitives[relative path] - Specifiy file name of user defined primitive/base types.  This is added to initial list of primitive types.

-r, --report - Option to output a report file (input-fname.report.txt).  Tab delimited: file path, class name, method header, stereotype.

-c, --large-class \[int] - Specify a parameter in determining the large-class stereotype.  Default is 21. Normally is the average methods/class + one standard deviation for a system.

-d, --debug - Option to turn on some output for debugging. 



## Output

Stereocode outputs an annotated archive for each input archive and optionaly a report file.

By default annotated archives have the same name/path as the input archive with `annotated.xml` at the end of the name.  There are options to overwrite the input file or specify a output file name.

The class and function tags are given a stereotype attribute:
```
<class st:stereotype="entity"> ... </class>
<function st:stereotype="get"> ... </function>
```

Class stereotypes:
- unclassified
- entity
- minimal-entity
- data-provider
- command
- boundary
- control
- pure-control
- factory
- large-class
- lazy-class
- degenerate
- data-class
- small-class

Method stereotypes:
- unclassified
- get
- non-const-get
- set
- predicate
- property
- void-accessor
- collaborator
- command
- non-void-command
- controller
- factory
- empty
- stateless
- wrapper


## Developer Notes:

The initial version of this code base was developed by Doleh and documented in his MS Thesis December 2021 at Kent State University.   Previously, a prototype of stereocode was developed by Collard and Dragan for Dragan's dissertation in December 2010 at KSU.  This prototype was further extended for DySDoc 2018 by Decker and Collard.

Developers of stereocode 1.0:
- Michael L. Collard - University of Akron
- Michael Decker - Bowling Green State University
- Zane Doleh - Kent State University
- Jonathan I. Maletic - Kent State University

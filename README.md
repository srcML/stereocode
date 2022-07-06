# Stereocode 1.0

## Prerequitsites
- [srcml 1.0](https://www.srcml.org/)
- [cmake](https://cmake.org/)


## Installation
intall srcml version 1.0 and cmake version 3.18 or later

clone or download this repo

locate the folder containing `Stereocode.cpp` 

build using cmake

example 
```
cmake [path to CMakeLists.txt]

#from location with created makefile
make
```

## Usage

examine PrimitiveTypes.txt and add any type names that are considered primitive (non-object) types.

stereocode can be run by using the command line, navigate to the executable location and use the executable name

example run command
```
./stereocode -a stereocode_tests/Range.xml
```
```
./stereocode -l stereocode_tests/ExampleList.txt
```

stereocode expects each archive file to contain 1 class definition and it must be in the first unit of the archive

example archives can be found in the `stereocode_tests` folder


## Options

-a \[relative path] - This option is to specify a single input srcML archive. Archives passed in this options must have one class, one header file and one implemtation file

-l \[relative path] - This option is to specify a file containing a list paths of archives described above in the -a option. Paths are relative to the stereocode executable file. Each path must be on a new line.

 -a or -l options are required to provide input.

-p \--primitives[relative path] - Specifiy file name of user defined primitive types.  This is added to initial list of primitive types.



## Output

stereocode outputs both an annotated archive for each input archive and a report file (CSV) 

Annotated archives have the same name/path as the input archive with `annotated.xml` at the end of the name.  For each <function> tag there is a 
stereotype attribute.

Report files have the same name/path as the input archive with `report.txt` at the end of the name.  The report file is a CSV file that contains:

1. input file path
2. method's header
3. assigned stereotype

separated by the pipe symbol `|`

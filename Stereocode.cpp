// Stereocode main
//
// compile with cmake, then make
//
// Input assumptions:
//  given cpp and hpp in 1 archive.
//  only 1 class per file   

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>

#include <srcml.h>
#include <CLI11.hpp>
#include "ClassInfo.hpp"
#include "PrimitiveTypes.hpp"

std::vector<std::string> readFileNames(const std::string&);

primitiveTypes PRIMITIVES;  //Global set of primitives.


int main(int argc, char const *argv[])
{
    CLI::App app{"StereoCode: Determines method stereotypes"};

    std::string fileList = "";
    std::string inputFile = "";
    std::string primitivesFile = "";
    std::string outputFile = "";
    bool        outputReport = false;
    std::vector<std::string> inputFileList;

    app.add_option("-a,--archive",     inputFile,      "File name of a single srcML archive (for C++ it is the hpp and cpp units");
    app.add_option("-o,--output-file", outputFile,     "File name of output srcML archive with stereotypes");
    app.add_option("-l,--list-file",   fileList,       "File name that contains a list of srcML archives");
    app.add_option("-p,--primitives",  primitivesFile, "File name of user supplied primitive types (one per line)");
    app.add_flag  ("-r,--report",      outputReport,   "Output optional report file (off by default)");

    CLI11_PARSE(app, argc, argv);

    if (inputFile != "") {
        inputFileList.push_back(inputFile);
    }
    else if(fileList != "") {
        inputFileList = readFileNames(fileList);
    } else {
        std::cerr << "Error: No input given " << std::endl;
        return -1;
    }
    if (primitivesFile != "") {  //Add any user defined primitive types to initial set
        std::ifstream in(primitivesFile);
        if (in.is_open())
            in >> PRIMITIVES;
        else {
            std::cerr << "Error: Primitive types file not found: " << primitivesFile << std::endl;
            return -1;
        }
        in.close();
    }

    std::cerr << "Computing stereotypes for the following classes: " << std::endl;

    for (int i = 0; i < inputFileList.size(); ++i){
        int error;
        srcml_archive* archive = srcml_archive_create();
        error = srcml_archive_read_open_filename(archive, inputFileList[i].c_str());   // read the srcml archive file
        if (error) {
            std::cerr << "Error: File not found: " << inputFileList[i] << ", error == " << error << "\n";
            return -1;
        }
        srcml_unit* firstUnit = srcml_archive_read_unit(archive);   //.hpp, .java, .cs, etc
        srcml_unit* secondUnit = srcml_archive_read_unit(archive);  //.cpp - only C++ has two units (hpp is first)
        classModel  aClass  = classModel(archive, firstUnit, secondUnit);

        aClass.stereotypeGetter();
        aClass.stereotypeSetter();
        aClass.stereotypeCollaborationalCommand();
        aClass.stereotypePredicate();
        aClass.stereotypeProperty();
        aClass.stereotypeVoidAccessor();
        aClass.stereotypeCommand();
        aClass.stereotypeFactory();
        aClass.stereotypeEmpty();
        aClass.stereotypeCollaborator();
        aClass.stereotypeStateless();

        if (outputReport) {
            std::ofstream reportFile;
            reportFile.open(inputFileList[i] + ".report.txt");
            aClass.printReportToFile(reportFile, inputFileList[i]);
            reportFile.close();
        }

        std::cerr << "Class name: " << aClass.getClassName() << std::endl;
        if (aClass.getUnitOneCount() != 0) {
            firstUnit = aClass.writeStereotypeAttribute(archive, firstUnit, true);
        }
        if (aClass.getUnitTwoCount() != 0) {
            secondUnit = aClass.writeStereotypeAttribute(archive, secondUnit, false);
        }

        srcml_archive* output_archive = srcml_archive_create();
        error = srcml_archive_register_namespace(output_archive, "st", "http://www.srcML.org/srcML/stereotype");
        if (error) {
            std::cerr << "Error registering namespace" << std::endl;
            return -1;
        }

        if (fileList == "" && outputFile != "") {
            error = srcml_archive_write_open_filename(output_archive, outputFile.c_str());
            if (error) {
                std::cerr << "Error opening: " << outputFile << std::endl;
                return -1;
            }
        } else {
            error = srcml_archive_write_open_filename(output_archive, (inputFileList[i] + ".annotated.xml").c_str());
            if (error) {
                std::cerr << "Error opening: " << inputFileList[i] << ".annotated.xml" << std::endl;
                return -1;
            }
        }
        srcml_archive_write_unit(output_archive, firstUnit);
        srcml_archive_write_unit(output_archive, secondUnit);

        srcml_unit_free(firstUnit);
        srcml_unit_free(secondUnit);
        srcml_archive_close(archive);
        srcml_archive_close(output_archive);
        srcml_archive_free(output_archive);
        srcml_archive_free(archive);
        aClass  = classModel();
    }
    
    std::cerr << "StereoCode completed." << std::endl;
    return 0;
}


//
// Read a file of file names for input.
// Each file name is a srcML archive.
std::vector<std::string> readFileNames(const std::string & fileName){
    std::vector<std::string> list_of_archives;
    std::ifstream archives_file;
    archives_file.open(fileName);

    if (archives_file.is_open()){
        std::string name;
        while(std::getline(archives_file, name)){
            list_of_archives.push_back(name);
        }
    } else{
        std::cerr << "Error: File not found: " << fileName << "\n";
    }
    return list_of_archives;
}



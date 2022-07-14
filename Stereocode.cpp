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

primitiveTypes primitives;  //Global set of primitives.


int main(int argc, char const *argv[])
{
    CLI::App app{"StereoCode: Determines method stereotypes"};
    std::string file_list = "none";
    std::string file_name = "none";
    std::string prim_file = "none";
    app.add_option("-l,--list-file",  file_list, "File name that contains a list of srcML archives");
    app.add_option("-a,--archive",    file_name, "File name of a single srcML archive containing hpp and cpp units");
    app.add_option("-p,--primitives", prim_file, "File name user supplied primitive types (one per line)");
    CLI11_PARSE(app, argc, argv);

    std::vector<std::string> file_names_list;
    if (file_name != "none") {
        file_names_list.push_back(file_name);
    }
    else if(file_list != "none") {
        file_names_list = readFileNames(file_list);
    } else {
        std::cerr << "Error, incorrect usage\n";
        std::cerr << "   Options: -a input-filename\n";
        std::cerr << "            -l input-list-filename\n";
        std::cerr << "            -p primitive-types-filename\n";
        //std::cerr << "            -o output-filename\n";
        std::cerr << "   Example: stereocode -a foo.xml \n";
        std::cerr << "   Input:  srcML archive of foo.hpp and foo.cpp\n";
        std::cerr << "   Output: srcML archive foo.annotated.xml - in same path as foo.xml\n";
      return -1;
    }
    if (prim_file != "none") {  //Add any user defined primitive types to initial set
        std::ifstream in(prim_file);
        if (in.is_open())
            in >> primitives;
        else {
            std::cerr << "Error: Primitive types file not found: " << prim_file << std::endl;
            return -1;
        }
        in.close();
    }

    std::cerr << "Computing stereotypes for the following classes: " << std::endl;

    for (int i = 0; i < file_names_list.size(); ++i){
        int error;
        srcml_archive* archive = srcml_archive_create();
        error = srcml_archive_read_open_filename(archive, file_names_list[i].c_str());   // read the srcml archive file
        if (error) {
            std::cerr << "Error: file not found: " << file_names_list[i] << ", error == " << error << "\n";
            return -1;
        }
        srcml_unit* firstUnit = srcml_archive_read_unit(archive);   //.hpp, .java, .cs, etc
        srcml_unit* secondUnit = srcml_archive_read_unit(archive);  //.cpp - only C++ has two units (hpp is first)
        classModel  aClass  = classModel(archive, firstUnit, secondUnit);

        aClass.stereotypeGetter                (archive, firstUnit, secondUnit);
        aClass.stereotypeSetter                (archive, firstUnit, secondUnit);
        aClass.stereotypeCollaborationalCommand(archive, firstUnit, secondUnit);
        aClass.stereotypePredicate             (archive, firstUnit, secondUnit);
        aClass.stereotypeProperty              (archive, firstUnit, secondUnit);
        aClass.stereotypeVoidAccessor          (archive, firstUnit, secondUnit);
        aClass.stereotypeCommand               (archive, firstUnit, secondUnit);
        aClass.stereotypeFactory               (archive, firstUnit, secondUnit);
        aClass.stereotypeEmpty                 (archive, firstUnit, secondUnit);
        aClass.stereotypeCollaborator          (archive, firstUnit, secondUnit);
        aClass.stereotypeStateless             (archive, firstUnit, secondUnit);

        std::ofstream reportFile;
        reportFile.open(file_names_list[i] + ".report.txt");
        aClass.printReportToFile(reportFile, file_names_list[i]);
        reportFile.close();

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
        error = srcml_archive_write_open_filename(output_archive, (file_names_list[i] + ".annotated.xml").c_str());
        if (error) {
            std::cerr << "Error opening " << file_names_list[i] << ".annotated.xml" << std::endl;
            return -1;
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
std::vector<std::string> readFileNames(const std::string & file_name){
    std::vector<std::string> list_of_archives;
    std::ifstream archives_file;
    archives_file.open(file_name);

    if (archives_file.is_open()){
        std::string name;
        while(std::getline(archives_file, name)){
            list_of_archives.push_back(name);
        }
    } else{
        std::cerr << "Error: file not found: " << file_name << "\n";
    }
    return list_of_archives;
}



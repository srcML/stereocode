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

primitiveTypes primitives;  //Global set of primitives.  Need to fix.


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
        std::cerr << "   Input: srcML archive of foo.hpp and foo.cpp\n";
        std::cerr << "   Output: srcML archive foo.annotated.xml - in same path as foo.xml\n";
      return -1;
    }

    if (prim_file != "none") {  //Add user defined primitive types to initial set
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
    std::ofstream output_file;
    output_file.open("stereotypeReport.txt");  //Need to make this variable instead of hard coded.

    for (int i = 0; i < file_names_list.size(); ++i){
        int error;
        srcml_archive* input_archive = srcml_archive_create();

        // read the srcml archive file
        //assumes the hpp file is the first unit
        error = srcml_archive_read_open_filename(input_archive, file_names_list[i].c_str());
        if (error) {
            std::cerr << "Error: file not found: " << file_names_list[i] << ", error == " << error << "\n";
            return -1;
        }

        srcml_unit* hpp_unit = srcml_archive_read_unit(input_archive);
        srcml_unit* cpp_unit = srcml_archive_read_unit(input_archive);

        ClassInfo class_representation = ClassInfo(input_archive, hpp_unit, cpp_unit);

        class_representation.stereotypeGetters               (input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeSetters               (input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeCollaborationalCommand(input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypePredicates            (input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeProperties            (input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeVoidAccessor          (input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeCommand               (input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeFactories             (input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeEmpty                 (input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeCollaborators         (input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeStateless             (input_archive, hpp_unit, cpp_unit);

        class_representation.printReportToFile(output_file, file_names_list[i]);

        std::cerr << "Class name: " << class_representation.getClassName() << std::endl;
        //Output for testing
        //class_representation.print_method_names();
        //class_representation.print_return_types();
        //class_representation.printMethodHeaders();
        //class_representation.printStereotypes();
        //class_representation.printAttributes();
        //class_representation.printReturnTypes();
        
        if (class_representation.getInlineFunctionCount() != 0) {
            hpp_unit = class_representation.writeStereotypeAttribute(input_archive, hpp_unit, true);
        }
        if (class_representation.getOutoflineFunctionCount() != 0) {
            cpp_unit = class_representation.writeStereotypeAttribute(input_archive, cpp_unit, false);
        }

        srcml_archive* output_archive = srcml_archive_create();

        error = srcml_archive_write_open_filename(output_archive, (file_names_list[i] + ".annotated.xml").c_str());
        if (error) {
            std::cerr << "Error opening " << file_names_list[i] << ".annotated.xml" << std::endl;
            return -1;
        }
        srcml_archive_write_unit(output_archive, hpp_unit);
        srcml_archive_write_unit(output_archive, cpp_unit);

        srcml_unit_free(hpp_unit);
        srcml_unit_free(cpp_unit);
        srcml_archive_close(input_archive);
        srcml_archive_close(output_archive);
        srcml_archive_free(output_archive);
        srcml_archive_free(input_archive);
    }
    output_file.close();
    std::cerr << "StereoCode completed." << std::endl;

    return 0;
}

//
//
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



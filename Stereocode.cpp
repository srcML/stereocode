//
// compile with
// c++ Stereocode.cpp -std=c++11 -l srcml -o stereocode.out

// assumptions:
//  given cpp and hpp in 1 archive.
//  only 1 class per file   

#include <iostream>
#include <srcml.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <algorithm>

#include "CLI11.hpp"
#include "ClassInfo.cpp"

std::vector<std::string> readFileNames(const std::string&);


int main(int argc, char const *argv[])
{
    CLI::App app{"App description"};

    std::string file_list = "default";
    std::string file_name = "default";
    app.add_option("-l,--list-file", file_list, "File name that contains a list of archives");
    app.add_option("-a,--archive", file_name,"File name of a single archive containing hpp and cpp units");


    CLI11_PARSE(app, argc, argv);



    std::string base_directory = "stereocode_tests/HippoDraw-1.11.1/";


    std::vector<std::string> file_names_list;
    if (file_name != "default"){
        file_names_list.push_back(file_name);
    }
    else if(file_list != "default"){
        file_names_list = readFileNames(file_list);
    }
    else{
        std::cout <<"please select one of the options, -l or -a\n";
        return -1;
    }
    

    std::ofstream output_file;
    output_file.open("stereotypeReport.txt");

    for (int i = 0; i < file_names_list.size(); ++i){
        std::string file_path = base_directory + file_names_list[i];
        srcml_archive* input_archive = srcml_archive_create();

        // read the srcml archive file.
        //std::cout << "attempting to open file at " << file_path << std::endl;
        int error = srcml_archive_read_open_filename(input_archive, file_path.c_str());
        //assumes the hpp file is the first unit.
        //std::cout << "ERROR OPENING FILE  "<< error << "\n" << std::endl;

        srcml_unit* hpp_unit = srcml_archive_read_unit(input_archive);
        srcml_unit* cpp_unit = srcml_archive_read_unit(input_archive);
        ClassInfo class_representation = ClassInfo(input_archive, hpp_unit, cpp_unit);
        //class_representation.print_method_names();
        //class_representation.print_return_types();
        class_representation.stereotypeGetters(input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeSetters(input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeCollaborationalCommand(input_archive, hpp_unit, cpp_unit);
        
        class_representation.stereotypePredicates(input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeProperties(input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeVoidAccessor(input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeCommand(input_archive, hpp_unit, cpp_unit);

        class_representation.stereotypeFactories(input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeEmpty(input_archive, hpp_unit, cpp_unit);

        class_representation.stereotypeCollaborators(input_archive, hpp_unit, cpp_unit);
        class_representation.stereotypeStateless(input_archive, hpp_unit, cpp_unit);

        class_representation.printReportToFile(output_file, file_names_list[i]);

        std::cout << "CLASS NAME: \n" << class_representation.get_class_name() << std::endl;
        //class_representation.printMethodHeaders();
        //class_representation.printStereotypes();
        class_representation.printAttributes();
        //class_representation.printReturnTypes();
        if (class_representation.get_inline_function_count() != 0){
            hpp_unit = class_representation.writeStereotypeAttribute(input_archive, hpp_unit, true);
        }
        if (class_representation.get_outofline_function_count() != 0){
            cpp_unit = class_representation.writeStereotypeAttribute(input_archive, cpp_unit, false);
        }


        srcml_archive* output_archive = srcml_archive_create();

        error = srcml_archive_write_open_filename(output_archive, (base_directory + "annotated/" + file_names_list[i]).c_str());
        if (error != 0){
            std::cout << "error opening " << base_directory + "annotated/" + file_names_list[i] << std::endl;
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

    return 0;
}

std::vector<std::string> readFileNames(const std::string & file_name){
    std::vector<std::string> list_of_archives;
    std::ifstream archives_file;
    archives_file.open(file_name);

    if (archives_file.is_open()){
        std::string name;

        while(std::getline(archives_file, name)){
            list_of_archives.push_back(name);
        }
    }
    else{
        std::cout << "unable to open file containing archive names\n";
    }
    return list_of_archives;
}

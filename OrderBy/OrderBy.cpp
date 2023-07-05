//FilePair takes your current directory location and traverses through all directories and subdirectories 
//It then returns an ordered list of files with the header(first) file coming before
//its corresponding implementation (second) file (if second file exists)


#include <algorithm>
#include <filesystem> 
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <cstring>
#include <string_view>
#include <vector>
#include <map>
#include <utility>

#include "CLI11.hpp"

//returns index of last index of a string
int findLastIndex (std::string& str, char x) {
    for (int i = str.length() - 1; i >= 0; i--)
        if (str[i] == x)
            return i;
    return -1;
}

//returns filename after root directory, excludes extension
//ex: if root = dir1 and file is in dir1/subdir/file.hpp
//returns: subdir/file
std::string getDirecPath (std::string fPath, std::string root, int lastDot, int lastSlash) {
    std::string fnameReturn = "";
    fPath = fPath.substr(0, lastDot);           //excludes extension
    std::string direc = fPath.substr(lastSlash+1);   //stores base fname initially
    int p1 = lastSlash, p2 = lastSlash;         //sets pointers equal to the slash before base fname
    do {
        fPath = fPath.substr(0, p1);
        p1 = fPath.length()-1;                  //points to last letter of last direc in remaining fpath

        direc += fnameReturn;
        fnameReturn = "/" + direc;

        p2 = findLastIndex(fPath, '/');
        direc = fPath.substr(p2+1, p1-p2);
        p1 = p2;
        
    } while (direc != root);
    return fnameReturn.substr(1);
}


//returns index of start of file extension
std::string getFileExt (std::string& filePath) { 
    const char ch = '.';
    std::string str_obj(filePath);
    char* char_arr;
    char_arr = &str_obj[0];
    char* result = strrchr(char_arr, ch);   //collects substr
    if(result == NULL){
        return "null";
    }else{
        std::string strResult = result;
        return strResult;
    } 
}

bool isFirstFile(std::string exten) {
    const char* c = exten.c_str();
    if (    strcmp(c, ".hpp") == 0
         || strcmp(c, ".h")   == 0
         || strcmp(c, ".hxx") == 0
         || strcmp(c, ".hh")  == 0
         || strcmp(c, ".h++") == 0
         || strcmp(c, ".H")   == 0
         || strcmp(c, ".i")   == 0
         || strcmp(c, ".ii")  == 0
         || strcmp(c, ".tcc") == 0)
    {
        return true;
    } else {
        return false;
    }
}

bool isSecondFile(std::string exten) {
    const char* c = exten.c_str();
    if (    strcmp(c, ".cpp") == 0
         || strcmp(c, ".c")   == 0
         || strcmp(c, ".CPP") == 0
         || strcmp(c, ".cp")  == 0
         || strcmp(c, ".cxx") == 0
         || strcmp(c, ".cc")  == 0
         || strcmp(c, ".c++") == 0
         || strcmp(c, ".C")   == 0) 
    {
        return true;
    } else {
        return false;
    }
}

std::string trimQuotes(std::string str)
{
    const std::string inChar = "\""; //char to search for and remove
    const auto strBegin = str.find_first_not_of(inChar);
    const auto strEnd = str.find_last_not_of(inChar);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

int main(int argc, char const *argv[]) {

    //Create CLI app
    CLI::App app{"pairhc: pairs header and implementation files"};
    
    std::string inputFolder = "";
    std::string outputFile = "";
    std::string warningReport = "";
    bool nonRecursive = false;
    bool quiet = false;
    bool fullPath = false;
    std::vector<std::string> inputFileList; //is this necessary? only one input dir expected?

    app.add_option("input",              inputFolder,    "Folder name containing files to be paired")->required();
    app.add_option("-o,--output",        outputFile,     "File name of output - ordered list of file names");
    app.add_option("-w,--warningReport", warningReport,  "Output optional report file for unpaired .cpp (off by default)");
    app.add_flag  ("-n,--nonrecursive",  nonRecursive,   "Enables nonrecursive traversion of subdirectories (off by default)");
    app.add_flag  ("-q,--quiet",         quiet,          "Disables warnings (warnings on by default)");
    app.add_flag  ("-f,--fullPath",      fullPath,       "Output contains full path of each file (relative path by default)");

    CLI11_PARSE(app, argc, argv);

    //OPEN oFile
    std::ofstream oFile;
    oFile.open(outputFile);
    
    std::map<std::string, std::pair<std::string, std::pair<std::string, std::pair<std::string,std::string>>>> fileNames;
    std::filesystem::path p = "";        //current direc
    std::string strPath = "";            //holds absolute path if necessary
    std::filesystem::path relPath = std::filesystem::current_path(); //holds relPath

    //remove trailing / in file path if exists
    if (inputFolder[inputFolder.length()-1] == '/') {
        inputFolder = inputFolder.substr(0, inputFolder.length()-1);
    }
    //if inputFolder is given, append to filepath
    if (inputFolder != "") {
        if(std::filesystem::exists(inputFolder)) { // If it's valid filepath (absolute or relative)
            p = inputFolder;
            strPath = p.string();
        }
        else { //error catch for invalid fpath
            std::cerr << "Invalid File Path\n";
            return 1;
        }
    }

    std::string root = p.filename().string();    
    std::string fpAddOn = trimQuotes(relPath.string()); //removes "" from either end of filepath

    //recursion on
    if (!nonRecursive) {
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{p}) {
            //converts direc entry into a string
            std::string path1 = dir_entry.path().string();
            std::string exten = getFileExt(path1);
            
            if (exten != "null") {   //if file exten exists
                int lastSlash = findLastIndex(path1, '/');
                int lastDot = findLastIndex(path1, '.');
                
                //handles case: ../.folder/file
                if (lastDot > lastSlash) {
                    std::string direcPath = getDirecPath(path1, root, lastDot, lastSlash); //directory path
                    std::string fname = path1.substr(lastSlash+1, lastDot-lastSlash-1);   //filename
                    
                    //if FIRST file
                    if (isFirstFile(exten)) {       
                        if (fileNames.find(direcPath) != fileNames.end()) {         //if direcPath exists in map already 
                            if (fileNames[direcPath].second.second.first.empty()) { //places fname, direcPath and 1st file ext in map
                                fileNames.insert_or_assign(direcPath, std::make_pair(fname, std::make_pair(fileNames[direcPath].second.first, std::make_pair(exten, fileNames[direcPath].second.second.second))));
                            } 
                        } else {    //direcPath not in map
                            fileNames.insert_or_assign(direcPath, std::make_pair(fname, std::make_pair("", std::make_pair(exten, ""))));
                        }
                    //if SECOND file
                    } else if (isSecondFile(exten)) {
                        if (fileNames.find(direcPath) != fileNames.end()) {
                            if (fileNames[direcPath].second.second.second.empty()) { //places fname, direcPath and 2nd file ext in map
                                fileNames.insert_or_assign(direcPath, std::make_pair(fileNames[direcPath].first, std::make_pair(fname, std::make_pair(fileNames[direcPath].second.second.first, exten))));
                            }
                        } else {    //direcPath not in map
                            fileNames.insert_or_assign(direcPath, std::make_pair("", std::make_pair(fname, std::make_pair("", exten))));
                        }
                    }
                }
            }
        }
    //recursion off
    } else {
        for (auto const& dir_entry : std::filesystem::directory_iterator{p}) {
            //converts direc entry into a string
            std::string path1 = dir_entry.path().string();
            std::string exten = getFileExt(path1);
            
            if (exten != "null") {   //if file exten exists
                int lastSlash = findLastIndex(path1, '/');
                int lastDot = findLastIndex(path1, '.');
                
                //handles case: ../.folder/file
                if (lastDot > lastSlash) {
                    std::string direcPath = getDirecPath(path1, root, lastDot, lastSlash); //directory path
                    std::string fname = path1.substr(lastSlash+1, lastDot-lastSlash-1);   //filename
                    
                    //if FIRST file
                    if (isFirstFile(exten)) {       
                        if (fileNames.find(direcPath) != fileNames.end()) {         //if direcPath exists in map already 
                            if (fileNames[direcPath].second.second.first.empty()) { //places fname, direcPath and 1st file ext in map
                                fileNames.insert_or_assign(direcPath, std::make_pair(fname, std::make_pair(fileNames[direcPath].second.first, std::make_pair(exten, fileNames[direcPath].second.second.second))));
                            } 
                        } else {    //direcPath not in map
                            fileNames.insert_or_assign(direcPath, std::make_pair(fname, std::make_pair("", std::make_pair(exten, ""))));
                        }
                    //if SECOND file
                    } else if (isSecondFile(exten)) {
                        if (fileNames.find(direcPath) != fileNames.end()) {
                            if (fileNames[direcPath].second.second.second.empty()) { //places fname, direcPath and 2nd file ext in map
                                fileNames.insert_or_assign(direcPath, std::make_pair(fileNames[direcPath].first, std::make_pair(fname, std::make_pair(fileNames[direcPath].second.second.first, exten))));
                            }
                        } else {    //direcPath not in map
                            fileNames.insert_or_assign(direcPath, std::make_pair("", std::make_pair(fname, std::make_pair("", exten))));
                        }
                    }
                }
            }
        }
    }

    //prepares output for paired and unpaired files
    std::vector<std::string> unpairedFiles;
    for (std::map<std::string, std::pair<std::string, std::pair<std::string, std::pair<std::string, std::string>>>>::iterator itr = fileNames.begin(); itr != fileNames.end(); ++itr) {  
       //prints only if first file exists for file name
       if ((*itr).second.second.second.first != "") {
            if(fullPath) {
                if (outputFile == "") {
                    std::cout << strPath << "/" << (*itr).first << (*itr).second.second.second.first << std::endl;
                } else {
                    oFile << strPath << "/" << (*itr).first << (*itr).second.second.second.first << std::endl;
                } 
            } else {
                if (outputFile == "") {
                    std::cout << (*itr).first << (*itr).second.second.second.first << std::endl;
                } else {
                    oFile << (*itr).first << (*itr).second.second.second.first << std::endl; 
                }
            }   
        }
        //only prints if second file exists for a fname AND if second file has corresponding first file
        if ((*itr).second.second.second.second != "" && (*itr).second.second.second.first != "") {
            if(fullPath) {
                if (outputFile == "") {
                    std::cout << strPath << "/" << (*itr).first << (*itr).second.second.second.second << std::endl;
                } else {
                    oFile << strPath << "/" <<  (*itr).first << (*itr).second.second.second.second << std::endl; 
                }
            } else {
                if (outputFile == "") {
                    std::cout << (*itr).first << (*itr).second.second.second.second << std::endl;
                } else {
                    oFile << (*itr).first << (*itr).second.second.second.second << std::endl; 
                }
            }
        }
        //takes unpaired second files and places them in vector
        if ((*itr).second.second.second.second != "" && (*itr).second.second.second.first == "") {
            if(fullPath) {
                unpairedFiles.push_back(strPath + "/" + (*itr).first + (*itr).second.second.second.second); 
            } else {
                unpairedFiles.push_back((*itr).first + (*itr).second.second.second.second);
            }    
        }
    }


    //outputs unpaired second files to terminal
    if (!quiet) {
        if (!unpairedFiles.empty()) {
            std::cout << "\nWARNING: these second files have no corresponding first file:\n";
        }
        for (int i=0; i < unpairedFiles.size(); ++i) {
            std::cout << unpairedFiles.at(i) << std::endl;
        }
    }

    std::ofstream errorFile;
    std::string unpairedTxt;

    //places unpaired files in txt file if -w
    if (warningReport != "") {
        unpairedTxt = warningReport;
        errorFile.open(unpairedTxt);
        if (unpairedFiles.empty()) {
            errorFile << "All files have a pair, no files without a pair";
        } else {
           for (int i=0; i < unpairedFiles.size(); ++i) {
                errorFile << unpairedFiles.at(i) << std::endl;
            } 
        }
    }
    oFile.close();
}

// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModelCollection.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef CLASSMODELCOLLECTION_HPP
#define CLASSMODELCOLLECTION_HPP

#include <thread>
#include <iomanip> 
#include <mutex>
#include <filesystem>
#include "ClassModel.hpp"

class classModelCollection {
public:
                         classModelCollection       (srcml_archive*, srcml_archive*, std::vector<srcml_unit*>&, 
                                                    const std::string&, const std::string&, bool, bool, bool);

    void                 findClassInfo              (srcml_archive*, std::vector<srcml_unit*>);
    void                 findFreeFunction           (srcml_archive*, std::vector<srcml_unit*>);
    void                 findInheritedAttribute     (classModel&);
    void                 findInheritedMethod        (classModel&);
    void                 buildCallGraph             (std::unordered_map<std::string, methodModel>&, methodModel&);
    void                 findAttrModifiedInsideCalls(classModel&, methodModel&);

    void                 outputWithStereotypes      (srcml_unit*, std::map<int, srcml_unit*>&,
                                                     int, const std::unordered_map<std::string, std::string>&,  
                                                     std::unordered_map<int, srcml_transform_result*>&, std::mutex&);
    void                 outputAsComments           (srcml_unit*, srcml_archive*) ;                            
    void                 outputTxtReportFile        (std::stringstream&, classModel&);
    void                 outputCsvReportFile        (std::ofstream&, classModel&);
    void                 outputCsvVerboseReportFile (const std::string&);

    bool                 isFriendFunction           (methodModel&);
    void                 computeFreeFunctionsStereotypes();
    
private:
    std::unordered_map<std::string, classModel>     classCollection;    // List of class names and their models
    std::unordered_map<std::string, std::string>    classGeneric;       // List of generic class names with and without <> for inheritance matching
    std::vector<methodModel>                        freeFunctions;       // List of free functions
};

#endif

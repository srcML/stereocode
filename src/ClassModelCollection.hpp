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
                         classModelCollection       (srcml_archive*, srcml_archive*, const std::string&, const std::string&, bool, bool, bool);

    void                 findClassInfo              (srcml_archive*, srcml_unit*, int);
    void                 findFreeFunctions              (srcml_archive*, srcml_unit*, int);
    void                 findInheritedFields            (classModel&);
    void                 findInheritedMethods           (classModel&);

    void                 outputWithStereotypes          (srcml_unit*, std::map<int, srcml_unit*>&,
                                                         int, const std::unordered_map<std::string, std::string>&,  
                                                         std::unordered_map<int, srcml_transform_result*>&, std::mutex&);
    void                 outputAsComments               (srcml_unit*, srcml_archive*) ;                            
    void                 outputTxtReportFile            (std::stringstream&, classModel*);
    void                 outputCsvReportFile            (std::ofstream&, classModel*);
    void                 outputCsvVerboseReportFile     (const std::string&);

    bool                 isFriendFunction               (methodModel&);
    void                 computeFreeFunctionsStereotypes();
    void                 analyzeFreeFunctions();
    
private:
    std::unordered_map<std::string, classModel>     classCollection;    // List of class names and their models
    std::unordered_map<std::string, std::string>        classGenerics;      // List of generic class names with and without <> for inheritance matching
    std::vector<methodModel>                            freeFunctions;          // List of free functions
};

#endif

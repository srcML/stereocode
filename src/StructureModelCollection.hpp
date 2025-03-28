// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file StructureModelCollection.hpp
 *
 * @copyright Copyright (C) 2021-2024 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef STRUCTUREMODELCOLLECTION_HPP
#define STRUCTUREMODELCOLLECTION_HPP

#include <thread>
#include <iomanip> 
#include <mutex>
#include <filesystem>
#include "StructureModel.hpp"

class structureModelCollection {
public:
                         structureModelCollection       (srcml_archive*, srcml_archive*, const std::string&, const std::string&, bool, bool, bool);

    void                 findStructureInfo              (srcml_archive*, srcml_unit*, int);
    void                 findFreeFunctions              (srcml_archive*, srcml_unit*, int);
    void                 findInheritedFields            (structureModel&);
    void                 findInheritedMethods           (structureModel&);

    void                 outputWithStereotypes          (srcml_unit*, std::map<int, srcml_unit*>&,
                                                         int, const std::unordered_map<std::string, std::string>&,  
                                                         std::unordered_map<int, srcml_transform_result*>&, std::mutex&);
    void                 outputAsComments               (srcml_unit*, srcml_archive*) ;                            
    void                 outputTxtReportFile            (std::stringstream&, structureModel*);
    void                 outputCsvReportFile            (std::ofstream&, structureModel*);
    void                 outputCsvVerboseReportFile     (const std::string&);

    bool                 isFriendFunction               (methodModel&);
    void                 computeFreeFunctionsStereotypes();
    void                 analyzeFreeFunctions();
    
private:
    std::unordered_map<std::string, structureModel>     structureCollection;    // List of structure names and their models
    std::unordered_map<std::string, std::string>        structureGenerics;      // List of generic structure names with and without <> for inheritance matching
    std::vector<methodModel>                            freeFunctions;          // List of free functions
};

#endif

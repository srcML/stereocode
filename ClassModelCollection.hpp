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

#include "ClassModel.hpp"
#include <thread>
#include <iomanip> 

class classModelCollection {
public:
                         classModelCollection       (srcml_archive*, srcml_archive*, std::vector<srcml_unit*>&, 
                                                    const std::string&, bool, bool);

    void                 findClassInfo              (srcml_archive*, std::vector<srcml_unit*>);
    void                 findFreeFunction           (srcml_archive*, std::vector<srcml_unit*>);
    void                 findInheritedAttribute     (classModel&);
    void                 findInheritedMethod        (classModel&);

    void                 findAttrModifiedInsideCalls  (classModel&, methodModel&);

    void                 outputWithStereotypes      (srcml_unit*, std::map<int, srcml_unit*>&,
                                                     int, const std::unordered_map<std::string, std::string>&,  std::unordered_map<int, srcml_transform_result*>&);
                                                     
    void                 outputReportFile           (std::stringstream&, classModel&);
    void                 allView                    (std::ofstream&, classModel&);
    void                 method_class_unique_views  (std::ofstream&, std::ofstream&, std::ofstream&, std::ofstream&, std::ofstream&);

    bool                 isFriendFunction            (methodModel&);
    
private:
    std::unordered_map<std::string, classModel>     classCollection;    // List of class names and their models
    std::unordered_map<std::string, std::string>    classGeneric;       // List of generic class names with and without <> for inheritance matching
    std::vector<methodModel>                        freeFunction;       // List of free functions
    std::unordered_map<std::string, int>            uniqueMethodStereotypesView;  
    std::unordered_map<std::string, int>            uniqueClassStereotypesView;  

    std::unordered_map<std::string, int> classStereotypes = {
        {"entity", 0},
        {"minimal-entity", 0},
        {"data-provider", 0},
        {"command", 0},
        {"boundary", 0},
        {"factory", 0},
        {"control", 0},
        {"pure-control", 0},
        {"large-class", 0},
        {"lazy-class", 0},
        {"degenerate", 0},
        {"data-class", 0},
        {"small-class", 0},
        {"unclassified", 0},
        {"empty", 0},
    };
    std::unordered_map<std::string, int> methodStereotypes = {
        {"get", 0},
        {"predicate", 0},
        {"property", 0},
        {"void-accessor", 0},
        {"set", 0},
        {"command", 0},
        {"non-void-command", 0},
        {"collaborator", 0},
        {"controller", 0},
        {"constructor", 0},
        {"copy-constructor", 0},
        {"destructor", 0},
        {"factory", 0},
        {"empty", 0},
        {"stateless", 0},
        {"wrapper", 0},
        {"unclassified", 0},
    };
};

#endif

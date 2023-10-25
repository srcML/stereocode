// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file ClassModelCollection.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */


#ifndef CLASSMODELCOLLECTION_HPP
#define CLASSMODELCOLLECTION_HPP

#include "ClassModel.hpp"
#include <unordered_map>
#include <iomanip> 


class classModelCollection{
public:
                         classModelCollection   () : classCollection(), freeFunction() {}
                         classModelCollection   (srcml_archive*, std::vector<srcml_unit*>);

    void                 findClassInfo          (srcml_archive*, std::vector<srcml_unit*>);
    void                 findFreeFunction       (srcml_archive*, std::vector<srcml_unit*>);
    void                 findInheritedAttribute ();
    bool                 isFriendFunction       (srcml_archive*, srcml_unit*, methodModel&, std::string);

    void                 outputWithStereotypes  (srcml_archive*, srcml_archive*, std::vector<srcml_unit*>);
    void                 outputReport           (std::ofstream&);
    void                 outputCSV              (std::string);
protected:
    std::unordered_map<std::string, classModel>     classCollection; // List of classes and their methods
    std::vector<std::string>                        freeFunction;    // List of free functions
};

#endif

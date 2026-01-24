// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotypes_analyzer.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef STEREOTYPES_ANALYZER_HPP
#define STEREOTYPES_ANALYZER_HPP

#include <map>
#include <mutex>

#include "type_model.hpp"

class stereotypesAnalyzer {
public:
                         stereotypesAnalyzer            (const std::string&, const std::string&);
private:
    struct ThreadResult {
        std::unordered_map<std::string, typeModel>         types;             // List of type names and their models
        std::vector<typeModel>                             duplicateTypes;    // List of types with duplicate names (e.g., partial classes in C#)
        std::unordered_map<std::string, std::string>       genericTypes;      // List of type names with and without generic parameter lists ( <> ) for inheritance matching
        std::vector<functionModel>                         freeFunctions;     // List of free functions

        void clear() {
            types.clear();
            duplicateTypes.clear();
            genericTypes.clear();
            freeFunctions.clear();
        }
    };

    std::unordered_map<std::string, typeModel>         types;             // List of type names and their models
    std::unordered_map<std::string, std::string>       genericTypes;      // List of type names with and without generic parameter lists ( <> ) for inheritance matching
    std::vector<functionModel>                         freeFunctions;     // List of free functions

    bool                 isFriendFunction               (typeModel&);
    void                 findInheritedDataMembers       (typeModel&);
    void                 findInheritedMethods           (typeModel&);
    void                 analyzeFreeFunctions();
    void                 outputStereotypes              (srcml_unit*, int, std::map<int, srcml_unit*>&, const std::unordered_map<std::string, std::string>&, std::unordered_map<int, srcml_transform_result*>&, std::mutex&);
    void                 outputStereotypesAsComments    (srcml_unit*) ;                            
    void                 outputStereotypesAsCSV         (std::ofstream&, typeModel*, bool);
    void                 analyzeWorker                  (srcml_unit*, int, ThreadResult&, std::mutex&);
    void                 outputWorker                   (srcml_unit*, int, std::map<int, srcml_unit*>&, const std::unordered_map<std::string, std::string>&, std::unordered_map<int, srcml_transform_result*>&, std::mutex&);
    void                 accumulateResults              (std::vector<ThreadResult>&, int);

    void                 findTypeInfo                   (srcml_unit*, int, ThreadResult&, std::mutex&);
    void                 findFreeFunctions              (srcml_unit*, int, ThreadResult&, std::mutex&);
};

#endif

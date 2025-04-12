// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotypes.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

 #ifndef STEREOTYPES_HPP
 #define STEREOTYPES_HPP

 #include "ClassModel.hpp"
 
 class stereotypes {
 public:
    void computeMethodStereotypes         (std::unordered_map<std::string, classModel>&);
    void computeClassStereotypes          (std::unordered_map<std::string, classModel>&);
    void computeFreeFunctionsStereotypes  (std::vector<methodModel>&                   );

 }; 
 
 #endif
 
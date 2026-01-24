// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file stereotype_rules.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

 #ifndef STEREOTYPE_RULES_HPP
 #define STEREOTYPE_RULES_HPP

 #include "type_model.hpp"
 
 class stereotypeRules {
 public:
    void computeMethodStereotypes         (std::unordered_map<std::string, typeModel>&);
    void computeTypeStereotypes          (std::unordered_map<std::string, typeModel>&);
    void computeFreeFunctionStereotypes  (std::vector<functionModel>&                );
 }; 
 
 #endif
 
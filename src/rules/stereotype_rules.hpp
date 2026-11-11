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

 #include "model/called_methods.hpp"
 #include "rules/stereotype_classifier.hpp"
 #include "model/type_model.hpp"

 #include <optional>
 #include <vector>
 
 class stereotypeRules {
 public:
    explicit stereotypeRules              (std::optional<int>);
    void computeMethodStereotypes         (std::vector<typeModel>&, const calledMethods&);
    void computeTypeStereotypes          (const std::vector<typeModel*>&             );
    void computeFreeFunctionStereotypes  (std::vector<functionModel>&                );
    void outputLargeClassThreshold       () const;
 private:
    std::optional<int>  largeClassThreshold;  // Method threshold of -l for the large type stereotype (e.g., large-class)
    methodCountSummary  methodSummary;        // Methods per class of the run, or per labeled type without a class, which give the threshold without -l
    bool                thresholdOverClasses{false};  // The run has a class, so methodSummary counts the classes alone
 }; 
 
 #endif
 
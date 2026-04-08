// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file call_model.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef CALL_MODEL_HPP
#define CALL_MODEL_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class callModel {
public:
    std::vector<std::string>&               getName                     ()                                 { return name;                    } 
    const std::string&                      getArgumentList             () const                           { return argumentList;            }
    const std::string&                      getSignature                () const                           { return signature;               }

    void                                    setName                     (const std::string& name_)         { name.push_back(name_);          }
    void                                    setArgumentList             (const std::string& argumentList_) { argumentList = argumentList_;   }
    void                                    setSignature                (const std::string& signature_)    { signature = signature_;         }

    friend std::istream&                    operator>>                  (std::istream&, callModel&);
private:
    std::vector<std::string>   name;                    // Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and and in-between generic in ( <> ) | same as last but without ( <> )
    std::string                argumentList;            // Argument list
    std::string                signature;               // Call signature
    bool                       internal{false};         // Whether it is a call to an internal method or an external
};

#endif

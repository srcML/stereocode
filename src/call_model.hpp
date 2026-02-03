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

class callModel {
public:
    const std::string&      getName                     () const                           { return name;          } 
    const std::string&      getArgumentList             () const                           { return argumentList;  }
    const std::string&      getSignature                () const                           { return signature;     }

    void                    setName                     (const std::string& name_)         { name = name_;                 }
    void                    setArgumentList             (const std::string& argumentList_) { argumentList = argumentList_; }
    void                    setSignature                (const std::string& signature_)    { signature = signature_;       }

    friend std::istream&    operator>>                  (std::istream&, callModel&);
private:
    std::string             name;                    // Call name
    std::string             argumentList;            // Argument list
    std::string             signature;               // Call signature
    bool                    internal{false};         // Whether it is a call to an internal method or an external
};

#endif

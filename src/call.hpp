// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file call.hpp
 *
 * @copyright Copyright (C) 2021-2025 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef CALL_HPP
#define CALL_HPP

#include <string>
#include <set>

class call {
public:
    const std::string&     getName            () const               { return name;          } 
    const std::string&     getArgumentList    () const               { return argumentList;  }
    const std::string&     getSignature       () const               { return signature;     }

    void                   setName            (const std::string& n) { name = n;             }
    void                   setArgumentList    (const std::string& l) { argumentList = l;     }
    void                   setSignature       (const std::string& s) { signature = s;        }

private:
    std::string       name;
    std::string       argumentList;
    std::string       signature;                   
};

#endif

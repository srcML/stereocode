// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file AttributeInfo.hpp
 *
 * @copyright Copyright (C) 2021-2023 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef ATTRIBUTEINFO_HPP
#define ATTRIBUTEINFO_HPP

#include <iostream>
#include <string>

class AttributeInfo {
public:
                AttributeInfo   () : name(), type(), modified(false), nonPrimitive(false) {}
                AttributeInfo   (const std::string& n) : AttributeInfo()                       { name = n;           }
                AttributeInfo   (const std::string& n, const std::string& t) : AttributeInfo() { name = n; type = t; }
    void        setName         (const std::string& n)                                         { name = n;           }
    void        setType         (const std::string& t)                                         { type = t;           }
    void        setModified     (const bool m)                                                 { modified = m;       }
    void        setNonPrimitive (const bool m)                                                 { nonPrimitive = m;   }
    std::string getName         () const                                                       { return name;        }
    std::string getType         () const                                                       { return type;        }
    bool        getModified     () const                                                       { return modified;    }
    bool        getNonPrimitive () const                                                       { return nonPrimitive;}
private:
    std::string name;
    std::string type;
    bool modified;
    bool nonPrimitive; // True if attribute is non-primitive
};

std::ostream& operator<<(std::ostream&, const AttributeInfo&);

#endif

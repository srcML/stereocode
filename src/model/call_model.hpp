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
    std::vector<std::string>&               getNameVector               ()                                 { return name;                    } 
    const std::vector<std::string>&         getName                     () const                           { return name;                    } 
    const std::string&                      getSignature                () const                           { return signature;               }
    const std::string&                      getReceiverType             () const                           { return receiverType;            }
    int                                     getReceiverUnit             () const                           { return receiverUnit;            }
    int                                     getPosition                 () const                           { return position;                }
    int                                     getArgumentCount            () const                           { return argumentCount;           }
    bool                                    isResultWritten             () const                           { return resultWritten;           }
    bool                                    isThroughPointer            () const                           { return throughPointer;          }

    void                                    setName                     (const std::string& name_)         { name.push_back(name_);          }
    void                                    setSignature                (const std::string& signature_)    { signature = signature_;         }
    void                                    setReceiverType             (const std::string& type)          { receiverType = type;            }
    void                                    setReceiverUnit             (int unitNumber)                   { receiverUnit = unitNumber;      }
    void                                    setThroughPointer           (bool through)                     { throughPointer = through;       }
    void                                    setPosition                 (int position_)                    { position = position_;           }
    void                                    setArgumentCount            (int count)                        { argumentCount = count;          }
    void                                    setResultWritten            (bool written)                     { resultWritten = written;        }

    friend std::istream&                    operator>>                  (std::istream&, callModel&);
private:
    std::vector<std::string>   name;                    // Size = 4 containing: Original name | name without whitespaces | name without whitespaces, namespaces, and and in-between generic in ( <> ) | same as last but without ( <> )
    std::string                signature;               // Call signature
    std::string                receiverType;            // Named type of the object whose method the call names directly on a data member, such as Engine of m_engine.rpm(), or of m_engine->rpm() for a std::unique_ptr<Engine>
    int                        receiverUnit{0};         // Unit that declares the data member, where the name of its type is written
    int                        position{0};             // Position of the name of the call in its function, in document order
    int                        argumentCount{0};        // Number of arguments that the call passes
    bool                       resultWritten{false};    // The expression writes through the result of the call, as v.at(i) = x does
    bool                       throughPointer{false};   // The call names its method through -> on a data member of a type that is no pointer, as of an alias of a smart pointer
    bool                       internal{false};         // Whether it is a call to an internal method or an external
};

#endif

// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file specifier_pattern.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef SPECIFIER_PATTERN_HPP
#define SPECIFIER_PATTERN_HPP

#include <array>
#include <cstddef>
#include <string>
#include <vector>

// The specifiers of one language as one pattern that finds each specifier in a type
// A default specifier is written with the escapes of a regular expression, such as const, &&, \*, @\w+, or IList(?=<),
// and a specifier from a user file is plain text, such as * or [[nodiscard]]
//
class specifierPattern {
public:
                                                    specifierPattern    (const std::vector<std::string>&, const std::vector<std::string>&);

    std::string                                     replace             (const std::string&) const;
    std::vector<std::string>                        split               (const std::string&) const;
    bool                                            finds               (const std::string&) const;
private:
    // A specifier that matches at the start of its text
    struct literalSpecifier {
        std::string                                 text;               // Characters at the start of a match
        bool                                        wholeWord{false};   // No word character comes before or after the match
        bool                                        wordRun{false};     // One or more word characters follow the text
        bool                                        genericOnly{false}; // A generic argument list follows the text, which the match leaves out
    };

    static literalSpecifier                         readDefault         (const std::string&);
    static literalSpecifier                         readUserSpecifier   (const std::string&);
    void                                            add                 (literalSpecifier);
    std::size_t                                     matchEnd            (const std::string&, std::size_t) const;

    std::vector<literalSpecifier>                   specifiers;         // In the order of the alternatives of the pattern
    std::array<std::vector<std::size_t>, 256>       byFirstCharacter;   // Positions in specifiers by the first character of their text
};

#endif

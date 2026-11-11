// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file interpolated_strings.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "model/interpolated_strings.hpp"
#include "archive/node_text.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <functional>
#include <utility>

namespace {
    // Tells whether a character can start a name, which may hold letters beyond ASCII
    //
    bool startsName(char c) {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '_' || static_cast<unsigned char>(c) >= 0x80;
    }

    // Tells whether a character can continue a name
    //
    bool continuesName(char c) {
        return startsName(c) || std::isdigit(static_cast<unsigned char>(c));
    }

    // Reads C# code from a text, which a source of more text extends where the code needs it, and collects the names that the holes
    // of its interpolated strings use
    //
    class holeScanner {
    public:
                                    holeScanner     (std::string text_, std::function<bool(std::string&)> more_) : text{std::move(text_)}, more{std::move(more_)} {}

        std::size_t                 interpolated    (std::size_t);
        bool                        ranOut          () const { return textEnded; }      // The text ends inside a string or a hole
        std::vector<std::string>&   names           ()       { return holeNames; }
    private:
        bool                        has             (std::size_t);
        char                        at              (std::size_t position) { return has(position) ? text[position] : '\0'; }
        bool                        startsNameAt    (std::size_t position) { return startsName(at(position)) || (at(position) == '@' && startsName(at(position + 1))); }
        std::size_t                 runOf           (std::size_t, char);
        std::size_t                 literal         (std::size_t);
        std::size_t                 quoted          (std::size_t, bool);
        std::size_t                 hole            (std::size_t, std::size_t);
        std::size_t                 chain           (std::size_t, bool);
        std::size_t                 spaces          (std::size_t);
        std::size_t                 ended           ();

        std::string                         text;
        std::function<bool(std::string&)>   more;           // Appends more text and tells whether there was more, if the text can grow
        std::vector<std::string>            holeNames;
        bool                                textEnded{false};
    };

    // Tells whether the text reaches a position, after the source of more text extends the text where needed
    //
    bool holeScanner::has(std::size_t position) {
        while (position >= text.size() && more && more(text)) {}
        return position < text.size();
    }

    // Counts the characters c from a position
    //
    std::size_t holeScanner::runOf(std::size_t position, char c) {
        std::size_t end = position;
        while (at(end) == c) ++end;
        return end - position;
    }

    // Gets the end of the interpolated string whose prefix, such as $, $@, @$, or $$, starts at a position, and collects the names of its holes
    // A raw string, such as $"""...""", opens a hole with as many braces as its prefix has $ signs, while another string doubles a brace of its text
    //
    std::size_t holeScanner::interpolated(std::size_t position) {
        std::size_t dollars = 0;
        bool verbatim = false;
        for (; at(position) == '$' || at(position) == '@'; ++position) {
            if (at(position) == '$') ++dollars;
            else                     verbatim = true;
        }
        const std::size_t quotes = runOf(position, '"');
        if (quotes == 0) return position;

        const bool raw = !verbatim && quotes >= 3;
        const std::size_t delimiter = raw ? quotes : 1;
        const std::size_t braces    = raw && dollars > 1 ? dollars : 1;
        for (position += delimiter; has(position);) {
            const char c = text[position];
            if (c == '"') {
                const std::size_t run = runOf(position, '"');
                if (raw && run < delimiter)             position += run;
                else if (!raw && verbatim && run >= 2)  position += 2;
                else                                    return position + delimiter;
            }
            else if (c == '\\' && !raw && !verbatim) {
                position += 2;
            }
            else if (c == '{') {
                const std::size_t run = runOf(position, '{');
                if (!raw && run >= 2)   position += 2;
                else if (run < braces)  position += run;
                else                    position = hole(position + run, braces);
            }
            else {
                ++position;
            }
        }
        return ended();
    }

    // Gets the end of a hole whose expression starts at a position, after the braces that close the hole, and collects the names of the expression
    // A comma or a colon outside brackets starts the alignment or the format of the hole, which hold no names
    //
    std::size_t holeScanner::hole(std::size_t position, std::size_t braces) {
        int depth = 0;
        bool format = false;
        bool afterAccess = false;   // The code before a name ends with an access operator, as f(). does before Name
        while (has(position)) {
            const char c = text[position];
            if (c == '}' && depth == 0 && runOf(position, '}') >= braces) return position + braces;

            if (format || std::isspace(static_cast<unsigned char>(c))) {
                ++position;
            }
            else if (c == '"' || c == '\'' || c == '$' || (c == '@' && (at(position + 1) == '"' || at(position + 1) == '$'))) {
                position = literal(position);
                afterAccess = false;
            }
            else if (startsNameAt(position)) {
                position = chain(position, afterAccess);
                afterAccess = false;
            }
            else if (std::isdigit(static_cast<unsigned char>(c))) {
                while (continuesName(at(position)) || at(position) == '.') ++position;
                afterAccess = false;
            }
            else {
                if (c == '(' || c == '[' || c == '{')                       ++depth;
                else if ((c == ')' || c == ']' || c == '}') && depth > 0)   --depth;
                else if ((c == ',' || c == ':') && depth == 0)              format = true;
                afterAccess = c == '.';
                ++position;
            }
        }
        return ended();
    }

    // Gets the end of the character literal or the string that starts at a position, and collects the names of the holes of an interpolated string
    //
    std::size_t holeScanner::literal(std::size_t position) {
        if (at(position) == '$' || (at(position) == '@' && at(position + 1) == '$')) return interpolated(position);
        if (at(position) == '@') return quoted(position + 1, true);
        return quoted(position, false);
    }

    // Gets the end of a character literal or of a regular, verbatim, or raw string without holes that starts at its quote, such as 'a', "a", or """a"""
    //
    std::size_t holeScanner::quoted(std::size_t position, bool verbatim) {
        const char quote = at(position);
        const std::size_t quotes = runOf(position, quote);
        if (quote == '"' && !verbatim && quotes >= 3) {
            for (position += quotes; has(position);) {
                const std::size_t run = runOf(position, '"');
                if (run >= quotes) return position + quotes;
                position += run > 0 ? run : 1;
            }
            return ended();
        }

        for (++position; has(position);) {
            if (text[position] == '\\' && !verbatim)            position += 2;
            else if (text[position] != quote)                   ++position;
            else if (verbatim && at(position + 1) == quote)     position += 2;
            else                                                return position + 1;
        }
        return ended();
    }

    // Gets the end of a chain of names that access operators join, such as this.items.Count, and collects the chain as a name of its hole
    // A call uses only its object, as items of items.Clear(), while a call through this or base and a member of a result, as Name of f().Name, use none
    //
    std::size_t holeScanner::chain(std::size_t position, bool member) {
        std::string name;
        std::size_t objectLength = 0;
        for (;;) {
            const std::size_t start = position;
            if (at(position) == '@') ++position;
            while (continuesName(at(position))) ++position;
            name.append(text.substr(start, position - start));

            const std::size_t next = spaces(position);
            std::size_t access = 0;
            if (at(next) == '.' && at(next + 1) != '.')                                              access = 1;
            else if ((at(next) == '?' && at(next + 1) == '.') || (at(next) == '-' && at(next + 1) == '>')) access = 2;
            const std::size_t following = spaces(next + access);
            if (access == 0 || !startsNameAt(following)) break;

            objectLength = name.size();
            name.append(text.substr(next, access));
            position = following;
        }
        if (member) return position;

        if (at(spaces(position)) == '(') {
            name.erase(objectLength);
            if (name == "this" || name == "base") name.clear();
        }
        if (!name.empty()) holeNames.push_back(std::move(name));
        return position;
    }

    // Gets the position after the white space at a position
    //
    std::size_t holeScanner::spaces(std::size_t position) {
        while (std::isspace(static_cast<unsigned char>(at(position)))) ++position;
        return position;
    }

    // Notes that the text ends inside a string or a hole, and gets the end of the text
    //
    std::size_t holeScanner::ended() {
        textEnded = true;
        return text.size();
    }

    // Gets the text node or the element without children below a node that holds the character at a position of the text of the node
    //
    const xmlNode* leafAt(const xmlNode* node, std::size_t position) {
        while (node->type == XML_ELEMENT_NODE && node->children) {
            const xmlNode* child = node->children;
            for (std::size_t length = sourceOf(child).size(); position >= length && child->next; length = sourceOf(child).size()) {
                position -= length;
                child = child->next;
            }
            node = child;
        }
        return node;
    }
}

// Reads the interpolated string that starts at the name of its prefix, such as $ of $"{count}", from the text of the nodes from the name on
// srcML 1.2 splits a string at each quote in a hole, which can end the expression around it, as in $"{Join(", ", items)}", and reads a raw string as code
//
interpolatedString readInterpolatedString(const xmlNode* start) {
    std::vector<std::pair<std::size_t, const xmlNode*>> pieces{{0, start}};    // Position of the text of each node in the text of the string
    const xmlNode* node = start;
    auto more = [&pieces, &node](std::string& text) {
        while (node && !node->next) node = node->parent;
        if (!node) return false;
        node = node->next;
        pieces.emplace_back(text.size(), node);
        text += sourceOf(node);
        return true;
    };

    holeScanner scanner(sourceOf(start), more);
    const std::size_t end = scanner.interpolated(0);

    interpolatedString string;
    string.names = std::move(scanner.names());
    if (scanner.ranOut()) return string;

    // The last node whose text starts before the closing quote holds the quote
    const auto closing = std::find_if(pieces.rbegin(), pieces.rend(), [end](const auto& piece) { return piece.first < end; });
    string.closingNode = leafAt(closing->second, end - 1 - closing->first);
    return string;
}

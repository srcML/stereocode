// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file annotated_unit.hpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#ifndef ANNOTATED_UNIT_HPP
#define ANNOTATED_UNIT_HPP

#include "archive/srcml_handles.hpp"
#include "output/stereotype_attributes.hpp"

#include <memory>
#include <string>
#include <vector>

// A unit of the input archive with the stereotype attributes of its labeled elements, and on request their stereotype comments,
// ready for the output archive
// Any thread may annotate a unit after the thread that reads the archive has read the srcML of the unit
//
class annotatedUnit {
public:
                        annotatedUnit   (unitPtr, const std::vector<stereotypeAnnotation>&, bool);

    srcml_unit*         get             () const;
private:
    unitPtr             input;                  // Unit as read from the input archive
    transformResultPtr  annotated;              // Unit with the attributes from a transform of libsrcml, or null
    // On the heap, since the reader thread of libsrcml reads the srcML until its archive is freed, also after the unit moves
    std::unique_ptr<const std::string> rewrittenSrcml;  // srcML of the unit with the attributes of this run alone and, on request, the comments
    archivePtr          rewrittenArchive;       // Archive in memory that reads rewrittenSrcml, which the unit read from it needs until its write
    unitPtr             rewritten;              // Unit read from rewrittenSrcml, or null

    void                readRewritten   (std::string);
};

#endif

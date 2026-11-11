// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file annotated_unit.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "output/annotated_unit.hpp"
#include "archive/srcml_namespaces.hpp"
#include "output/stereotype_comments.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

namespace {
    // Gets the XPath of the element at a path, such as /*/*[3]/*[2] for the second element child of the third element child of the unit
    //
    std::string xpathOf(const elementPath& path) {
        std::string xpath = "/*";
        for (int position : path) xpath += "/*[" + std::to_string(position) + "]";
        return xpath;
    }

    // Adds the st:stereotype attribute to each annotated element, which srcML finds by its child positions in one parse of the unit
    // The transforms belong to an archive of their own, so that no other thread shares them
    //
    transformResultPtr annotate(srcml_unit* unit, const std::vector<stereotypeAnnotation>& annotations) {
        archivePtr transforms = createArchive();
        for (const stereotypeAnnotation& annotation : annotations)
            checkStatus(srcml_append_transform_xpath_attribute(transforms.get(), xpathOf(annotation.path).c_str(), "st", stereotypeNamespace, "stereotype", annotation.stereotypes.c_str()),
                        "cannot add a stereotype attribute");

        srcml_transform_result* result = nullptr;
        const int status = srcml_unit_apply_transforms(transforms.get(), unit, &result);
        transformResultPtr annotated(result);
        if (status != SRCML_STATUS_OK || !annotated || srcml_transform_get_unit_size(annotated.get()) == 0)
            throw std::runtime_error("cannot add the stereotype attributes to a unit");
        return annotated;
    }
}

// Adds the stereotype attributes and, on request, the stereotype comments to a unit, and declares the stereotype namespace on it
// libxml2 replaces the stereotypes and the stereotype comments of an earlier run, while a transform of libsrcml labels any other unit faster
// Every unit declares the namespace, as the README states and the outputs of the integration tests expect
//
annotatedUnit::annotatedUnit(unitPtr input_, const std::vector<stereotypeAnnotation>& annotations, bool comments) : input{std::move(input_)} {
    const char* srcml = srcml_unit_get_srcml(input.get());
    if (!srcml) throw std::runtime_error("cannot read the srcML of a unit");

    const bool commented = comments && (!annotations.empty() || mayHoldStereotypeComments(srcml));
    if (commented || mayHoldStereotypes(srcml)) {
        std::string labeled = withStereotypeAttributes(srcml, annotations);
        readRewritten(commented ? withStereotypeComments(labeled) : std::move(labeled));
    }
    else if (!annotations.empty()) {
        annotated = annotate(input.get(), annotations);
    }
    checkStatus(srcml_unit_register_namespace(get(), "st", stereotypeNamespace), "cannot declare the stereotype namespace");
}

// Gets the unit to write
//
srcml_unit* annotatedUnit::get() const {
    if (rewritten) return rewritten.get();
    return annotated ? srcml_transform_get_unit(annotated.get(), 0) : input.get();
}

// Reads the unit back from its rewritten srcML, since libsrcml cannot change the nodes of a unit
//
void annotatedUnit::readRewritten(std::string srcml) {
    rewrittenSrcml = std::make_unique<const std::string>(std::move(srcml));
    rewrittenArchive = createArchive();
    checkStatus(srcml_archive_read_open_memory(rewrittenArchive.get(), rewrittenSrcml->data(), rewrittenSrcml->size()),
                "cannot read a unit with its stereotypes");
    rewritten.reset(srcml_archive_read_unit(rewrittenArchive.get()));
    if (!rewritten) throw std::runtime_error("cannot read a unit with its stereotypes");
}

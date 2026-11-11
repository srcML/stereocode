// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file input_archive.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "archive/input_archive.hpp"
#include "archive/srcml_namespaces.hpp"

#include <libxml/parser.h>
#include <libxml/xmlreader.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    const char* const lfsPointerStart = "version https://git-lfs";

    // Throws when the file cannot be read or does not start like a srcML document
    //
    void checkFileStart(const std::string& fileName) {
        std::error_code error;
        if (std::filesystem::is_directory(fileName, error))
            throw std::runtime_error(fileName + " is a directory, not a srcML archive");

        std::unique_ptr<std::FILE, int (*)(std::FILE*)> file(std::fopen(fileName.c_str(), "rb"), &std::fclose);
        if (!file) throw std::runtime_error("cannot open " + fileName + ": " + std::strerror(errno));

        char start[32] = {};
        const std::size_t length = std::fread(start, 1, sizeof(start), file.get());
        if (length == 0) throw std::runtime_error(fileName + " is empty, not a srcML archive");

        if (std::strncmp(start, lfsPointerStart, std::strlen(lfsPointerStart)) == 0)
            throw std::runtime_error(fileName + " is a Git LFS pointer, not a srcML archive (run git lfs pull to fetch the archive)");
    }

    // Throws when the root element is not a srcML unit, since libsrcml waits forever on such a document
    //
    void checkRootElement(const std::string& fileName) {
        std::unique_ptr<xmlTextReader, void (*)(xmlTextReaderPtr)> reader(
            xmlReaderForFile(fileName.c_str(), nullptr, XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING), &xmlFreeTextReader);
        if (!reader) throw std::runtime_error("cannot open " + fileName);

        int status = xmlTextReaderRead(reader.get());
        while (status == 1 && xmlTextReaderNodeType(reader.get()) != XML_READER_TYPE_ELEMENT)
            status = xmlTextReaderRead(reader.get());

        const bool isUnit = status == 1 &&
                            xmlStrEqual(xmlTextReaderConstLocalName(reader.get()), BAD_CAST "unit") &&
                            xmlStrEqual(xmlTextReaderConstNamespaceUri(reader.get()), BAD_CAST srcNamespace);
        if (!isUnit) throw std::runtime_error(fileName + " is not a srcML archive (its root element is not a srcML unit)");
    }
}

// Opens a srcML archive for reading after checking that the file holds srcML
//
archivePtr openInputArchive(const std::string& fileName) {
    checkFileStart(fileName);
    checkRootElement(fileName);

    archivePtr archive = createArchive();
    if (srcml_archive_read_open_filename(archive.get(), fileName.c_str()) != SRCML_STATUS_OK)
        throw std::runtime_error("cannot read the srcML archive " + fileName);
    return archive;
}

// Throws when the file is not well-formed XML, such as an archive cut off inside or after a unit
// libsrcml reports such damage only on standard error and then reads the archive as if it ended, so the whole file is parsed here
//
void checkWellFormed(const std::string& fileName) {
    std::unique_ptr<std::FILE, int (*)(std::FILE*)> file(std::fopen(fileName.c_str(), "rb"), &std::fclose);
    if (!file) throw std::runtime_error("cannot open " + fileName + ": " + std::strerror(errno));

    // A SAX parser without callbacks builds nothing and only checks the document
    xmlSAXHandler noCallbacks{};
    noCallbacks.initialized = XML_SAX2_MAGIC;
    std::unique_ptr<xmlParserCtxt, void (*)(xmlParserCtxtPtr)> parser(
        xmlCreatePushParserCtxt(&noCallbacks, nullptr, nullptr, 0, fileName.c_str()), &xmlFreeParserCtxt);
    if (!parser) throw std::runtime_error("cannot create an XML parser for " + fileName);
    xmlCtxtUseOptions(parser.get(), XML_PARSE_NONET | XML_PARSE_HUGE | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

    std::vector<char> buffer(1 << 16);
    bool parsed = true;
    std::size_t length = 0;
    while (parsed && (length = std::fread(buffer.data(), 1, buffer.size(), file.get())) > 0)
        parsed = xmlParseChunk(parser.get(), buffer.data(), static_cast<int>(length), 0) == 0;
    if (std::ferror(file.get())) throw std::runtime_error("cannot read " + fileName);
    if (parsed) xmlParseChunk(parser.get(), nullptr, 0, 1);

    if (!parser->wellFormed) {
        const xmlError* error = xmlCtxtGetLastError(parser.get());
        std::string reason = error && error->message ? error->message : "unknown error";
        while (!reason.empty() && (reason.back() == '\n' || reason.back() == ' ')) reason.pop_back();
        const int line = error ? error->line : 0;
        throw std::runtime_error(fileName + " is not well-formed XML at line " + std::to_string(line) + ": " + reason);
    }
}

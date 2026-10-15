#pragma once

#include <string>
#include <vector>

// Reader for the CSV files of Oura's data export (membership.ouraring.com/data-export).
//
// They are semicolon separated and not quite regular. Cracked-Oura, which ingests
// the same files, documents what it had to tolerate, and so does this reader:
//  * a whole line is sometimes wrapped in quotes, with doubled quotes inside;
//  * JSON cells (contributors, heart-rate series) contain commas, braces and
//    quotes, but never need quoting by the reader if brackets are tracked;
//  * in some files (daily activity, sleep model) a row holds fewer cells than the
//    header, and the missing ones are the *leading* columns;
//  * in others a row may simply stop early, or carry a surplus cell.
namespace Oura {

struct CsvTable
{
    std::vector<std::string> header;
    std::vector<std::vector<std::string>> rows;
    char delimiter = ';';

    int column(const std::string &name) const
    {
        for (size_t i = 0; i < header.size(); ++i)
            if (header[i] == name)
                return static_cast<int>(i);
        return -1;
    }
};

// `fileName` (lower case, without a directory) selects the alignment rule for
// short rows.
CsvTable parseOuraCsv(const std::string &text, const std::string &fileName);

// Splits one line into cells; exposed for tests.
std::vector<std::string> splitCsvLine(const std::string &line, char delimiter);

} // namespace Oura

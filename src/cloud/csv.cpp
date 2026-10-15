#include "csv.h"

#include <algorithm>

namespace Oura {

namespace {

std::string trim(const std::string &s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n'))
        ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n'))
        --b;
    return s.substr(a, b - a);
}

std::string replaceAll(std::string s, const std::string &from, const std::string &to)
{
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

// A cell that is entirely inside one pair of quotes loses them.
std::string unquote(const std::string &cell)
{
    if (cell.size() >= 2 && cell.front() == '"' && cell.back() == '"')
        return replaceAll(cell.substr(1, cell.size() - 2), "\"\"", "\"");
    return cell;
}

bool shortRowsMissLeadingColumns(const std::string &fileName)
{
    return fileName.find("dailyactivity") != std::string::npos
            || fileName.find("sleepmodel") != std::string::npos;
}

} // namespace

std::vector<std::string> splitCsvLine(const std::string &lineIn, char delimiter)
{
    std::string line = lineIn;
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
        line.pop_back();

    std::vector<std::string> cells;

    // Standard CSV: every cell quoted, "a";"b". Anything else that starts and
    // ends with a quote is a wrapped line.
    const std::string quotedSeparator = std::string("\"") + delimiter + "\"";
    const bool wrapped = line.size() >= 2 && line.front() == '"' && line.back() == '"';
    const bool standard = wrapped && line.find(quotedSeparator) != std::string::npos;
    if (wrapped && !standard)
        line = replaceAll(line.substr(1, line.size() - 2), "\"\"", "\"");

    if (standard) {
        std::string cell;
        bool inQuotes = false;
        for (size_t i = 0; i < line.size(); ++i) {
            const char c = line[i];
            if (inQuotes) {
                if (c == '"' && i + 1 < line.size() && line[i + 1] == '"') {
                    cell += '"';
                    ++i;
                } else if (c == '"') {
                    inQuotes = false;
                } else {
                    cell += c;
                }
            } else if (c == '"') {
                inQuotes = true;
            } else if (c == delimiter) {
                cells.push_back(cell);
                cell.clear();
            } else {
                cell += c;
            }
        }
        cells.push_back(cell);
        return cells;
    }

    // Tolerant mode: split on the delimiter outside brackets and JSON strings.
    std::string cell;
    int depth = 0;
    bool inString = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (inString) {
            cell += c;
            if (c == '\\' && i + 1 < line.size()) {
                cell += line[++i];
            } else if (c == '"') {
                inString = false;
            }
            continue;
        }
        if (depth > 0 && c == '"') {
            inString = true;
            cell += c;
            continue;
        }
        if (c == '{' || c == '[') {
            ++depth;
        } else if ((c == '}' || c == ']') && depth > 0) {
            --depth;
        } else if (c == delimiter && depth == 0) {
            cells.push_back(unquote(trim(cell)));
            cell.clear();
            continue;
        }
        cell += c;
    }
    cells.push_back(unquote(trim(cell)));
    return cells;
}

CsvTable parseOuraCsv(const std::string &textIn, const std::string &fileName)
{
    CsvTable table;
    std::string text = textIn;
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xef
        && static_cast<unsigned char>(text[1]) == 0xbb && static_cast<unsigned char>(text[2]) == 0xbf)
        text.erase(0, 3);

    size_t pos = 0;
    bool haveHeader = false;
    while (pos < text.size()) {
        size_t end = text.find('\n', pos);
        if (end == std::string::npos)
            end = text.size();
        const std::string line = text.substr(pos, end - pos);
        pos = end + 1;
        if (trim(line).empty())
            continue;

        if (!haveHeader) {
            table.delimiter = line.find(';') != std::string::npos ? ';' : ',';
            for (std::string &cell : splitCsvLine(line, table.delimiter)) {
                table.header.push_back(trim(unquote(trim(cell))));
            }
            haveHeader = true;
            continue;
        }

        std::vector<std::string> cells = splitCsvLine(line, table.delimiter);
        const size_t want = table.header.size();
        if (cells.size() < want) {
            const size_t missing = want - cells.size();
            if (shortRowsMissLeadingColumns(fileName))
                cells.insert(cells.begin(), missing, std::string());
            else
                cells.insert(cells.end(), missing, std::string());
        } else if (cells.size() > want) {
            cells.resize(want);
        }
        table.rows.push_back(std::move(cells));
    }
    return table;
}

} // namespace Oura

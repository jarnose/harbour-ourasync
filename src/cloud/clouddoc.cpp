#include "clouddoc.h"

#include "analysis/daysummary.h" // daysFromCivil

#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace Oura {

// ---- time -------------------------------------------------------------------

namespace {

bool digits(const std::string &s, size_t from, size_t count, int &out)
{
    if (from + count > s.size())
        return false;
    int v = 0;
    for (size_t i = 0; i < count; ++i) {
        const char c = s[from + i];
        if (c < '0' || c > '9')
            return false;
        v = v * 10 + (c - '0');
    }
    out = v;
    return true;
}

} // namespace

std::optional<int64_t> parseIso8601(const std::string &text)
{
    int y, mo, d;
    if (!digits(text, 0, 4, y) || text.size() < 10 || text[4] != '-' || !digits(text, 5, 2, mo)
        || text[7] != '-' || !digits(text, 8, 2, d))
        return std::nullopt;
    int64_t seconds = daysFromCivil(y, static_cast<unsigned>(mo), static_cast<unsigned>(d)) * 86400;
    if (text.size() == 10)
        return seconds;

    int h = 0, mi = 0, s = 0;
    if ((text[10] != 'T' && text[10] != ' ') || !digits(text, 11, 2, h) || text.size() < 16 || text[13] != ':'
        || !digits(text, 14, 2, mi))
        return std::nullopt;
    size_t i = 16;
    if (i < text.size() && text[i] == ':' && digits(text, i + 1, 2, s))
        i += 3;
    if (i < text.size() && text[i] == '.') {
        ++i;
        while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])))
            ++i;
    }
    seconds += h * 3600 + mi * 60 + s;
    return seconds - isoOffsetSeconds(text);
}

int isoOffsetSeconds(const std::string &text)
{
    // The offset is the last +hh:mm or -hh:mm after the time part.
    if (text.size() < 17)
        return 0;
    const size_t at = text.find_last_of("+-");
    if (at == std::string::npos || at < 11)
        return 0;
    int hh, mm = 0;
    if (!digits(text, at + 1, 2, hh))
        return 0;
    size_t p = at + 3;
    if (p < text.size() && text[p] == ':')
        ++p;
    digits(text, p, 2, mm);
    const int off = hh * 3600 + mm * 60;
    return text[at] == '-' ? -off : off;
}

// ---- documents --------------------------------------------------------------

std::string kindForCsvName(const std::string &n)
{
    // The export names its files without separators; accept the underscored
    // spelling too, since the API's names are.
    std::string name;
    for (char c : n)
        if (c != '_' && c != '-')
            name += c;
    if (name.size() > 4 && name.compare(name.size() - 4, 4, ".csv") == 0)
        name.erase(name.size() - 4);
    if (name == "dailysleep") return CloudKind::DailySleep;
    if (name == "dailyreadiness") return CloudKind::DailyReadiness;
    if (name == "dailyactivity") return CloudKind::DailyActivity;
    if (name == "sleepmodel" || name == "sleep") return CloudKind::Sleep;
    if (name == "dailyspo2") return CloudKind::DailySpo2;
    if (name == "dailystress") return CloudKind::DailyStress;
    if (name == "dailyresilience") return CloudKind::DailyResilience;
    if (name == "dailycardiovascularage") return CloudKind::CardiovascularAge;
    if (name == "workout") return CloudKind::Workout;
    if (name == "session") return CloudKind::Session;
    if (name == "enhancedtag" || name == "tag") return CloudKind::Tag;
    if (name == "sleeptime") return CloudKind::SleepTime;
    return std::string();
}

namespace {

Json cellToJson(const std::string &cell)
{
    if (cell.empty() || cell == "null" || cell == "NaN" || cell == "nan")
        return Json();
    if (cell.front() == '{' || cell.front() == '[') {
        // A JSON cell, possibly still carrying the CSV's doubled quotes.
        if (auto j = Json::parse(cell))
            return *j;
        std::string fixed;
        for (size_t i = 0; i < cell.size(); ++i) {
            if (cell[i] == '"' && i + 1 < cell.size() && cell[i + 1] == '"')
                ++i;
            fixed += cell[i];
        }
        if (auto j = Json::parse(fixed))
            return *j;
    }
    return Json::string(cell);
}

std::string dayOf(const Json &row)
{
    std::string day = row.str("day");
    if (day.size() >= 10)
        return day.substr(0, 10);
    for (const char *key : { "timestamp", "bedtime_start", "start_datetime", "start_time" }) {
        const std::string t = row.str(key);
        if (t.size() >= 10 && parseIso8601(t.substr(0, 10)))
            return t.substr(0, 10);
    }
    return std::string();
}

} // namespace

void addCsvFile(CloudImport &out, const std::string &fileName, const std::string &text)
{
    std::string lower;
    for (char c : fileName)
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    const size_t slash = lower.find_last_of("/\\");
    if (slash != std::string::npos)
        lower.erase(0, slash + 1);

    const CsvTable table = parseOuraCsv(text, lower);

    if (lower == "heartrate.csv") {
        const int ts = table.column("timestamp"), bpm = table.column("bpm");
        size_t added = 0;
        if (ts >= 0 && bpm >= 0) {
            out.hr.reserve(out.hr.size() + table.rows.size());
            for (const auto &row : table.rows) {
                const auto t = parseIso8601(row[static_cast<size_t>(ts)]);
                const int v = std::atoi(row[static_cast<size_t>(bpm)].c_str());
                if (t && v >= 25 && v <= 250) {
                    out.hr.push_back({ *t, v });
                    ++added;
                }
            }
        }
        out.notes.push_back(lower + ": " + std::to_string(added) + " heart rate samples");
        return;
    }

    const std::string kind = kindForCsvName(lower);
    if (kind.empty())
        return;

    size_t added = 0, skipped = 0;
    for (const auto &cells : table.rows) {
        Json row = Json::object();
        for (size_t i = 0; i < table.header.size() && i < cells.size(); ++i)
            row.set(table.header[i], cellToJson(cells[i]));
        CloudDoc doc;
        doc.kind = kind;
        doc.day = dayOf(row);
        if (doc.day.empty()) {
            ++skipped;
            continue;
        }
        doc.id = row.str("id");
        if (doc.id.empty() || doc.id == "nan")
            doc.id = kind + ":" + doc.day + ":" + row.str("timestamp") + row.str("start_datetime");
        doc.json = std::move(row);
        out.docs.push_back(std::move(doc));
        ++added;
    }
    std::string note = lower + ": " + std::to_string(added) + " rows";
    if (skipped)
        note += " (" + std::to_string(skipped) + " without a date skipped)";
    out.notes.push_back(note);
}

std::string addApiPage(CloudImport &out, const std::string &kind, const Json &reply)
{
    if (const Json *data = reply.get("data")) {
        for (const Json &item : data->items()) {
            if (!item.isObject())
                continue;
            CloudDoc doc;
            doc.kind = kind;
            doc.day = dayOf(item);
            if (doc.day.empty())
                continue;
            doc.id = item.str("id");
            if (doc.id.empty())
                doc.id = kind + ":" + doc.day;
            doc.json = item;
            out.docs.push_back(std::move(doc));
        }
    }
    const Json *next = reply.get("next_token");
    return next && next->isString() ? next->asString() : std::string();
}

} // namespace Oura

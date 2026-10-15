// g++ -std=c++17 -Wall -Wextra -I ../src ../src/cloud/json.cpp ../src/cloud/csv.cpp ../src/cloud/zip.cpp test_cloudparse.cpp -lz -o /tmp/cp && /tmp/cp
#include "hex.h"
#include "cloud/csv.h"
#include "cloud/json.h"
#include "cloud/zip.h"

#include <zlib.h>

#include <clocale>
#include <cstdio>
#include <cstring>

using namespace Oura;

static void json()
{
    auto j = Json::parse(R"({"a":1,"b":[1,2.5,"x"],"c":{"d":null,"e":true},"s":"q\"ä\n"})");
    CHECK(j.has_value());
    CHECK_EQ(j->num("a"), 1.0);
    CHECK_EQ(j->get("b")->items().size(), size_t(3));
    CHECK_EQ(j->get("b")->items()[1].asNumber(), 2.5);
    CHECK(j->get("c")->get("d")->isNull());
    CHECK_EQ(j->str("s"), std::string("q\"\xc3\xa4\n"));
    // Round trip keeps order.
    CHECK_EQ(Json::parse(j->dump())->dump(), j->dump());
    // Numbers in strings count as numbers (CSV cells are text).
    auto k = Json::parse(R"({"score":"82","name":"abc","empty":""})");
    CHECK_EQ(k->num("score"), 82.0);
    CHECK(k->optNum("score").has_value());
    CHECK(!k->optNum("name").has_value());
    CHECK(!k->optNum("empty").has_value());
    CHECK(!k->optNum("missing").has_value());
    // Surrogate pair.
    CHECK_EQ(Json::parse(R"("😀")")->asString(), std::string("\xf0\x9f\x98\x80"));
    // Garbage is refused, with a position.
    size_t at = 0;
    CHECK(!Json::parse("{\"a\":", &at).has_value());
    CHECK(!Json::parse("[1,2] x").has_value());
    CHECK(!Json::parse("").has_value());
    // NaN as some exports write it.
    CHECK(Json::parse("[NaN]")->items()[0].isNull());
}

static void csv()
{
    // Plain header and rows.
    auto t = parseOuraCsv("id;day;score\nabc;2026-10-14;82\ndef;2026-10-15;77\n", "dailysleep.csv");
    CHECK_EQ(t.header.size(), size_t(3));
    CHECK_EQ(t.rows.size(), size_t(2));
    CHECK_EQ(t.rows[1][2], std::string("77"));
    CHECK_EQ(t.column("day"), 1);

    // A JSON cell with commas and quotes, in a wrapped line (doubled quotes).
    auto w = parseOuraCsv("id;day;contributors\n\"x1;2026-10-14;{\"\"deep_sleep\"\":85,\"\"timing\"\":99}\"\n", "dailysleep.csv");
    CHECK_EQ(w.rows.size(), size_t(1));
    CHECK_EQ(w.rows[0][2], std::string("{\"deep_sleep\":85,\"timing\":99}"));
    auto c = Json::parse(w.rows[0][2]);
    CHECK(c.has_value());
    CHECK_EQ(c->num("timing"), 99.0);

    // The same JSON unquoted.
    auto u = parseOuraCsv("id;day;contributors\nx1;2026-10-14;{\"deep_sleep\":85,\"timing\":99}\n", "dailysleep.csv");
    CHECK_EQ(u.rows[0][2], std::string("{\"deep_sleep\":85,\"timing\":99}"));

    // A semicolon inside a JSON string does not split the cell.
    auto s = splitCsvLine("a;{\"note\":\"x;y\"};b", ';');
    CHECK_EQ(s.size(), size_t(3));
    CHECK_EQ(s[1], std::string("{\"note\":\"x;y\"}"));

    // Standard quoting of every cell.
    auto q = splitCsvLine("\"a\";\"b;c\";\"d\"\"e\"", ';');
    CHECK_EQ(q.size(), size_t(3));
    CHECK_EQ(q[1], std::string("b;c"));
    CHECK_EQ(q[2], std::string("d\"e"));

    // Short rows: leading columns missing in the activity file...
    auto a = parseOuraCsv("id;day;score;steps\n2026-10-14;80;9000\n", "dailyactivity.csv");
    CHECK_EQ(a.rows[0][0], std::string(""));
    CHECK_EQ(a.rows[0][1], std::string("2026-10-14"));
    CHECK_EQ(a.rows[0][3], std::string("9000"));
    // ... trailing ones elsewhere.
    auto b = parseOuraCsv("id;day;score;steps\nx;2026-10-14;80\n", "dailysleep.csv");
    CHECK_EQ(b.rows[0][2], std::string("80"));
    CHECK_EQ(b.rows[0][3], std::string(""));
    // A surplus cell is dropped.
    auto e = parseOuraCsv("a;b\n1;2;3\n", "x.csv");
    CHECK_EQ(e.rows[0].size(), size_t(2));
    // BOM, CRLF and blank lines.
    auto d = parseOuraCsv("\xef\xbb\xbf" "a;b\r\n1;2\r\n\r\n3;4\r\n", "x.csv");
    CHECK_EQ(d.header[0], std::string("a"));
    CHECK_EQ(d.rows.size(), size_t(2));
    // Comma-separated files are recognised too.
    auto m = parseOuraCsv("a,b\n1,2\n", "x.csv");
    CHECK_EQ(m.delimiter, ',');
    CHECK_EQ(m.rows[0][1], std::string("2"));
}

// A ZIP built by hand, so the test needs no files and no zip tool.
static void put16(std::string &s, unsigned v) { s += char(v & 0xff); s += char(v >> 8); }
static void put32(std::string &s, uint32_t v) { put16(s, v & 0xffff); put16(s, v >> 16); }

static std::string makeZip(const std::vector<std::pair<std::string, std::string>> &files, bool compress)
{
    std::string out;
    std::string central;
    for (const auto &f : files) {
        std::string payload = f.second;
        unsigned method = 0;
        if (compress) {
            z_stream z;
            std::memset(&z, 0, sizeof z);
            deflateInit2(&z, 6, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
            std::string buf(f.second.size() + 64, '\0');
            z.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(f.second.data()));
            z.avail_in = static_cast<uInt>(f.second.size());
            z.next_out = reinterpret_cast<Bytef *>(&buf[0]);
            z.avail_out = static_cast<uInt>(buf.size());
            deflate(&z, Z_FINISH);
            buf.resize(buf.size() - z.avail_out);
            deflateEnd(&z);
            payload = buf;
            method = 8;
        }
        const uint32_t offset = static_cast<uint32_t>(out.size());
        const uint32_t crc = crc32(0, reinterpret_cast<const Bytef *>(f.second.data()), static_cast<uInt>(f.second.size()));
        put32(out, 0x04034b50); put16(out, 20); put16(out, 0); put16(out, method); put16(out, 0); put16(out, 0);
        put32(out, crc); put32(out, uint32_t(payload.size())); put32(out, uint32_t(f.second.size()));
        put16(out, unsigned(f.first.size())); put16(out, 0);
        out += f.first; out += payload;
        put32(central, 0x02014b50); put16(central, 20); put16(central, 20); put16(central, 0); put16(central, method);
        put16(central, 0); put16(central, 0); put32(central, crc); put32(central, uint32_t(payload.size()));
        put32(central, uint32_t(f.second.size())); put16(central, unsigned(f.first.size())); put16(central, 0);
        put16(central, 0); put16(central, 0); put16(central, 0); put32(central, 0); put32(central, offset);
        central += f.first;
    }
    const uint32_t dirOffset = static_cast<uint32_t>(out.size());
    out += central;
    put32(out, 0x06054b50); put16(out, 0); put16(out, 0); put16(out, unsigned(files.size())); put16(out, unsigned(files.size()));
    put32(out, uint32_t(central.size())); put32(out, dirOffset); put16(out, 0);
    return out;
}

static void zip()
{
    std::string big;
    for (int i = 0; i < 5000; ++i)
        big += "row " + std::to_string(i) + ";2026-10-14;82\n";
    for (bool compress : { false, true }) {
        const std::string z = makeZip({ { "export/dailysleep.csv", "id;day;score\na;2026-10-14;82\n" },
                                        { "export/heartrate.csv", big } },
                                       compress);
        std::vector<ZipEntry> entries;
        std::string err;
        CHECK(listZip(z, entries, err));
        CHECK_EQ(entries.size(), size_t(2));
        CHECK_EQ(entries[0].name, std::string("export/dailysleep.csv"));
        std::string out;
        CHECK(extractZipEntry(z, entries[0], out, err));
        CHECK_EQ(out, std::string("id;day;score\na;2026-10-14;82\n"));
        CHECK(extractZipEntry(z, entries[1], out, err));
        CHECK_EQ(out, big);
        if (compress)
            CHECK(entries[1].compressedSize < entries[1].size);
    }
    std::vector<ZipEntry> entries;
    std::string err;
    CHECK(!listZip("not a zip file at all, really", entries, err));
    CHECK(!listZip("", entries, err));
    // A truncated archive is refused rather than read past its end.
    std::string z = makeZip({ { "a.csv", "x;y\n1;2\n" } }, true);
    CHECK(!listZip(z.substr(0, z.size() - 10), entries, err));
}

// A Qt application takes the process locale from the environment; with a comma
// as the decimal separator strtod and printf misread and misprint JSON numbers.
// This failed on a real Finnish phone with Oura's token reply.
static void commaLocale()
{
    const char *names[] = { "fi_FI.UTF-8", "fi_FI.utf8", "fi_FI", "de_DE.UTF-8", "de_DE.utf8", "de_DE" };
    bool set = false;
    for (const char *name : names)
        if (std::setlocale(LC_ALL, name)) {
            set = true;
            break;
        }
    if (!set) {
        std::printf("(no comma locale installed here; the locale test is skipped)\n");
        return;
    }
    const char *reply = R"({"access_token":"A","token_type":"bearer","expires_in":25919999,"id_token":null,"scope":"x","refresh_token":"R"})";
    auto j = Json::parse(reply);
    CHECK(j.has_value());
    if (j) {
        CHECK_EQ(j->str("access_token"), std::string("A"));
        CHECK_EQ(j->num("expires_in"), 25919999.0);
        CHECK_EQ(j->str("refresh_token"), std::string("R"));
    }
    auto k = Json::parse("[1,2.5,3,-4e2,0.25]");
    CHECK(k.has_value());
    if (k) {
        CHECK_EQ(k->items().size(), size_t(5));
        CHECK_EQ(k->items()[1].asNumber(), 2.5);
        CHECK_EQ(k->items()[3].asNumber(), -400.0);
        CHECK_EQ(k->items()[4].asNumber(), 0.25);
        CHECK_EQ(k->dump(), std::string("[1,2.5,3,-400,0.25]"));
    }
    // CSV cells are text; their decimals use a point.
    auto c = Json::parse(R"({"v":"12.5","w":"7"})");
    CHECK_EQ(c->num("v"), 12.5);
    CHECK_EQ(c->num("w"), 7.0);
    std::setlocale(LC_ALL, "C");
}

int main()
{
    commaLocale();
    json();
    csv();
    zip();
    return finish("cloudparse");
}

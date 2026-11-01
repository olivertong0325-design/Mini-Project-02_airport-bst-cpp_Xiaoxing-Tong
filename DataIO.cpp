#include "DataIO.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {
std::string cleanLine(std::string line) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.size() >= 3 && line.substr(0, 3) == "\xEF\xBB\xBF") line.erase(0, 3);
    return line;
}

double coordinate(const std::string& text, double lo, double hi) {
    std::size_t consumed = 0;
    double value = std::stod(text, &consumed);
    if (consumed != text.size() || !std::isfinite(value) || value < lo || value > hi) {
        throw std::runtime_error("Invalid coordinate: " + text);
    }
    return value;
}

std::string quoteCSV(const std::string& text) {
    std::string result = "\"";
    int n = static_cast<int>(text.size());
    for (int i = 0; i < n; i++) {
        if (text[i] == '"') result += '"';
        result += text[i];
    }
    return result + '"';
}

void insertRecords(AirportBST& tree, const std::vector<Airport>& records,
                   std::vector<std::string>* duplicates = nullptr) {
    int n = static_cast<int>(records.size());
    for (int i = 0; i < n; i++) {
        if (!tree.insert(records[i]) && duplicates != nullptr) {
            duplicates->push_back(records[i].code);
        }
    }
}
}

// Task 1(a): a quote-aware parser, including commas and escaped double quotes.
std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool quoted = false;
    bool closedQuote = false;
    int n = static_cast<int>(line.size());
    for (int i = 0; i < n; i++) {
        char ch = line[i];
        if (quoted) {
            if (ch == '"') {
                if (i + 1 < n && line[i + 1] == '"') { field += '"'; i++; }
                else { quoted = false; closedQuote = true; }
            } else field += ch;
        } else if (ch == ',') {
            fields.push_back(field);
            field.clear();
            closedQuote = false;
        } else if (closedQuote) {
            throw std::runtime_error("Unexpected character after closing CSV quote");
        } else if (ch == '"') {
            if (!field.empty()) throw std::runtime_error("Quote inside unquoted field");
            quoted = true;
        } else field += ch;
    }
    if (quoted) throw std::runtime_error("Unterminated CSV quote");
    fields.push_back(field);
    return fields;
}

DataSet readAirportData(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Cannot open data file: " + path);
    DataSet result;
    std::string line;
    int physicalLine = 0;
    while (std::getline(input, line)) {
        physicalLine++;
        line = cleanLine(line);
        try {
            std::vector<std::string> fields = splitCSV(line);
            if (physicalLine == 1 && !fields.empty() && fields[0] == "airport_id") {
                result.headers++;
                continue;
            }
            result.lines++;
            if (fields.size() != 14) throw std::runtime_error("Expected 14 CSV fields");
            if (fields[4] == "\\N" || fields[4].empty()) {
                result.skippedMissing++;
                continue;
            }
            if (!validCode(fields[4])) throw std::runtime_error("Invalid code: " + fields[4]);
            Airport airport;
            airport.name = fields[1];
            airport.city = fields[2];
            airport.country = fields[3];
            airport.code = fields[4];
            airport.icao = fields[5];
            airport.latitude = coordinate(fields[6], -90.0, 90.0);
            airport.longitude = coordinate(fields[7], -180.0, 180.0);
            result.airports.push_back(airport);
        } catch (const std::exception& error) {
            throw std::runtime_error(path + ":" + std::to_string(physicalLine) + ": " + error.what());
        }
    }
    if (input.bad()) throw std::runtime_error("Failed reading data file: " + path);
    return result;
}

// Task 4(b): codes stay in a plain preorder file; a sidecar preserves full records.
void saveTree(const std::string& path, const AirportBST& tree) {
    std::vector<Airport> records = tree.preorder();
    std::ofstream metadata(path + ".records.csv");
    if (!metadata) throw std::runtime_error("Cannot write metadata sidecar: " + path);
    metadata << "airport_id,name,city,country,iata,icao,latitude,longitude,altitude,timezone,dst,tz_database,type,source\n";
    metadata << std::setprecision(17);
    int n = static_cast<int>(records.size());
    for (int i = 0; i < n; i++) {
        const Airport& a = records[i];
        metadata << "0," << quoteCSV(a.name) << ',' << quoteCSV(a.city) << ','
                 << quoteCSV(a.country) << ',' << quoteCSV(a.code) << ','
                 << quoteCSV(a.icao) << ',' << a.latitude << ',' << a.longitude
                 << ",0,\\N,\\N,\\N,airport,student-snapshot\n";
    }
    metadata.close();
    if (!metadata) throw std::runtime_error("Failed writing metadata sidecar");
    std::ofstream output(path);
    if (!output) throw std::runtime_error("Cannot write preorder file: " + path);
    for (int i = 0; i < n; i++) output << records[i].code << '\n';
    output.close();
    if (!output) throw std::runtime_error("Failed writing preorder file");
}

LoadSummary loadTree(const std::string& path, AirportBST& tree,
                     std::vector<Airport>& catalog) {
    std::ifstream probe(path);
    if (!probe) throw std::runtime_error("Cannot open file: " + path);
    std::string first;
    std::getline(probe, first);
    first = cleanLine(first);
    probe.close();
    LoadSummary summary;
    summary.dataFile = first.find(',') != std::string::npos;
    AirportBST replacement;
    std::vector<Airport> replacementCatalog;
    if (summary.dataFile) {
        DataSet data = readAirportData(path);
        summary.lines = data.lines;
        summary.skippedMissing = data.skippedMissing;
        replacementCatalog = data.airports;
        insertRecords(replacement, data.airports, &summary.duplicates);
    } else {
        AirportBST available;
        // Prefer the saved metadata, then current records, then the data catalog.
        std::ifstream sidecar(path + ".records.csv");
        if (sidecar.good()) {
            sidecar.close();
            DataSet saved = readAirportData(path + ".records.csv");
            insertRecords(available, saved.airports);
        }
        insertRecords(available, tree.preorder());
        insertRecords(available, catalog);
        std::ifstream input(path);
        std::string code;
        while (std::getline(input, code)) {
            summary.lines++;
            code = cleanLine(code);
            if (!validCode(code)) throw std::runtime_error("Invalid saved code: " + code);
            const Airport* airport = available.lookup(code);
            if (airport == nullptr) {
                throw std::runtime_error("No record for " + code + "; retain the .records.csv sidecar");
            }
            if (!replacement.insert(*airport)) {
                throw std::runtime_error("Duplicate code in saved preorder: " + code);
            }
        }
        if (input.bad()) throw std::runtime_error("Failed reading preorder file");
        summary.sameShape = tree.shapeSignature() == replacement.shapeSignature();
    }
    if (!replacement.validate()) throw std::runtime_error("Loaded tree failed validation");
    summary.inserted = replacement.size();
    // Commit only after all parsing and validation succeeds.
    tree.swap(replacement);
    if (summary.dataFile) catalog.swap(replacementCatalog);
    return summary;
}

#include "AirportBST.h"
#include "DataIO.h"
#include "Experiment.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
// Quoted command arguments preserve Windows backslashes; use "" for a quote.
std::vector<std::string> arguments(const std::string& line) {
    std::vector<std::string> result;
    std::string token;
    bool quoted = false;
    bool started = false;
    int n = static_cast<int>(line.size());
    for (int i = 0; i < n; i++) {
        char ch = line[i];
        if (ch == '"') {
            if (quoted && i + 1 < n && line[i + 1] == '"') { token += '"'; i++; }
            else quoted = !quoted;
            started = true;
        } else if (!quoted && (ch == ' ' || ch == '\t' || ch == '\r')) {
            if (started) { result.push_back(token); token.clear(); started = false; }
        } else { token += ch; started = true; }
    }
    if (quoted) throw std::runtime_error("Unterminated command quote");
    if (started) result.push_back(token);
    return result;
}

void require(bool condition, const std::string& usage) {
    if (!condition) throw std::runtime_error("Usage: " + usage);
}

std::string codeArgument(const std::string& text) {
    std::string code = uppercaseCode(text);
    if (!validCode(code)) throw std::runtime_error("Use a three-character alphanumeric code");
    return code;
}

double numberArgument(const std::string& text, double limit) {
    std::size_t used = 0;
    double value = std::stod(text, &used);
    if (used != text.size() || !(value >= -limit && value <= limit)) {
        throw std::runtime_error("Invalid coordinate: " + text);
    }
    return value;
}

void printStats(const AirportBST& tree) {
    TreeStats s = tree.stats();
    std::cout << "Airports: " << s.size << "  Height: " << s.height
              << "  Average depth: " << std::fixed << std::setprecision(4)
              << s.averageDepth << '\n';
    std::vector<Airport> airports = tree.inorder();
    std::vector<std::string> countries;
    int us = 0;
    for (int i = 0; i < s.size; i++) {
        countries.push_back(airports[i].country);
        if (airports[i].country == "United States") us++;
    }
    std::sort(countries.begin(), countries.end());
    int count = 0;
    for (int i = 0; i < s.size; i++) {
        if (i == 0 || countries[i] != countries[i - 1]) count++;
    }
    std::cout << "Countries: " << count << "  United States: " << us << '\n';
}

void printSearch(const AirportBST& tree, const std::string& code) {
    SearchResult result = tree.search(code);
    std::cout << "Path: ";
    int n = static_cast<int>(result.path.size());
    for (int i = 0; i < n; i++) {
        if (i > 0) std::cout << " -> ";
        std::cout << result.path[i];
    }
    if (!result.found) std::cout << (n > 0 ? " -> (NULL)" : "(NULL)");
    std::cout << " (comparisons: " << n << ")\n";
    if (result.found) result.airport.print(std::cout);
    else std::cout << code << " not found\n";
}

void help() {
    std::cout << "Commands (quote arguments containing spaces):\n"
        << "  load FILE       Load course .dat/.csv or saved preorder + sidecar\n"
        << "  find CODE       Show airport, path and comparison count\n"
        << "  add CODE \"NAME\" \"CITY\" \"COUNTRY\" ICAO LAT LON\n"
        << "  remove CODE     Delete; show top three levels before/after\n"
        << "  list            Inorder listing and sortedness check\n"
        << "  save FILE       Save preorder codes and FILE.records.csv\n"
        << "  validate        Check strict ancestor (lo, hi) bounds\n"
        << "  range FROM TO   Inclusive pruned range query\n"
        << "  min / max       Minimum / maximum airport\n"
        << "  levels          Top three levels (- means missing child)\n"
        << "  cases           Find real examples of search/deletion cases\n"
        << "  stats           Size, height, average depth, country counts\n"
        << "  rebalance       Build by the sorted-array middle rule\n"
        << "  experiment [CSV]  Compare four builds on original loaded data\n"
        << "  help / quit\n";
}

bool execute(const std::vector<std::string>& args, AirportBST& tree,
             std::vector<Airport>& catalog) {
    if (args.empty()) return true;
    const std::string& command = args[0];
    int n = static_cast<int>(args.size());
    if (command == "quit" || command == "exit") return false;
    if (command == "help") help();
    else if (command == "load") {
        require(n == 2, "load FILE");
        LoadSummary summary = loadTree(args[1], tree, catalog);
        std::cout << "Loaded " << summary.inserted << " airports ("
                  << summary.skippedMissing << " lines skipped: no IATA code). Duplicates: "
                  << summary.duplicates.size() << '\n';
        int duplicates = static_cast<int>(summary.duplicates.size());
        for (int i = 0; i < duplicates; i++) {
            std::cout << "Duplicate rejected (I4): " << summary.duplicates[i] << '\n';
        }
        if (!summary.dataFile) {
            std::cout << "Same shape as previous tree: " << (summary.sameShape ? "yes" : "no") << '\n';
        }
        printStats(tree);
    } else if (command == "find") {
        require(n == 2, "find CODE");
        printSearch(tree, codeArgument(args[1]));
    } else if (command == "add") {
        require(n == 8, "add CODE \"NAME\" \"CITY\" \"COUNTRY\" ICAO LAT LON");
        Airport airport;
        airport.code = codeArgument(args[1]);
        airport.name = args[2]; airport.city = args[3]; airport.country = args[4];
        airport.icao = args[5];
        airport.latitude = numberArgument(args[6], 90.0);
        airport.longitude = numberArgument(args[7], 180.0);
        bool inserted = tree.insert(airport);
        std::cout << (inserted ? "Added " : "Duplicate rejected (I4): ") << airport.code
                  << "  validate: " << (tree.validate() ? "OK" : "FAILED") << '\n';
    } else if (command == "remove") {
        require(n == 2, "remove CODE");
        std::string code = codeArgument(args[1]);
        std::cout << "Before deleting " << code << ":\n";
        tree.printLevels(std::cout);
        DeleteResult result = tree.remove(code);
        if (result.removed) {
            std::cout << "Deleted " << code << " (case " << result.caseName;
            if (!result.replacement.empty()) std::cout << "; replacement " << result.replacement;
            std::cout << ")\n";
        } else std::cout << code << " not found; nothing deleted\n";
        std::cout << "After:\n";
        tree.printLevels(std::cout);
        std::cout << "validate: " << (tree.validate() ? "OK" : "FAILED") << '\n';
    } else if (command == "list") {
        require(n == 1, "list");
        std::vector<Airport> records = tree.inorder();
        bool sorted = true;
        int count = static_cast<int>(records.size());
        for (int i = 0; i < count; i++) {
            records[i].print(std::cout);
            if (i > 0 && records[i - 1].code >= records[i].code) sorted = false;
        }
        std::cout << count << " airports. Inorder strictly sorted: " << (sorted ? "yes" : "no") << '\n';
    } else if (command == "save") {
        require(n == 2, "save FILE");
        saveTree(args[1], tree);
        std::cout << "Saved " << tree.size() << " preorder codes to " << args[1]
                  << " (+ .records.csv sidecar)\n";
    } else if (command == "validate") {
        require(n == 1, "validate");
        std::cout << "validate: " << (tree.validate() ? "OK" : "FAILED") << '\n';
    } else if (command == "range") {
        require(n == 3, "range FROM TO");
        std::string from = codeArgument(args[1]), to = codeArgument(args[2]);
        require(from <= to, "range FROM TO (FROM must be <= TO)");
        RangeResult result = tree.range(from, to);
        int count = static_cast<int>(result.airports.size());
        for (int i = 0; i < count; i++) result.airports[i].print(std::cout);
        std::cout << count << " airports, " << result.visited << " nodes visited\n";
    } else if (command == "min" || command == "max") {
        require(n == 1, "min / max");
        const Airport* airport = command == "min" ? tree.minimum() : tree.maximum();
        if (airport != nullptr) airport->print(std::cout);
        else std::cout << "Tree is empty\n";
    } else if (command == "levels") {
        require(n == 1, "levels");
        tree.printLevels(std::cout);
    } else if (command == "cases") {
        require(n == 1, "cases");
        std::cout << "Root: " << tree.rootCode() << "  Leaf: " << tree.candidate("leaf")
                  << "  Deep: " << tree.deepestCode() << '\n'
                  << "Left-only: " << tree.candidate("left")
                  << "  Right-only: " << tree.candidate("right")
                  << "  Two children: " << tree.candidate("two") << '\n';
    } else if (command == "stats") {
        require(n == 1, "stats");
        printStats(tree);
    } else if (command == "rebalance") {
        require(n == 1, "rebalance");
        tree.rebalance();
        printStats(tree);
        std::cout << "validate: " << (tree.validate() ? "OK" : "FAILED") << '\n';
    } else if (command == "experiment") {
        require(n == 1 || n == 2, "experiment [CSV]");
        std::ofstream output;
        if (n == 2) {
            output.open(args[1]);
            if (!output) throw std::runtime_error("Cannot write experiment CSV");
        }
        runExperiment(catalog.empty() ? tree.preorder() : catalog, std::cout,
                      n == 2 ? &output : nullptr);
        if (n == 2) {
            output.close();
            if (!output) throw std::runtime_error("Failed writing experiment CSV");
        }
    } else throw std::runtime_error("Unknown command: " + command + " (try help)");
    return true;
}
}

int main(int argc, char* argv[]) {
    try {
        if (argc >= 2 && std::string(argv[1]) == "--experiment") {
            require(argc == 3 || argc == 4, "airports --experiment DATA [CSV]");
            DataSet data = readAirportData(argv[2]);
            std::ofstream csv;
            if (argc == 4) {
                csv.open(argv[3]);
                if (!csv) throw std::runtime_error("Cannot write experiment CSV");
            }
            runExperiment(data.airports, std::cout, argc == 4 ? &csv : nullptr);
            if (argc == 4) {
                csv.close();
                if (!csv) throw std::runtime_error("Failed writing experiment CSV");
            }
            return 0;
        }
        std::ifstream script;
        std::istream* input = &std::cin;
        bool batch = argc > 1;
        if (batch) {
            require(argc == 3 && std::string(argv[1]) == "--script", "airports [--script FILE]");
            script.open(argv[2]);
            if (!script) throw std::runtime_error("Cannot open command script");
            input = &script;
        } else std::cout << "Airport BST lookup service (C++17). Type help.\n";
        AirportBST tree;
        std::vector<Airport> catalog;
        std::string line;
        bool hadError = false;
        while (true) {
            if (!batch) std::cout << "> " << std::flush;
            if (!std::getline(*input, line)) break;
            if (line.empty() || line[0] == '#') continue;
            if (batch) std::cout << "> " << line << '\n';
            try {
                if (!execute(arguments(line), tree, catalog)) break;
            } catch (const std::exception& error) {
                std::cerr << "Error: " << error.what() << '\n';
                hadError = true;
            }
        }
        return batch && hadError ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}

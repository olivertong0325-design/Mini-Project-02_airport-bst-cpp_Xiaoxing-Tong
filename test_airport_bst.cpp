#include "AirportBST.h"
#include "DataIO.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int checks = 0;
int validatedMutations = 0;
const std::string scratch = "tests/tmp/";

void check(bool condition, const std::string& message) {
    checks++;
    if (!condition) throw std::runtime_error(message);
}

Airport example(const std::string& code) {
    Airport a;
    a.code = code; a.name = "Test " + code; a.city = "Boston";
    a.country = "United States"; a.icao = "K" + code;
    a.latitude = 42.36; a.longitude = -71.01;
    return a;
}

void validateMutation(const AirportBST& tree) {
    check(tree.validate(), "BST invalid after a mutation");
    validatedMutations++;
}

void buildChecked(AirportBST& tree, const std::vector<Airport>& records) {
    int n = static_cast<int>(records.size());
    for (int i = 0; i < n; i++) {
        check(tree.insert(records[i]), "Unexpected duplicate: " + records[i].code);
        validateMutation(tree);
    }
}

bool sameRecord(const Airport& a, const Airport& b) {
    return a.code == b.code && a.name == b.name && a.city == b.city &&
           a.country == b.country && a.icao == b.icao &&
           a.latitude == b.latitude && a.longitude == b.longitude;
}

template <typename Action> void throws(Action action, const std::string& label) {
    bool failed = false;
    try { action(); } catch (const std::exception&) { failed = true; }
    check(failed, "Expected rejection: " + label);
}

void testSmallTrees() {
    AirportBST tree;
    check(tree.validate() && tree.size() == 0, "Empty invariant");
    check(tree.stats().height == -1 && tree.stats().averageDepth == 0, "Empty stats");
    check(tree.minimum() == nullptr && tree.maximum() == nullptr, "Empty min/max");
    check(!tree.search("AAA").found && tree.search("AAA").path.empty(), "Empty search");
    check(tree.range("AAA", "ZZZ").visited == 0, "Empty range");
    check(!tree.remove("AAA").removed, "Empty delete"); validateMutation(tree);
    check(tree.insert(example("MMM")), "I1 empty-root insertion"); validateMutation(tree);
    check(tree.insert(example("FFF")), "I2 left insertion"); validateMutation(tree);
    check(tree.insert(example("TTT")), "I3 right insertion"); validateMutation(tree);
    Airport duplicate = example("MMM"); duplicate.name = "Must not replace";
    check(!tree.insert(duplicate), "I4 duplicate rejected"); validateMutation(tree);
    check(tree.lookup("MMM")->name == "Test MMM" && tree.size() == 3, "Duplicate preserves record");
    check(tree.insert(example("JJJ")), "Insert descendant"); validateMutation(tree);
    // Deliberately corrupt an ancestor bound while preserving JJJ's parent bound.
    Airport* bad = const_cast<Airport*>(tree.lookup("JJJ"));
    bad->code = "NNN"; // Right of FFF, but illegally left of root MMM.
    check(!tree.validate(), "Validator must detect nonlocal ancestor violation");
    bad->code = "JJJ";
    check(tree.validate(), "Restore deliberate validator corruption");
    std::vector<Airport> invalid = {example("BBB"), example("AAA")};
    std::vector<std::string> old = tree.shapeSignature();
    throws([&]() { tree.buildBalanced(invalid); }, "unsorted balanced input");
    check(tree.shapeSignature() == old, "Failed balance preserves tree");
    throws([&]() { tree.insert(example("AB")); }, "invalid insert key");
    validateMutation(tree);
    AirportBST left;
    check(left.insert(example("MMM")), "Left root insert"); validateMutation(left);
    check(left.insert(example("FFF")), "Left child insert"); validateMutation(left);
    check(left.remove("MMM").caseName == "D2: only a left child", "Left root deletion");
    validateMutation(left);
    check(left.rootCode() == "FFF", "Left child promoted to root");
    AirportBST right;
    check(right.insert(example("MMM")), "Right root insert"); validateMutation(right);
    check(right.insert(example("TTT")), "Right child insert"); validateMutation(right);
    check(right.remove("MMM").caseName == "D2: only a right child", "Right root deletion");
    validateMutation(right);
    check(right.remove("TTT").caseName == "D1: leaf", "Final root deletion"); validateMutation(right);
    check(right.size() == 0 && right.rootCode().empty(), "Delete last node");
    AirportBST successor;
    std::vector<Airport> nodes = {example("MMM"), example("FFF"), example("TTT"),
                                  example("PPP"), example("RRR")};
    buildChecked(successor, nodes);
    DeleteResult removed = successor.remove("MMM"); validateMutation(successor);
    check(removed.caseName == "D3: two children" && removed.replacement == "PPP", "D3 successor");
    check(successor.lookup("RRR") != nullptr, "Successor's right child reattached");
    check(successor.lookup("PPP")->name == "Test PPP", "D3 copies full record");
    std::cout << "PASS: empty trees, I1-I4, root deletions, D3 successor child, ancestor bounds\n";
}

void testParser() {
    std::vector<std::string> fields = splitCSV("1,\"Name, with \"\"quotes\"\"\",City,,DU9");
    check(fields.size() == 5 && fields[1] == "Name, with \"quotes\"" &&
          fields[3].empty() && fields[4] == "DU9", "Quoted CSV parse");
    check(splitCSV("a,b,").size() == 3, "Trailing empty field");
    throws([]() { splitCSV("\"broken"); }, "unterminated quote");
    throws([]() { splitCSV("\"ok\"extra"); }, "text after closing quote");
    throws([]() { splitCSV("ab\"cd"); }, "quote inside unquoted field");
    std::ofstream fixture(scratch + "fixture.dat", std::ios::binary);
    fixture << "1,\"Airport, \"\"North\"\"\",City,Country,QQQ,TEST,1,2,0,0,U,TZ,airport,test\r\n"
            << "2,Missing,City,Country,\\N,TEST,1,2,0,0,U,TZ,airport,test\r\n"
            << "3,Duplicate,City,Country,QQQ,TEST,1,2,0,0,U,TZ,airport,test\r\n"
            << "4,Dunnville,City,Country,DU9,CDU9,1,2,0,0,U,TZ,airport,test\r\n";
    fixture.close();
    AirportBST tree;
    std::vector<Airport> catalog;
    LoadSummary summary = loadTree(scratch + "fixture.dat", tree, catalog);
    check(summary.lines == 4 && summary.skippedMissing == 1 && summary.inserted == 2 &&
          summary.duplicates == std::vector<std::string>{"QQQ"}, "Loader counters/duplicate reporting");
    check(tree.lookup("QQQ")->name == "Airport, \"North\"", "First duplicate record retained");
    check(tree.lookup("DU9") != nullptr, "Alphanumeric course key retained");
    std::vector<std::string> before = tree.shapeSignature();
    std::ofstream malformed(scratch + "bad.dat");
    malformed << "1,Bad,City,Country,AAA,AAAA,nan,2,0,0,U,TZ,airport,test\n";
    malformed.close();
    throws([&]() { loadTree(scratch + "bad.dat", tree, catalog); }, "invalid coordinate");
    check(before == tree.shapeSignature(), "Failed load is transactional");
    std::cout << "PASS: CSV quotes, commas, CRLF, missing IATA, DU9, duplicates, malformed input\n";
}

void testRealData(const DataSet& data) {
    check(data.lines == 7698 && data.skippedMissing == 1626 && data.airports.size() == 6072,
          "Posted data counts");
    std::set<std::string> codes, countries;
    int us = 0;
    for (int i = 0; i < 6072; i++) {
        codes.insert(data.airports[i].code); countries.insert(data.airports[i].country);
        if (data.airports[i].country == "United States") us++;
    }
    check(codes.size() == 6072 && countries.size() == 235 && us == 1251, "Dataset facts");
    DataSet csv = readAirportData("data/airports.csv");
    check(csv.lines == data.lines && csv.airports.size() == data.airports.size() && csv.headers == 1,
          "Headered CSV loader");
    for (int i = 0; i < 6072; i++) {
        check(csv.airports[i].code == data.airports[i].code, "CSV and DAT key order");
    }
    AirportBST tree;
    buildChecked(tree, data.airports);
    TreeStats stats = tree.stats();
    check(stats.height == 177 && stats.depthSum == 111259, "File-order height and depth sum");
    check(std::abs(stats.averageDepth - 18.32328722002635) < 1e-10, "File-order average");
    std::vector<Airport> ordered = tree.inorder();
    for (int i = 1; i < 6072; i++) check(ordered[i - 1].code < ordered[i].code, "Sorted inorder");
    check(tree.minimum()->code == "AAA" && tree.maximum()->code == "ZZV", "Real min/max");
    const std::string queries[] = {tree.rootCode(), tree.candidate("leaf"), tree.deepestCode(),
                                   "XYZ", "000", "ZZZ"};
    for (int i = 0; i < 6; i++) {
        SearchResult found = tree.search(queries[i]);
        check(found.found == (i < 3), "Search case " + queries[i]);
        int count = 0; tree.lookup(queries[i], &count);
        check(count == static_cast<int>(found.path.size()), "Path/comparison agreement");
        std::cout << "Search case " << queries[i] << ": " << (found.found ? "found" : "not found")
                  << ", comparisons=" << count << '\n';
    }
    check(tree.search("BOS").path.size() == 12, "BOS comparisons");
    RangeResult range = tree.range("BOS", "BWI");
    check(range.airports.size() == 113 && range.visited == 127, "BOS-BWI example");
    check(tree.range("BOS", "BOS").airports.size() == 1, "Inclusive singleton range");
    check(tree.range("XYZ", "XYZ").airports.empty(), "Missing singleton range");
    check(tree.range("ZZZ", "AAA").visited == 0, "Reversed range");
    RangeResult all = tree.range("000", "ZZZ");
    check(all.airports.size() == 6072 && all.visited == 6072, "Full range visits once per node");
    for (int i = 0; i < 113; i++) {
        check(range.airports[i].code >= "BOS" && range.airports[i].code <= "BWI", "Range membership");
    }
    std::vector<std::string> shape = tree.shapeSignature(), level = tree.levelOrder();
    saveTree(scratch + "real.preorder", tree);
    std::vector<Airport> catalog = data.airports;
    LoadSummary loaded = loadTree(scratch + "real.preorder", tree, catalog);
    check(loaded.sameShape && tree.shapeSignature() == shape && tree.levelOrder() == level &&
          tree.stats().height == 177, "Real preorder exact shape round-trip");
    Airport madeUp = example("QQQ"); madeUp.name = "Xiaoxing Tong Airport";
    madeUp.city = "Shanghai"; madeUp.country = "China"; madeUp.icao = "ZQQQ";
    madeUp.latitude = 31.2304; madeUp.longitude = 121.4737;
    check(tree.insert(madeUp), "QQQ add"); validateMutation(tree);
    check(tree.search("QQQ").found, "QQQ find");
    saveTree(scratch + "custom.preorder", tree);
    AirportBST fresh; std::vector<Airport> emptyCatalog;
    loadTree(scratch + "custom.preorder", fresh, emptyCatalog);
    check(fresh.shapeSignature() == tree.shapeSignature(), "Fresh-process sidecar shape");
    check(sameRecord(*fresh.lookup("QQQ"), madeUp), "Fresh-process made-up metadata");
    check(tree.remove("QQQ").removed, "QQQ remove"); validateMutation(tree);
    check(!tree.search("QQQ").found && tree.shapeSignature() == shape, "QQQ original shape restored");
    tree.rebalance(); validateMutation(tree);
    check(tree.stats().height == 12 && tree.inorder().size() == 6072, "Balanced real tree");
    std::cout << "PASS: real data facts, search cases, inorder, pruned range, snapshots, QQQ, rebalance\n";
}

void testRealDeletes(const DataSet& data) {
    const std::string kinds[] = {"leaf", "left", "right", "two", "root"};
    const std::string expected[] = {"D1: leaf", "D2: only a left child", "D2: only a right child",
                                    "D3: two children", "D3: two children"};
    for (int i = 0; i < 5; i++) {
        AirportBST tree; buildChecked(tree, data.airports);
        std::string code = kinds[i] == "root" ? tree.rootCode() : tree.candidate(kinds[i]);
        if (kinds[i] == "two") code = "AEY"; // A two-child node distinct from root GKA.
        check(!code.empty(), "Missing real deletion case");
        std::vector<Airport> before = tree.inorder();
        std::cout << "\nDelete " << kinds[i] << ' ' << code << " BEFORE:\n";
        tree.printLevels(std::cout);
        DeleteResult result = tree.remove(code); validateMutation(tree);
        std::cout << "AFTER (" << result.caseName << "):\n";
        tree.printLevels(std::cout);
        check(result.removed && result.caseName == expected[i], "Wrong deletion case");
        check(tree.size() == 6071 && tree.lookup(code) == nullptr, "Delete key/size");
        std::vector<Airport> after = tree.inorder();
        int position = 0;
        for (int j = 0; j < 6072; j++) {
            if (before[j].code == code) continue;
            check(sameRecord(before[j], after[position]), "Delete preserves every other full record");
            position++;
        }
        check(!tree.remove(code).removed, "Repeated delete absent key"); validateMutation(tree);
        std::cout << "validate: OK\n";
    }
    std::cout << "PASS: all five real deletion scenarios with top three levels\n";
}

void testDegenerate(const DataSet& data) {
    std::vector<Airport> sorted = data.airports;
    std::sort(sorted.begin(), sorted.end(), [](const Airport& a, const Airport& b) { return a.code < b.code; });
    AirportBST chain; buildChecked(chain, sorted);
    check(chain.stats().height == 6071 && chain.stats().averageDepth == 3035.5, "Sorted chain metrics");
    check(chain.search("ZZV").path.size() == 6072, "Deep iterative search");
    check(chain.inorder().size() == 6072 && chain.preorder().size() == 6072, "Deep iterative traversals");
    check(chain.shapeSignature().size() == 12145, "Deep iterative shape");
    check(chain.range("000", "ZZZ").airports.size() == 6072, "Deep iterative range");
    check(chain.remove("AAA").removed, "Deep root remove"); validateMutation(chain);
    check(chain.remove("ZZV").removed, "Deep leaf remove"); validateMutation(chain);
    chain.clear(); validateMutation(chain);
    check(chain.size() == 0, "Iterative cleanup");
    std::cout << "PASS: 6,072-node sorted chain, height 6,071, no recursive stack dependency\n";
}

void testSnapshotFailures(const DataSet& data) {
    AirportBST tree;
    check(tree.insert(example("MMM")), "Snapshot root"); validateMutation(tree);
    check(tree.insert(example("FFF")), "Snapshot child"); validateMutation(tree);
    const std::vector<std::string> before = tree.shapeSignature();
    std::vector<Airport> catalog = data.airports;
    std::ofstream unknown(scratch + "unknown.preorder"); unknown << "QQQ\n"; unknown.close();
    throws([&]() { loadTree(scratch + "unknown.preorder", tree, catalog); }, "unknown saved record");
    check(tree.shapeSignature() == before, "Unknown snapshot preserves tree");
    std::ofstream duplicate(scratch + "duplicate.preorder");
    duplicate << "MMM\nMMM\n"; duplicate.close();
    throws([&]() { loadTree(scratch + "duplicate.preorder", tree, catalog); }, "duplicate snapshot");
    check(tree.shapeSignature() == before, "Duplicate snapshot preserves tree");
    throws([&]() { loadTree(scratch + "does-not-exist", tree, catalog); }, "missing input");
    throws([&]() { saveTree(scratch + "absent-directory/tree", tree); }, "unwritable output");
    std::ofstream plain(scratch + "plain.preorder"); plain << "MMM\nFFF\n"; plain.close();
    check(loadTree(scratch + "plain.preorder", tree, catalog).sameShape, "Code-only current-record fallback");
    AirportBST empty; saveTree(scratch + "empty.preorder", empty);
    loadTree(scratch + "empty.preorder", tree, catalog);
    check(tree.size() == 0 && tree.validate(), "Empty snapshot round-trip");
    std::cout << "PASS: snapshot failure rollback, missing metadata/files, plain codes, empty snapshots\n";
}

void testRandomized() {
    AirportBST tree;
    // std::map is only a test oracle; src/ implements its own node-based BST.
    std::map<std::string, Airport> oracle;
    std::mt19937 generator(7205);
    for (int operation = 0; operation < 3000; operation++) {
        unsigned int index = generator() % 676;
        std::string code = "A";
        code += static_cast<char>('A' + index / 26);
        code += static_cast<char>('A' + index % 26);
        if (generator() % 2 == 0) {
            Airport a = example(code);
            bool expected = oracle.emplace(code, a).second;
            check(tree.insert(a) == expected, "Random insertion result"); validateMutation(tree);
        } else {
            bool expected = oracle.erase(code) != 0;
            check(tree.remove(code).removed == expected, "Random deletion result"); validateMutation(tree);
        }
        check(tree.size() == static_cast<int>(oracle.size()), "Random size agreement");
        if (operation % 23 == 0) {
            std::vector<Airport> actual = tree.inorder();
            auto expected = oracle.begin();
            int count = static_cast<int>(actual.size());
            for (int i = 0; i < count; i++) {
                check(expected != oracle.end() && sameRecord(actual[i], expected->second), "Random full records");
                ++expected;
            }
            RangeResult range = tree.range("AFF", "AMM");
            int matches = 0;
            auto it = oracle.lower_bound("AFF");
            while (it != oracle.end() && it->first <= "AMM") {
                check(matches < static_cast<int>(range.airports.size()) &&
                      range.airports[matches].code == it->first, "Random range oracle");
                matches++; ++it;
            }
            check(matches == static_cast<int>(range.airports.size()), "Random range size");
        }
    }
    std::cout << "PASS: 3,000 mixed insert/delete operations compared with independent std::map oracle\n";
}
}

int main() {
    try {
        std::filesystem::create_directories(scratch);
        testSmallTrees();
        testParser();
        DataSet data = readAirportData("data/airports.dat");
        testRealData(data);
        testRealDeletes(data);
        testDegenerate(data);
        testSnapshotFailures(data);
        testRandomized();
        std::cout << "\nALL TESTS PASSED: " << checks << " checks; " << validatedMutations
                  << " explicit post-mutation validations.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "TEST FAILED: " << error.what() << '\n';
        return 1;
    }
}

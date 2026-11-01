#include "Experiment.h"
#include "AirportBST.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <random>
#include <stdexcept>
#include <string>

// Task 6: identical pre-generated queries for all trees; see pseudocode first.
void runExperiment(const std::vector<Airport>& records, std::ostream& out,
                   std::ostream* csv) {
    if (records.empty()) throw std::runtime_error("Load some airports before experiment");
    std::vector<Airport> sorted = records;
    std::sort(sorted.begin(), sorted.end(), [](const Airport& a, const Airport& b) {
        return a.code < b.code;
    });
    int n = static_cast<int>(sorted.size());
    for (int i = 1; i < n; i++) {
        if (sorted[i - 1].code == sorted[i].code) {
            throw std::runtime_error("Experiment requires unique input codes");
        }
    }
    std::mt19937 queryGenerator(2026);
    std::uniform_int_distribution<int> choose(0, n - 1);
    std::vector<std::string> queries;
    for (int i = 0; i < 10000; i++) queries.push_back(sorted[choose(queryGenerator)].code);
    const std::string names[] = {"File order", "Sorted order", "Balanced", "Random shuffle"};
    out << "10,000 successful searches; seed=2026; median of 7 batches; milliseconds.\n"
        << "Build order,Height,Average depth,Search ms,Mean comparisons\n";
    if (csv != nullptr) {
        *csv << "build_order,height,average_depth,searches,median_ms,mean_comparisons,seed,repeats\n";
    }
    for (int kind = 0; kind < 4; kind++) {
        AirportBST tree;
        if (kind == 2) tree.buildBalanced(sorted);
        else {
            std::vector<Airport> ordered = kind == 0 ? records : sorted;
            if (kind == 3) {
                std::mt19937 shuffleGenerator(2026);
                std::shuffle(ordered.begin(), ordered.end(), shuffleGenerator);
            }
            for (int i = 0; i < n; i++) tree.insert(ordered[i]);
        }
        if (!tree.validate()) throw std::runtime_error("Experiment tree is invalid");
        TreeStats stats = tree.stats();
        volatile std::uint64_t checksum = 0; // Make the work observable to -O2.
        for (int i = 0; i < 1000; i++) {
            const Airport* result = tree.lookup(queries[i]);
            checksum += static_cast<unsigned char>(result->code[0]);
        }
        std::vector<double> times;
        for (int repetition = 0; repetition < 7; repetition++) {
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < 10000; i++) {
                const Airport* result = tree.lookup(queries[i]);
                checksum += static_cast<unsigned char>(result->code[0]);
            }
            auto stop = std::chrono::steady_clock::now();
            times.push_back(std::chrono::duration<double, std::milli>(stop - start).count());
        }
        std::sort(times.begin(), times.end());
        std::uint64_t comparisons = 0;
        for (int i = 0; i < 10000; i++) {
            int count = 0;
            tree.lookup(queries[i], &count);
            comparisons += static_cast<std::uint64_t>(count);
        }
        double mean = static_cast<double>(comparisons) / 10000;
        out << names[kind] << ',' << stats.height << ',' << std::fixed
            << std::setprecision(4) << stats.averageDepth << ',' << times[3]
            << ',' << mean << " (checksum=" << checksum << ")\n";
        if (csv != nullptr) {
            *csv << names[kind] << ',' << stats.height << ',' << std::fixed
                 << std::setprecision(6) << stats.averageDepth << ",10000,"
                 << times[3] << ',' << mean << ",2026,7\n";
        }
    }
}

#ifndef DATA_IO_H
#define DATA_IO_H

#include "AirportBST.h"

#include <string>
#include <vector>

struct DataSet {
    std::vector<Airport> airports;
    int lines = 0;
    int skippedMissing = 0;
    int headers = 0;
};

struct LoadSummary {
    bool dataFile = false;
    int lines = 0;
    int skippedMissing = 0;
    int inserted = 0;
    std::vector<std::string> duplicates;
    bool sameShape = false;
};

std::vector<std::string> splitCSV(const std::string& line);
DataSet readAirportData(const std::string& path);
LoadSummary loadTree(const std::string& path, AirportBST& tree,
                     std::vector<Airport>& catalog);
void saveTree(const std::string& path, const AirportBST& tree);

#endif

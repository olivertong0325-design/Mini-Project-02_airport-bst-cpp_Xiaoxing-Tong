#ifndef AIRPORT_H
#define AIRPORT_H

#include <iosfwd>
#include <string>

// The record stored at each BST node. The code is the unique search key.
class Airport {
public:
    std::string code;
    std::string name;
    std::string city;
    std::string country;
    std::string icao;
    double latitude = 0.0;
    double longitude = 0.0;

    void print(std::ostream& out) const;
};

bool validCode(const std::string& code);
std::string uppercaseCode(std::string code);

#endif

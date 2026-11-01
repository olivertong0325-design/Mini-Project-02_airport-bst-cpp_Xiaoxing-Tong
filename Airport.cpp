#include "Airport.h"

#include <iomanip>
#include <ostream>

bool validCode(const std::string& code) {
    // DU9 is present in the course data: keep three-character alphanumeric keys.
    if (code.size() != 3) return false;
    for (int i = 0; i < 3; i++) {
        if (!((code[i] >= 'A' && code[i] <= 'Z') ||
              (code[i] >= '0' && code[i] <= '9'))) return false;
    }
    return true;
}

std::string uppercaseCode(std::string code) {
    int n = static_cast<int>(code.size());
    for (int i = 0; i < n; i++) {
        if (code[i] >= 'a' && code[i] <= 'z') code[i] -= 'a' - 'A';
    }
    return code;
}

void Airport::print(std::ostream& out) const {
    std::ios::fmtflags oldFlags = out.flags();
    std::streamsize oldPrecision = out.precision();
    out << code << "  " << name << ", " << city << ", " << country
        << " | ICAO: " << icao << " | " << std::fixed << std::setprecision(6)
        << latitude << ", " << longitude << '\n';
    out.flags(oldFlags);
    out.precision(oldPrecision);
}

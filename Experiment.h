#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "Airport.h"

#include <iosfwd>
#include <vector>

void runExperiment(const std::vector<Airport>& records, std::ostream& out,
                   std::ostream* csv = nullptr);

#endif

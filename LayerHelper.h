#pragma once

#include <map>
#include <set>
#include <utility>

#include "MetaPresent.h"

namespace lh{

Coords findLB(const MetaPresent &p, const MetaPresentV &placed_presents, const std::set<int> &constx_boundaries, const std::set<int> &consty_boundaries, std::pair<int, int> layer_size);
//find Coords closest to (0, 0) where p fits

Coords findRT(const MetaPresent &p, const MetaPresentV &placed_presents, std::set<int> &constx_boundaries, std::set<int> &consty_boundaries, std::pair<int, int> layer_size);
//find Coords closest to (1000, 1000) where p fits


}
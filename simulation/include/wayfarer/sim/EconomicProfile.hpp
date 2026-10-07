// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

namespace wayfarer::sim {

struct IndustryTag {
	std::string key; // "mining", "industrial"
	double intensity = 1.0;
};

/// Metadata attached to a Pioneer location (planet/station).
struct EconomicProfile {
	double population = 0.0;
	double wealth = 1.0;
	std::string government;
	std::vector<IndustryTag> industries;
	std::vector<std::string> resources; // commodity keys produced locally
	std::vector<std::string> exports;
	std::vector<std::string> imports;
};

} // namespace wayfarer::sim

// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "EconomicProfile.hpp"
#include "Market.hpp"
#include "Types.hpp"

#include <string>

namespace wayfarer::sim {

/// Simulation-side location. Bound to Pioneer via pioneer_body_id / path.
struct WorldLocation {
	LocationId id = InvalidLocationId;
	std::string key;  // "mining_world"
	std::string name; // "Mining World"
	std::string kind; // "planet", "station"

	/// Pioneer binding (opaque to simulation). Empty when headless-only.
	std::string pioneer_body_id;
	std::string pioneer_system;

	EconomicProfile economy;
	Market market;
	FactionId controlling_faction = InvalidFactionId;

	/// Travel time to another location (sim seconds). Stage 0: simple map.
	double DistanceTo(const WorldLocation &other) const;
};

} // namespace wayfarer::sim

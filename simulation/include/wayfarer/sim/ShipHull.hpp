// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

namespace wayfarer::sim {

/// Minimal Stage 0 hull. Extensible via extra numeric fields later.
struct ShipHull {
	std::string key;   // "merchant_freighter"
	std::string name;  // "TSV Merchant"
	std::string role;  // "trader", "player", ...

	double mass = 100.0;
	double cargo_capacity = 40.0;
	double fuel_capacity = 20.0;
	double engine_power = 1.0;
	double shield_strength = 100.0;
	double weapon_power = 0.0;
	double hull_strength = 200.0;
	double crew_capacity = 2.0;
	double outfit_capacity = 20.0;

	/// Optional Pioneer ship model name for visual binding.
	std::string pioneer_model;
};

struct Outfit {
	std::string key;
	std::string name;
	std::string category; // engine, weapon, shield, generator, utility
	double mass = 0.0;
	double outfit_space = 1.0;
	double engine_power = 0.0;
	double shield_strength = 0.0;
	double weapon_power = 0.0;
	double cargo_bonus = 0.0;
};

/// Effective stats after installing outfits on a hull.
struct EffectiveShipStats {
	double mass = 0.0;
	double cargo_capacity = 0.0;
	double fuel_capacity = 0.0;
	double engine_power = 0.0;
	double shield_strength = 0.0;
	double weapon_power = 0.0;
	double hull_strength = 0.0;
};

EffectiveShipStats ComputeStats(const ShipHull &hull, const std::vector<Outfit> &outfits);

} // namespace wayfarer::sim

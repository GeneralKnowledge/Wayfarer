// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/ShipHull.hpp"

namespace wayfarer::sim {

EffectiveShipStats ComputeStats(const ShipHull &hull, const std::vector<Outfit> &outfits)
{
	EffectiveShipStats s;
	s.mass = hull.mass;
	s.cargo_capacity = hull.cargo_capacity;
	s.fuel_capacity = hull.fuel_capacity;
	s.engine_power = hull.engine_power;
	s.shield_strength = hull.shield_strength;
	s.weapon_power = hull.weapon_power;
	s.hull_strength = hull.hull_strength;

	for (const auto &o : outfits) {
		s.mass += o.mass;
		s.engine_power += o.engine_power;
		s.shield_strength += o.shield_strength;
		s.weapon_power += o.weapon_power;
		s.cargo_capacity += o.cargo_bonus;
	}
	return s;
}

} // namespace wayfarer::sim

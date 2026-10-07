// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/WorldLocation.hpp"

#include <cmath>

namespace wayfarer::sim {

double WorldLocation::DistanceTo(const WorldLocation &other) const
{
	if (id == other.id) {
		return 0.0;
	}
	// Stage 0: fixed inter-location travel time based on name hash mix.
	// Keeps demos deterministic without needing Pioneer coordinates yet.
	const auto mix = [](const std::string &a, const std::string &b) {
		std::uint64_t h = 14695981039346656037ULL;
		for (char c : a) {
			h ^= static_cast<unsigned char>(c);
			h *= 1099511628211ULL;
		}
		h ^= '|';
		for (char c : b) {
			h ^= static_cast<unsigned char>(c);
			h *= 1099511628211ULL;
		}
		return h;
	};
	const double base = 30.0 + static_cast<double>(mix(key, other.key) % 90);
	return base;
}

} // namespace wayfarer::sim

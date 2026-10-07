// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "WorldSimulation.hpp"

#include <string>
#include <unordered_map>

namespace wayfarer::sim {

/// Boundary between Pioneer runtime objects and the headless simulation.
/// Stage 0 keeps this as an ID/path map — no renderer types.
class PioneerWorldAdapter {
public:
	void BindLocation(LocationId sim_id, std::string pioneer_system, std::string pioneer_body_id);
	void BindShip(ShipId sim_id, std::string pioneer_ship_ref);

	const std::string *PioneerBodyFor(LocationId id) const;
	const std::string *PioneerShipFor(ShipId id) const;
	LocationId LocationForPioneerBody(const std::string &body_id) const;
	ShipId ShipForPioneerRef(const std::string &ref) const;

	/// Push a suggested NPC travel progress into presentation (0..1).
	struct NpcVisualState {
		ShipId ship = InvalidShipId;
		LocationId from = InvalidLocationId;
		LocationId to = InvalidLocationId;
		double progress = 0.0; // 0 at origin, 1 at destination
		std::string name;
		TraderStatus status = TraderStatus::Idle;
	};

	std::vector<NpcVisualState> CollectNpcVisuals(const WorldSimulation &sim) const;

private:
	std::unordered_map<LocationId, std::pair<std::string, std::string>> m_location_bind;
	std::unordered_map<ShipId, std::string> m_ship_bind;
	std::unordered_map<std::string, LocationId> m_body_to_location;
	std::unordered_map<std::string, ShipId> m_ref_to_ship;
};

} // namespace wayfarer::sim

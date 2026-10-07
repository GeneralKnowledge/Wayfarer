// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/PioneerWorldAdapter.hpp"

namespace wayfarer::sim {

void PioneerWorldAdapter::BindLocation(LocationId sim_id, std::string pioneer_system, std::string pioneer_body_id)
{
	m_location_bind[sim_id] = { std::move(pioneer_system), pioneer_body_id };
	m_body_to_location[pioneer_body_id] = sim_id;
}

void PioneerWorldAdapter::BindShip(ShipId sim_id, std::string pioneer_ship_ref)
{
	m_ship_bind[sim_id] = pioneer_ship_ref;
	m_ref_to_ship[pioneer_ship_ref] = sim_id;
}

const std::string *PioneerWorldAdapter::PioneerBodyFor(LocationId id) const
{
	auto it = m_location_bind.find(id);
	return it == m_location_bind.end() ? nullptr : &it->second.second;
}

const std::string *PioneerWorldAdapter::PioneerShipFor(ShipId id) const
{
	auto it = m_ship_bind.find(id);
	return it == m_ship_bind.end() ? nullptr : &it->second;
}

LocationId PioneerWorldAdapter::LocationForPioneerBody(const std::string &body_id) const
{
	auto it = m_body_to_location.find(body_id);
	return it == m_body_to_location.end() ? InvalidLocationId : it->second;
}

ShipId PioneerWorldAdapter::ShipForPioneerRef(const std::string &ref) const
{
	auto it = m_ref_to_ship.find(ref);
	return it == m_ref_to_ship.end() ? InvalidShipId : it->second;
}

std::vector<PioneerWorldAdapter::NpcVisualState> PioneerWorldAdapter::CollectNpcVisuals(const WorldSimulation &sim) const
{
	std::vector<NpcVisualState> out;
	for (const auto &ship : sim.GetShips()) {
		if (!ship.is_npc_trader) {
			continue;
		}
		NpcVisualState v;
		v.ship = ship.id;
		v.name = ship.name;
		v.status = ship.status;
		v.to = ship.destination.value_or(InvalidLocationId);
		v.from = ship.location;
		if (ship.status == TraderStatus::InTransit && ship.destination) {
			const auto *from = sim.GetLocation(ship.location);
			const auto *to = sim.GetLocation(*ship.destination);
			// During transit, location is still origin until arrival.
			if (ship.active_trade) {
				from = sim.GetLocation(ship.active_trade->origin);
			}
			if (from && to) {
				const double total = from->DistanceTo(*to);
				v.from = from->id;
				v.to = to->id;
				v.progress = total > 0.0 ? 1.0 - (ship.travel_remaining / total) : 1.0;
				if (v.progress < 0.0) {
					v.progress = 0.0;
				}
				if (v.progress > 1.0) {
					v.progress = 1.0;
				}
			}
		} else if (ship.status == TraderStatus::Selling || ship.status == TraderStatus::Idle) {
			v.progress = 1.0;
			v.from = ship.location;
			v.to = ship.location;
		}
		out.push_back(v);
	}
	return out;
}

} // namespace wayfarer::sim

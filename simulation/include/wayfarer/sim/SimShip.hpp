// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "CargoHold.hpp"
#include "ShipHull.hpp"
#include "Types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace wayfarer::sim {

struct TradeOrder {
	CommodityId commodity = InvalidCommodityId;
	double quantity = 0.0;
	LocationId origin = InvalidLocationId;
	LocationId destination = InvalidLocationId;
	Credits expected_profit = 0;
};

struct SimShip {
	ShipId id = InvalidShipId;
	std::string name;
	ShipHull hull;
	std::vector<Outfit> outfits;
	CargoHold cargo;
	FactionId faction = InvalidFactionId;
	LocationId location = InvalidLocationId;
	std::optional<LocationId> destination;
	Credits credits = 0;
	bool is_player = false;
	bool is_npc_trader = false;

	TraderStatus status = TraderStatus::Idle;
	std::optional<TradeOrder> active_trade;
	double travel_remaining = 0.0; // sim seconds until arrival

	/// Optional Pioneer ship body binding.
	std::string pioneer_ship_ref;

	EffectiveShipStats Stats() const { return ComputeStats(hull, outfits); }

	void RefreshCargoCapacity()
	{
		cargo.SetCapacity(Stats().cargo_capacity);
	}
};

} // namespace wayfarer::sim

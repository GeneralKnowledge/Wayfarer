// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Types.hpp"

#include <string>

namespace wayfarer::sim {

struct Contract {
	ContractId id = InvalidContractId;
	std::string title;
	std::string issuer; // faction or station name
	FactionId issuer_faction = InvalidFactionId;
	LocationId origin = InvalidLocationId;
	LocationId destination = InvalidLocationId;
	CommodityId commodity = InvalidCommodityId;
	double quantity = 0.0;
	Credits reward = 0;
	double deadline = 0.0; // absolute sim time
	ContractStatus status = ContractStatus::Available;
	ShipId assignee = InvalidShipId;
};

} // namespace wayfarer::sim

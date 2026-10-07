// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

namespace wayfarer::sim {

using ShipId = std::uint32_t;
using LocationId = std::uint32_t;
using FactionId = std::uint32_t;
using ContractId = std::uint32_t;
using CommodityId = std::uint32_t;
using Credits = std::int64_t;

constexpr ShipId InvalidShipId = 0;
constexpr LocationId InvalidLocationId = 0;
constexpr FactionId InvalidFactionId = 0;
constexpr ContractId InvalidContractId = 0;
constexpr CommodityId InvalidCommodityId = 0;

enum class TraderStatus {
	Idle,
	Buying,
	InTransit,
	Selling,
};

enum class ContractStatus {
	Available,
	Accepted,
	Completed,
	Failed,
	Expired,
};

inline const char *ToString(TraderStatus s)
{
	switch (s) {
	case TraderStatus::Idle: return "Idle";
	case TraderStatus::Buying: return "Buying";
	case TraderStatus::InTransit: return "In Transit";
	case TraderStatus::Selling: return "Selling";
	}
	return "Unknown";
}

inline const char *ToString(ContractStatus s)
{
	switch (s) {
	case ContractStatus::Available: return "Available";
	case ContractStatus::Accepted: return "Accepted";
	case ContractStatus::Completed: return "Completed";
	case ContractStatus::Failed: return "Failed";
	case ContractStatus::Expired: return "Expired";
	}
	return "Unknown";
}

} // namespace wayfarer::sim

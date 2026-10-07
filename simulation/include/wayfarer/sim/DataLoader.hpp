// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "WorldSimulation.hpp"

#include <string>

namespace wayfarer::sim {

/// Load Wayfarer JSON data into a simulation (commodities, factions, ships defs).
/// Format: human-readable JSON, Pioneer-compatible style.
class DataLoader {
public:
	explicit DataLoader(std::string data_root);

	bool LoadAll(WorldSimulation &sim);
	bool LoadCommodities(WorldSimulation &sim);
	bool LoadFactions(WorldSimulation &sim);

	const std::string &LastError() const { return m_error; }

	static ShipHull LoadShipHullFile(const std::string &path, std::string &error);
	static Outfit LoadOutfitFile(const std::string &path, std::string &error);

private:
	std::string m_root;
	std::string m_error;
};

} // namespace wayfarer::sim

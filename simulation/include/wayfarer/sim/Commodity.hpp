// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

namespace wayfarer::sim {

enum class CommodityRole {
	Food,
	RawMaterial,
	RefinedMaterial,
	Fuel,
	Industrial,
	HighTech,
	Luxury,
	Other,
};

struct Commodity {
	CommodityId id = InvalidCommodityId;
	std::string key;   // stable id: "ore", "electronics"
	std::string name;  // display name
	CommodityRole role = CommodityRole::Other;
	double base_price = 100.0;
	double mass_per_unit = 1.0;
};

class CommodityRegistry {
public:
	CommodityId Add(Commodity c);
	const Commodity *FindByKey(const std::string &key) const;
	const Commodity *FindById(CommodityId id) const;
	const std::vector<Commodity> &All() const { return m_items; }
	void Clear();

private:
	std::vector<Commodity> m_items;
	CommodityId m_next = 1;
};

} // namespace wayfarer::sim

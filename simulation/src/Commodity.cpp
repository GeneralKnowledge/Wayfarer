// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/Commodity.hpp"

namespace wayfarer::sim {

CommodityId CommodityRegistry::Add(Commodity c)
{
	c.id = m_next++;
	m_items.push_back(std::move(c));
	return m_items.back().id;
}

const Commodity *CommodityRegistry::FindByKey(const std::string &key) const
{
	for (const auto &c : m_items) {
		if (c.key == key) {
			return &c;
		}
	}
	return nullptr;
}

const Commodity *CommodityRegistry::FindById(CommodityId id) const
{
	for (const auto &c : m_items) {
		if (c.id == id) {
			return &c;
		}
	}
	return nullptr;
}

void CommodityRegistry::Clear()
{
	m_items.clear();
	m_next = 1;
}

} // namespace wayfarer::sim

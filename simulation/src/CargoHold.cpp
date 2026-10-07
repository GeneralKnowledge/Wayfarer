// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/CargoHold.hpp"

namespace wayfarer::sim {

double CargoHold::Used() const
{
	double u = 0.0;
	for (const auto &kv : m_contents) {
		u += kv.second;
	}
	return u;
}

double CargoHold::Quantity(CommodityId id) const
{
	auto it = m_contents.find(id);
	return it == m_contents.end() ? 0.0 : it->second;
}

bool CargoHold::TryAdd(CommodityId id, double qty)
{
	if (qty < 0.0) {
		return false;
	}
	if (Used() + qty > m_capacity + 1e-9) {
		return false;
	}
	m_contents[id] += qty;
	return true;
}

bool CargoHold::TryRemove(CommodityId id, double qty)
{
	if (qty < 0.0) {
		return false;
	}
	auto it = m_contents.find(id);
	if (it == m_contents.end() || it->second + 1e-9 < qty) {
		return false;
	}
	it->second -= qty;
	if (it->second <= 1e-9) {
		m_contents.erase(it);
	}
	return true;
}

void CargoHold::Clear()
{
	m_contents.clear();
}

} // namespace wayfarer::sim

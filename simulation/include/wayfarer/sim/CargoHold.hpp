// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Types.hpp"

#include <unordered_map>

namespace wayfarer::sim {

class CargoHold {
public:
	explicit CargoHold(double capacity = 0.0) : m_capacity(capacity) {}

	double Capacity() const { return m_capacity; }
	void SetCapacity(double c) { m_capacity = c; }

	double Used() const;
	double Free() const { return m_capacity - Used(); }

	double Quantity(CommodityId id) const;
	const std::unordered_map<CommodityId, double> &Contents() const { return m_contents; }

	/// Returns false if cargo would exceed capacity.
	bool TryAdd(CommodityId id, double qty);
	bool TryRemove(CommodityId id, double qty);
	void Clear();

private:
	double m_capacity = 0.0;
	std::unordered_map<CommodityId, double> m_contents;
};

} // namespace wayfarer::sim

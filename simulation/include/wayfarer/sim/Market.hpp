// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Commodity.hpp"
#include "Types.hpp"

#include <unordered_map>

namespace wayfarer::sim {

/// Per-commodity market state at a location.
/// Prices are derived from supply and demand (not static tables).
struct MarketListing {
	CommodityId commodity = InvalidCommodityId;
	double base_price = 100.0;
	double supply = 0.0;
	double demand = 0.0;
	double available_quantity = 0.0;

	/// Buy price paid by a ship purchasing from the market.
	double BuyPrice() const;

	/// Sell price received when selling to the market.
	double SellPrice() const;
};

class Market {
public:
	void SetListing(MarketListing listing);
	MarketListing *Find(CommodityId id);
	const MarketListing *Find(CommodityId id) const;
	const std::unordered_map<CommodityId, MarketListing> &Listings() const { return m_listings; }

	/// Purchase up to `qty` from market; returns units actually bought and total cost.
	struct TradeResult {
		double quantity = 0.0;
		Credits cost = 0;
		bool ok = false;
	};

	TradeResult BuyFromMarket(CommodityId id, double qty);
	TradeResult SellToMarket(CommodityId id, double qty);

	/// Soft production/consumption tick (industries).
	void ApplyProduction(CommodityId id, double produced, double consumed);

private:
	std::unordered_map<CommodityId, MarketListing> m_listings;
};

/// Simple price model:
///   pressure = demand / max(supply, epsilon)
///   high supply + low demand → cheap
///   low supply + high demand → expensive
double PriceFromSupplyDemand(double base_price, double supply, double demand);

} // namespace wayfarer::sim

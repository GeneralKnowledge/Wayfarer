// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/Market.hpp"

#include <algorithm>
#include <cmath>

namespace wayfarer::sim {

double PriceFromSupplyDemand(double base_price, double supply, double demand)
{
	const double s = std::max(supply, 0.1);
	const double d = std::max(demand, 0.1);
	// pressure > 1 → expensive; < 1 → cheap
	const double pressure = d / s;
	// Smooth curve around base price; clamp to avoid extremes in Stage 0.
	const double factor = std::clamp(std::pow(pressure, 0.65), 0.25, 4.0);
	return std::max(1.0, base_price * factor);
}

double MarketListing::BuyPrice() const
{
	return PriceFromSupplyDemand(base_price, supply, demand);
}

double MarketListing::SellPrice() const
{
	// Slightly worse than buy (spread) so markets are not free money machines.
	return PriceFromSupplyDemand(base_price, supply, demand) * 0.92;
}

void Market::SetListing(MarketListing listing)
{
	m_listings[listing.commodity] = listing;
}

MarketListing *Market::Find(CommodityId id)
{
	auto it = m_listings.find(id);
	return it == m_listings.end() ? nullptr : &it->second;
}

const MarketListing *Market::Find(CommodityId id) const
{
	auto it = m_listings.find(id);
	return it == m_listings.end() ? nullptr : &it->second;
}

Market::TradeResult Market::BuyFromMarket(CommodityId id, double qty)
{
	TradeResult r;
	auto *listing = Find(id);
	if (!listing || qty <= 0.0) {
		return r;
	}
	const double taken = std::min(qty, listing->available_quantity);
	if (taken <= 0.0) {
		return r;
	}
	const double unit = listing->BuyPrice();
	r.quantity = taken;
	r.cost = static_cast<Credits>(std::llround(unit * taken));
	r.ok = true;
	listing->available_quantity -= taken;
	listing->supply = std::max(0.0, listing->supply - taken);
	return r;
}

Market::TradeResult Market::SellToMarket(CommodityId id, double qty)
{
	TradeResult r;
	auto *listing = Find(id);
	if (!listing || qty <= 0.0) {
		return r;
	}
	const double unit = listing->SellPrice();
	r.quantity = qty;
	r.cost = static_cast<Credits>(std::llround(unit * qty)); // revenue to seller
	r.ok = true;
	listing->available_quantity += qty;
	listing->supply += qty;
	listing->demand = std::max(0.0, listing->demand - qty * 0.5);
	return r;
}

void Market::ApplyProduction(CommodityId id, double produced, double consumed)
{
	auto *listing = Find(id);
	if (!listing) {
		return;
	}
	listing->supply = std::max(0.0, listing->supply + produced - consumed);
	listing->available_quantity = std::max(0.0, listing->available_quantity + produced - consumed);
	listing->demand = std::max(0.0, listing->demand + consumed * 0.1);
}

} // namespace wayfarer::sim

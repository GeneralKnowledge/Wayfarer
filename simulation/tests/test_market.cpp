// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/Market.hpp"

#include <cmath>
#include <iostream>

using namespace wayfarer::sim;

int test_market()
{
	int fails = 0;

	const double cheap = PriceFromSupplyDemand(100.0, 100.0, 20.0);
	const double expensive = PriceFromSupplyDemand(100.0, 20.0, 100.0);

	if (!(expensive > cheap * 1.5)) {
		std::cerr << "FAIL market: expected high-demand price >> high-supply price ("
				  << expensive << " vs " << cheap << ")\n";
		++fails;
	}

	Market market;
	MarketListing highSupply;
	highSupply.commodity = 1;
	highSupply.base_price = 100;
	highSupply.supply = 100;
	highSupply.demand = 20;
	highSupply.available_quantity = 100;
	market.SetListing(highSupply);

	MarketListing highDemand = highSupply;
	highDemand.commodity = 2;
	highDemand.supply = 20;
	highDemand.demand = 100;
	highDemand.available_quantity = 20;
	market.SetListing(highDemand);

	const double buyCheap = market.Find(1)->BuyPrice();
	const double buyDear = market.Find(2)->BuyPrice();
	if (!(buyDear > buyCheap)) {
		std::cerr << "FAIL market listings pricing\n";
		++fails;
	}

	auto bought = market.BuyFromMarket(1, 10);
	if (!bought.ok || bought.quantity != 10.0) {
		std::cerr << "FAIL market buy quantity\n";
		++fails;
	}
	if (!(market.Find(1)->available_quantity < 100.0)) {
		std::cerr << "FAIL market supply decreased on buy\n";
		++fails;
	}

	if (fails == 0) {
		std::cout << "OK market\n";
	}
	return fails > 0 ? 1 : 0;
}

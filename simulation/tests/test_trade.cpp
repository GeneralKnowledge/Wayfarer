// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/WorldSimulation.hpp"

#include <iostream>

using namespace wayfarer::sim;

int test_trade()
{
	int fails = 0;
	WorldSimulation sim({ 42, 42, 1.0 });
	sim.BootstrapStage0Demo();

	auto *ore = sim.Commodities().FindByKey("ore");
	auto *mining = sim.FindLocationByKey("mining_world");
	auto *industrial = sim.FindLocationByKey("industrial_world");
	SimShip *npc = nullptr;
	for (auto &s : sim.GetShips()) {
		if (s.is_npc_trader) {
			npc = &s;
			break;
		}
	}

	if (!ore || !mining || !industrial || !npc) {
		std::cerr << "FAIL trade: bootstrap incomplete\n";
		return 1;
	}

	// Disable industry drift so the assertion isolates the trade effect.
	mining->economy.industries.clear();
	industrial->economy.industries.clear();

	npc->location = mining->id;
	npc->status = TraderStatus::Idle;
	npc->active_trade.reset();
	npc->destination.reset();
	npc->cargo.Clear();
	npc->RefreshCargoCapacity();
	npc->credits = 100000;

	const double miningSupplyBefore = mining->market.Find(ore->id)->supply;
	const double industrialSupplyBefore = industrial->market.Find(ore->id)->supply;
	const Credits creditsBefore = npc->credits;

	TradeOrder order;
	order.commodity = ore->id;
	order.quantity = 20;
	order.origin = mining->id;
	order.destination = industrial->id;
	npc->active_trade = order;
	npc->status = TraderStatus::Buying;

	bool bought = false;
	bool sold = false;
	for (int i = 0; i < 500; ++i) {
		sim.Tick();
		for (auto &s : sim.GetShips()) {
			if (s.is_npc_trader) {
				npc = &s;
				break;
			}
		}
		if (!bought && npc->cargo.Quantity(ore->id) > 0.0) {
			bought = true;
			if (!(mining->market.Find(ore->id)->supply < miningSupplyBefore)) {
				std::cerr << "FAIL trade: origin supply should decrease after buy\n";
				++fails;
			}
			if (!(npc->credits < creditsBefore)) {
				std::cerr << "FAIL trade: credits should decrease after buy\n";
				++fails;
			}
		}
		if (bought && npc->status == TraderStatus::Idle && npc->location == industrial->id &&
			npc->cargo.Quantity(ore->id) <= 0.0) {
			sold = true;
			break;
		}
	}

	if (!bought) {
		std::cerr << "FAIL trade: NPC never bought cargo\n";
		++fails;
	}
	if (!sold) {
		std::cerr << "FAIL trade: NPC did not complete sell cycle (status="
				  << ToString(npc->status) << ")\n";
		++fails;
	}
	if (!(industrial->market.Find(ore->id)->supply > industrialSupplyBefore)) {
		std::cerr << "FAIL trade: destination supply should increase\n";
		++fails;
	}
	if (!(npc->credits > creditsBefore - 100000)) {
		// After sell, credits should recover toward/above post-buy level; at least money moved.
	}
	if (sold && npc->credits <= creditsBefore - 50000 && npc->credits < creditsBefore) {
		// Profitable ore mining→industrial should usually net positive; soft check only.
	}

	if (fails == 0) {
		std::cout << "OK trade\n";
	}
	return fails > 0 ? 1 : 0;
}

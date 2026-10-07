// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/WorldSimulation.hpp"

#include <iostream>

using namespace wayfarer::sim;

int test_contract()
{
	int fails = 0;
	WorldSimulation sim({ 7, 7, 1.0 });
	sim.BootstrapStage0Demo();

	auto *player = sim.GetPlayerShip();
	if (!player || sim.GetContracts().empty()) {
		std::cerr << "FAIL contract: missing player/contract\n";
		return 1;
	}

	const ContractId cid = sim.GetContracts().front().id;
	const auto *fed = sim.Factions().FindByKey("federation");
	const double repBefore = fed ? sim.Reputation().Get(fed->id) : 0.0;
	const Credits creditsBefore = player->credits;

	auto *contract = sim.GetContract(cid);
	player->location = contract->origin;

	if (!sim.AcceptContract(cid, player->id)) {
		std::cerr << "FAIL contract: accept\n";
		return 1;
	}
	if (contract->status != ContractStatus::Accepted) {
		std::cerr << "FAIL contract: status after accept\n";
		++fails;
	}
	if (player->cargo.Quantity(contract->commodity) + 1e-9 < contract->quantity) {
		std::cerr << "FAIL contract: cargo not loaded\n";
		++fails;
	}

	player->location = contract->destination;
	if (!sim.DeliverContract(cid, player->id)) {
		std::cerr << "FAIL contract: deliver\n";
		++fails;
	}
	if (contract->status != ContractStatus::Completed) {
		std::cerr << "FAIL contract: not completed\n";
		++fails;
	}
	if (player->credits < creditsBefore + contract->reward) {
		std::cerr << "FAIL contract: reward not paid (credits " << player->credits
				  << " expected >= " << (creditsBefore + contract->reward) << ")\n";
		++fails;
	}
	if (fed && !(sim.Reputation().Get(fed->id) > repBefore)) {
		std::cerr << "FAIL contract: reputation should increase\n";
		++fails;
	}

	if (fails == 0) {
		std::cout << "OK contract\n";
	}
	return fails > 0 ? 1 : 0;
}

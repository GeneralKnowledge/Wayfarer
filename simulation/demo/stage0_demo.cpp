// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

/// Stage 0 acceptance demo (headless).
/// Proves: living markets, NPC trader, contracts, reputation, persistent economy.
/// Pioneer 3D presentation consumes the same WorldSimulation via PioneerWorldAdapter.

#include "wayfarer/sim/PioneerWorldAdapter.hpp"
#include "wayfarer/sim/WorldSimulation.hpp"

#include <iostream>

using namespace wayfarer::sim;

int main()
{
	WorldSimulation sim({ /*world*/ 2026, /*sim*/ 77, /*tick*/ 1.0 });
	sim.BootstrapStage0Demo();

	PioneerWorldAdapter adapter;
	for (const auto &loc : sim.GetLocations()) {
		adapter.BindLocation(loc.id, loc.pioneer_system, loc.pioneer_body_id);
	}
	for (const auto &ship : sim.GetShips()) {
		adapter.BindShip(ship.id, ship.is_player ? "player" : ship.name);
	}

	std::cout << "=== WAYFARER STAGE 0 DEMO ===\n";
	std::cout << "Headless world simulation (Pioneer is the visual host).\n\n";
	std::cout << sim.FormatDebugPanel() << "\n";

	auto *ore = sim.Commodities().FindByKey("ore");
	auto *mining = sim.FindLocationByKey("mining_world");
	const double orePriceBefore = mining->market.Find(ore->id)->BuyPrice();
	const double oreSupplyBefore = mining->market.Find(ore->id)->supply;

	std::cout << "--- Advancing simulation (NPC trading) ---\n";
	SimShip *npc = nullptr;
	for (auto &s : sim.GetShips()) {
		if (s.is_npc_trader) {
			npc = &s;
			break;
		}
	}

	bool npcMoved = false;
	bool npcSold = false;
	LocationId npcStart = npc->location;
	for (int t = 0; t < 400; ++t) {
		sim.Tick();
		if (npc->location != npcStart) {
			npcMoved = true;
		}
		if (npcMoved && npc->status == TraderStatus::Idle && npc->location != npcStart) {
			npcSold = true;
			break;
		}
		if (t % 50 == 0) {
			auto visuals = adapter.CollectNpcVisuals(sim);
			for (const auto &v : visuals) {
				std::cout << "  t=" << static_cast<int>(sim.Clock().current_time)
						  << " NPC '" << v.name << "' status=" << ToString(v.status)
						  << " progress=" << v.progress << "\n";
			}
		}
	}

	const double orePriceAfter = mining->market.Find(ore->id)->BuyPrice();
	const double oreSupplyAfter = mining->market.Find(ore->id)->supply;

	std::cout << "\nNPC moved between locations: " << (npcMoved ? "YES" : "NO") << "\n";
	std::cout << "NPC completed a trade cycle: " << (npcSold ? "YES" : "NO") << "\n";
	std::cout << "Mining ore supply: " << oreSupplyBefore << " -> " << oreSupplyAfter << "\n";
	std::cout << "Mining ore buy price: " << orePriceBefore << " -> " << orePriceAfter << "\n";

	std::cout << "\n--- Player contract ---\n";
	auto *player = sim.GetPlayerShip();
	auto &contract = sim.GetContracts().front();
	player->location = contract.origin;
	const Credits before = player->credits;
	const double repBefore = sim.Reputation().Get(contract.issuer_faction);

	if (!sim.AcceptContract(contract.id, player->id)) {
		std::cerr << "Failed to accept contract\n";
		return 1;
	}
	std::cout << "Accepted: " << contract.title << "\n";
	std::cout << "Cargo loaded: " << player->cargo.Quantity(contract.commodity) << "\n";

	// Player "flies" using Pioneer flight in the real game; here we teleport on arrival.
	player->location = contract.destination;
	if (!sim.DeliverContract(contract.id, player->id)) {
		std::cerr << "Failed to deliver contract\n";
		return 1;
	}
	std::cout << "Delivered. Credits: " << before << " -> " << player->credits << "\n";
	std::cout << "Federation reputation: " << repBefore << " -> "
			  << sim.Reputation().Get(contract.issuer_faction) << "\n";

	std::cout << "\n=== FINAL ECONOMY PANEL ===\n";
	std::cout << sim.FormatDebugPanel();

	const bool ok = npcMoved && (oreSupplyAfter != oreSupplyBefore) &&
		contract.status == ContractStatus::Completed &&
		player->credits > before;

	std::cout << "\nSTAGE 0 ACCEPTANCE: " << (ok ? "PASSED" : "FAILED") << "\n";
	return ok ? 0 : 1;
}

// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Commodity.hpp"
#include "Contract.hpp"
#include "DeterministicRng.hpp"
#include "Faction.hpp"
#include "SimShip.hpp"
#include "SimulationClock.hpp"
#include "WorldLocation.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace wayfarer::sim {

struct SimulationConfig {
	std::uint64_t world_seed = 1;
	std::uint64_t simulation_seed = 1;
	double tick_duration = 1.0;
};

/// Headless world simulation: economy, factions, ships, contracts.
/// Does not depend on rendering.
class WorldSimulation {
public:
	explicit WorldSimulation(SimulationConfig config = {});

	void Reset(SimulationConfig config);
	void Tick();
	void Tick(int steps);

	SimulationClock &Clock() { return m_clock; }
	const SimulationClock &Clock() const { return m_clock; }
	DeterministicRng &Rng() { return m_rng; }

	CommodityRegistry &Commodities() { return m_commodities; }
	const CommodityRegistry &Commodities() const { return m_commodities; }
	FactionRegistry &Factions() { return m_factions; }
	const FactionRegistry &Factions() const { return m_factions; }
	ReputationBoard &Reputation() { return m_reputation; }
	const ReputationBoard &Reputation() const { return m_reputation; }

	LocationId AddLocation(WorldLocation loc);
	WorldLocation *GetLocation(LocationId id);
	const WorldLocation *GetLocation(LocationId id) const;
	WorldLocation *FindLocationByKey(const std::string &key);
	const std::vector<WorldLocation> &GetLocations() const { return m_locations; }

	ShipId AddShip(SimShip ship);
	SimShip *GetShip(ShipId id);
	const SimShip *GetShip(ShipId id) const;
	SimShip *GetPlayerShip();
	std::vector<SimShip> &GetShips() { return m_ships; }
	const std::vector<SimShip> &GetShips() const { return m_ships; }

	const std::vector<Contract> &GetContracts() const { return m_contracts; }
	Contract *GetContract(ContractId id);
	ContractId OfferContract(Contract c);

	/// Player API
	bool AcceptContract(ContractId id, ShipId ship);
	bool DeliverContract(ContractId id, ShipId ship);

	/// Manual trade helpers (tests / player station UI).
	bool PlayerBuy(CommodityId commodity, double qty, LocationId market);
	bool PlayerSell(CommodityId commodity, double qty, LocationId market);

	/// Build Stage 0 demo world: mining + industrial locations, NPC, contract.
	void BootstrapStage0Demo();

	/// Snapshot hash for determinism checks (coarse).
	std::uint64_t StateFingerprint() const;

	std::string FormatDebugPanel() const;

private:
	void TickNpcTraders();
	void TickTravel();
	void TickIndustries();
	void EnsureNpcHasTrade(SimShip &ship);
	bool ExecuteBuy(SimShip &ship, LocationId loc, CommodityId c, double qty);
	bool ExecuteSell(SimShip &ship, LocationId loc, CommodityId c, double qty);
	std::optional<TradeOrder> FindBestTrade(const SimShip &ship);

	SimulationConfig m_config;
	SimulationClock m_clock;
	DeterministicRng m_rng;

	CommodityRegistry m_commodities;
	FactionRegistry m_factions;
	ReputationBoard m_reputation;

	std::vector<WorldLocation> m_locations;
	std::vector<SimShip> m_ships;
	std::vector<Contract> m_contracts;

	LocationId m_next_location = 1;
	ShipId m_next_ship = 1;
	ContractId m_next_contract = 1;
};

} // namespace wayfarer::sim

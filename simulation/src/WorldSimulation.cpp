// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/WorldSimulation.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace wayfarer::sim {

WorldSimulation::WorldSimulation(SimulationConfig config)
{
	Reset(config);
}

void WorldSimulation::Reset(SimulationConfig config)
{
	m_config = config;
	m_clock = SimulationClock{};
	m_clock.tick_duration = config.tick_duration;
	m_rng.Seed(config.simulation_seed);

	m_commodities.Clear();
	m_factions.Clear();
	m_reputation.Clear();
	m_locations.clear();
	m_ships.clear();
	m_contracts.clear();
	m_next_location = 1;
	m_next_ship = 1;
	m_next_contract = 1;
}

void WorldSimulation::Tick()
{
	if (m_clock.paused) {
		return;
	}
	TickIndustries();
	TickNpcTraders();
	TickTravel();
	m_clock.Advance();
}

void WorldSimulation::Tick(int steps)
{
	for (int i = 0; i < steps; ++i) {
		Tick();
	}
}

LocationId WorldSimulation::AddLocation(WorldLocation loc)
{
	loc.id = m_next_location++;
	m_locations.push_back(std::move(loc));
	return m_locations.back().id;
}

WorldLocation *WorldSimulation::GetLocation(LocationId id)
{
	for (auto &l : m_locations) {
		if (l.id == id) {
			return &l;
		}
	}
	return nullptr;
}

const WorldLocation *WorldSimulation::GetLocation(LocationId id) const
{
	for (const auto &l : m_locations) {
		if (l.id == id) {
			return &l;
		}
	}
	return nullptr;
}

WorldLocation *WorldSimulation::FindLocationByKey(const std::string &key)
{
	for (auto &l : m_locations) {
		if (l.key == key) {
			return &l;
		}
	}
	return nullptr;
}

ShipId WorldSimulation::AddShip(SimShip ship)
{
	ship.id = m_next_ship++;
	ship.RefreshCargoCapacity();
	m_ships.push_back(std::move(ship));
	return m_ships.back().id;
}

SimShip *WorldSimulation::GetShip(ShipId id)
{
	for (auto &s : m_ships) {
		if (s.id == id) {
			return &s;
		}
	}
	return nullptr;
}

const SimShip *WorldSimulation::GetShip(ShipId id) const
{
	for (const auto &s : m_ships) {
		if (s.id == id) {
			return &s;
		}
	}
	return nullptr;
}

SimShip *WorldSimulation::GetPlayerShip()
{
	for (auto &s : m_ships) {
		if (s.is_player) {
			return &s;
		}
	}
	return nullptr;
}

Contract *WorldSimulation::GetContract(ContractId id)
{
	for (auto &c : m_contracts) {
		if (c.id == id) {
			return &c;
		}
	}
	return nullptr;
}

ContractId WorldSimulation::OfferContract(Contract c)
{
	c.id = m_next_contract++;
	m_contracts.push_back(std::move(c));
	return m_contracts.back().id;
}

bool WorldSimulation::ExecuteBuy(SimShip &ship, LocationId loc, CommodityId c, double qty)
{
	auto *location = GetLocation(loc);
	if (!location || ship.location != loc) {
		return false;
	}
	auto trade = location->market.BuyFromMarket(c, qty);
	if (!trade.ok) {
		return false;
	}
	if (ship.credits < trade.cost) {
		// refund market
		location->market.SellToMarket(c, trade.quantity);
		return false;
	}
	if (!ship.cargo.TryAdd(c, trade.quantity)) {
		location->market.SellToMarket(c, trade.quantity);
		return false;
	}
	ship.credits -= trade.cost;
	return true;
}

bool WorldSimulation::ExecuteSell(SimShip &ship, LocationId loc, CommodityId c, double qty)
{
	auto *location = GetLocation(loc);
	if (!location || ship.location != loc) {
		return false;
	}
	if (!ship.cargo.TryRemove(c, qty)) {
		return false;
	}
	auto trade = location->market.SellToMarket(c, qty);
	if (!trade.ok) {
		ship.cargo.TryAdd(c, qty);
		return false;
	}
	ship.credits += trade.cost;
	return true;
}

bool WorldSimulation::PlayerBuy(CommodityId commodity, double qty, LocationId market)
{
	auto *player = GetPlayerShip();
	if (!player) {
		return false;
	}
	return ExecuteBuy(*player, market, commodity, qty);
}

bool WorldSimulation::PlayerSell(CommodityId commodity, double qty, LocationId market)
{
	auto *player = GetPlayerShip();
	if (!player) {
		return false;
	}
	return ExecuteSell(*player, market, commodity, qty);
}

bool WorldSimulation::AcceptContract(ContractId id, ShipId shipId)
{
	auto *contract = GetContract(id);
	auto *ship = GetShip(shipId);
	if (!contract || !ship) {
		return false;
	}
	if (contract->status != ContractStatus::Available) {
		return false;
	}
	if (ship->location != contract->origin) {
		return false;
	}
	auto *origin = GetLocation(contract->origin);
	if (!origin) {
		return false;
	}

	auto trade = origin->market.BuyFromMarket(contract->commodity, contract->quantity);
	if (!trade.ok || trade.quantity + 1e-9 < contract->quantity) {
		if (trade.ok) {
			origin->market.SellToMarket(contract->commodity, trade.quantity);
		}
		return false;
	}
	// Contract cargo is provided by issuer (mission cargo) — refund purchase cost.
	ship->credits += trade.cost;
	if (!ship->cargo.TryAdd(contract->commodity, contract->quantity)) {
		origin->market.SellToMarket(contract->commodity, contract->quantity);
		ship->credits -= trade.cost;
		return false;
	}

	contract->status = ContractStatus::Accepted;
	contract->assignee = shipId;
	return true;
}

bool WorldSimulation::DeliverContract(ContractId id, ShipId shipId)
{
	auto *contract = GetContract(id);
	auto *ship = GetShip(shipId);
	if (!contract || !ship) {
		return false;
	}
	if (contract->status != ContractStatus::Accepted || contract->assignee != shipId) {
		return false;
	}
	if (ship->location != contract->destination) {
		return false;
	}
	if (!ship->cargo.TryRemove(contract->commodity, contract->quantity)) {
		return false;
	}

	auto *dest = GetLocation(contract->destination);
	if (dest) {
		dest->market.SellToMarket(contract->commodity, contract->quantity);
	}

	ship->credits += contract->reward;
	contract->status = ContractStatus::Completed;

	if (contract->issuer_faction != InvalidFactionId) {
		m_reputation.Adjust(contract->issuer_faction, 5.0);
	}
	return true;
}

std::optional<TradeOrder> WorldSimulation::FindBestTrade(const SimShip &ship)
{
	std::optional<TradeOrder> best;
	Credits bestProfit = 0;

	const double capacity = ship.cargo.Free();
	if (capacity < 1.0) {
		return std::nullopt;
	}

	for (const auto &origin : m_locations) {
		if (origin.id != ship.location) {
			continue;
		}
		for (const auto &dest : m_locations) {
			if (dest.id == origin.id) {
				continue;
			}
			for (const auto &kv : origin.market.Listings()) {
				const auto *buyListing = &kv.second;
				const auto *sellListing = dest.market.Find(kv.first);
				if (!sellListing || buyListing->available_quantity < 1.0) {
					continue;
				}
				const double qty = std::min({ capacity, buyListing->available_quantity, 20.0 });
				if (qty < 1.0) {
					continue;
				}
				const double buy = buyListing->BuyPrice();
				const double sell = sellListing->SellPrice();
				const Credits profit = static_cast<Credits>(std::llround((sell - buy) * qty));
				if (profit > bestProfit) {
					bestProfit = profit;
					TradeOrder order;
					order.commodity = kv.first;
					order.quantity = qty;
					order.origin = origin.id;
					order.destination = dest.id;
					order.expected_profit = profit;
					best = order;
				}
			}
		}
	}
	return best;
}

void WorldSimulation::EnsureNpcHasTrade(SimShip &ship)
{
	if (!ship.is_npc_trader) {
		return;
	}
	if (ship.status != TraderStatus::Idle) {
		return;
	}
	auto order = FindBestTrade(ship);
	if (!order) {
		return;
	}
	ship.active_trade = order;
	ship.status = TraderStatus::Buying;
}

void WorldSimulation::TickNpcTraders()
{
	for (auto &ship : m_ships) {
		if (!ship.is_npc_trader) {
			continue;
		}
		EnsureNpcHasTrade(ship);

		if (ship.status == TraderStatus::Buying && ship.active_trade) {
			const auto &order = *ship.active_trade;
			if (ExecuteBuy(ship, order.origin, order.commodity, order.quantity)) {
				auto *origin = GetLocation(order.origin);
				auto *dest = GetLocation(order.destination);
				ship.destination = order.destination;
				ship.travel_remaining = origin && dest ? origin->DistanceTo(*dest) : 60.0;
				ship.status = TraderStatus::InTransit;
			} else {
				ship.active_trade.reset();
				ship.status = TraderStatus::Idle;
			}
		} else if (ship.status == TraderStatus::Selling && ship.active_trade) {
			const auto &order = *ship.active_trade;
			const double qty = ship.cargo.Quantity(order.commodity);
			if (qty > 0.0 && ExecuteSell(ship, ship.location, order.commodity, qty)) {
				ship.active_trade.reset();
				ship.destination.reset();
				ship.status = TraderStatus::Idle;
			} else {
				ship.active_trade.reset();
				ship.status = TraderStatus::Idle;
			}
		}
	}
}

void WorldSimulation::TickTravel()
{
	for (auto &ship : m_ships) {
		if (ship.status != TraderStatus::InTransit || !ship.destination) {
			continue;
		}
		ship.travel_remaining -= m_clock.tick_duration * m_clock.time_scale;
		if (ship.travel_remaining <= 0.0) {
			ship.location = *ship.destination;
			ship.travel_remaining = 0.0;
			ship.status = ship.is_npc_trader ? TraderStatus::Selling : TraderStatus::Idle;
		}
	}
}

void WorldSimulation::TickIndustries()
{
	for (auto &loc : m_locations) {
		for (const auto &industry : loc.economy.industries) {
			const double intensity = industry.intensity * loc.economy.wealth;
			if (industry.key == "mining") {
				if (const auto *ore = m_commodities.FindByKey("ore")) {
					loc.market.ApplyProduction(ore->id, 0.4 * intensity, 0.0);
				}
				if (const auto *metal = m_commodities.FindByKey("metal")) {
					loc.market.ApplyProduction(metal->id, 0.2 * intensity, 0.05 * intensity);
				}
			} else if (industry.key == "industrial") {
				if (const auto *ore = m_commodities.FindByKey("ore")) {
					loc.market.ApplyProduction(ore->id, 0.0, 0.35 * intensity);
				}
				if (const auto *metal = m_commodities.FindByKey("metal")) {
					loc.market.ApplyProduction(metal->id, 0.0, 0.25 * intensity);
				}
				if (const auto *mach = m_commodities.FindByKey("machinery")) {
					loc.market.ApplyProduction(mach->id, 0.25 * intensity, 0.0);
				}
				if (const auto *elec = m_commodities.FindByKey("electronics")) {
					loc.market.ApplyProduction(elec->id, 0.15 * intensity, 0.0);
				}
			}
		}
	}
}

static CommodityRole RoleFromString(const std::string &s)
{
	if (s == "food") return CommodityRole::Food;
	if (s == "raw") return CommodityRole::RawMaterial;
	if (s == "refined") return CommodityRole::RefinedMaterial;
	if (s == "fuel") return CommodityRole::Fuel;
	if (s == "industrial") return CommodityRole::Industrial;
	if (s == "high_tech") return CommodityRole::HighTech;
	if (s == "luxury") return CommodityRole::Luxury;
	return CommodityRole::Other;
}

void WorldSimulation::BootstrapStage0Demo()
{
	Reset(m_config);

	struct CDef {
		const char *key;
		const char *name;
		const char *role;
		double price;
	};
	const CDef defs[] = {
		{ "food", "Food", "food", 80 },
		{ "ore", "Ore", "raw", 100 },
		{ "metal", "Metal", "refined", 160 },
		{ "fuel", "Fuel", "fuel", 70 },
		{ "machinery", "Machinery", "industrial", 320 },
		{ "electronics", "Electronics", "high_tech", 480 },
		{ "luxury_goods", "Luxury Goods", "luxury", 600 },
	};
	for (const auto &d : defs) {
		Commodity c;
		c.key = d.key;
		c.name = d.name;
		c.role = RoleFromString(d.role);
		c.base_price = d.price;
		m_commodities.Add(c);
	}

	Faction federation;
	federation.key = "federation";
	federation.name = "Federation";
	federation.colour = { 0.2f, 0.45f, 0.9f };
	federation.government = "representative_democracy";
	federation.economic_strength = 1.2;
	federation.military_strength = 1.1;
	federation.territory = { "industrial_world", "frontier_station" };
	const auto fedId = m_factions.Add(federation);

	Faction miningUnion;
	miningUnion.key = "mining_union";
	miningUnion.name = "Mining Union";
	miningUnion.colour = { 0.75f, 0.55f, 0.15f };
	miningUnion.government = "syndicate";
	miningUnion.economic_strength = 0.9;
	miningUnion.military_strength = 0.4;
	miningUnion.territory = { "mining_world" };
	const auto unionId = m_factions.Add(miningUnion);

	Faction pirates;
	pirates.key = "pirates";
	pirates.name = "Pirates";
	pirates.colour = { 0.7f, 0.1f, 0.1f };
	pirates.government = "anarchy";
	pirates.economic_strength = 0.3;
	pirates.military_strength = 0.8;
	m_factions.Add(pirates);

	Faction frontier;
	frontier.key = "frontier_league";
	frontier.name = "Frontier League";
	frontier.colour = { 0.3f, 0.7f, 0.4f };
	frontier.government = "confederation";
	frontier.economic_strength = 0.7;
	frontier.military_strength = 0.6;
	frontier.territory = { "frontier_station" };
	const auto frontierId = m_factions.Add(frontier);

	m_reputation.Set(fedId, 20.0);
	m_reputation.Set(unionId, 5.0);
	m_reputation.Set(m_factions.FindByKey("pirates")->id, -40.0);
	m_reputation.Set(frontierId, 10.0);

	auto addListing = [&](WorldLocation &loc, const char *key, double supply, double demand, double qty) {
		const auto *c = m_commodities.FindByKey(key);
		if (!c) {
			return;
		}
		MarketListing listing;
		listing.commodity = c->id;
		listing.base_price = c->base_price;
		listing.supply = supply;
		listing.demand = demand;
		listing.available_quantity = qty;
		loc.market.SetListing(listing);
	};

	WorldLocation mining;
	mining.key = "mining_world";
	mining.name = "Mining World";
	mining.kind = "planet";
	mining.pioneer_system = "New Albion";
	mining.pioneer_body_id = "new_albion/mining_world";
	mining.controlling_faction = unionId;
	mining.economy.population = 120000;
	mining.economy.wealth = 0.8;
	mining.economy.government = "syndicate";
	mining.economy.industries = { { "mining", 1.2 } };
	mining.economy.resources = { "ore", "metal" };
	mining.economy.exports = { "ore", "metal" };
	mining.economy.imports = { "machinery", "electronics", "food" };
	addListing(mining, "ore", 120, 25, 120);
	addListing(mining, "metal", 80, 30, 80);
	addListing(mining, "fuel", 60, 40, 60);
	addListing(mining, "food", 30, 70, 30);
	addListing(mining, "machinery", 10, 90, 10);
	addListing(mining, "electronics", 5, 95, 5);
	addListing(mining, "luxury_goods", 2, 40, 2);
	const auto miningId = AddLocation(std::move(mining));

	WorldLocation industrial;
	industrial.key = "industrial_world";
	industrial.name = "Industrial World";
	industrial.kind = "planet";
	industrial.pioneer_system = "New Albion";
	industrial.pioneer_body_id = "new_albion/industrial_world";
	industrial.controlling_faction = fedId;
	industrial.economy.population = 850000;
	industrial.economy.wealth = 1.3;
	industrial.economy.government = "representative_democracy";
	industrial.economy.industries = { { "industrial", 1.4 } };
	industrial.economy.resources = { "machinery", "electronics" };
	industrial.economy.exports = { "machinery", "electronics" };
	industrial.economy.imports = { "ore", "metal", "fuel" };
	addListing(industrial, "ore", 20, 110, 20);
	addListing(industrial, "metal", 25, 100, 25);
	addListing(industrial, "fuel", 40, 60, 40);
	addListing(industrial, "food", 50, 55, 50);
	addListing(industrial, "machinery", 90, 35, 90);
	addListing(industrial, "electronics", 70, 40, 70);
	addListing(industrial, "luxury_goods", 25, 50, 25);
	const auto industrialId = AddLocation(std::move(industrial));

	WorldLocation frontierStation;
	frontierStation.key = "frontier_station";
	frontierStation.name = "Frontier Station";
	frontierStation.kind = "station";
	frontierStation.pioneer_system = "New Albion";
	frontierStation.pioneer_body_id = "new_albion/frontier_station";
	frontierStation.controlling_faction = frontierId;
	frontierStation.economy.population = 18000;
	frontierStation.economy.wealth = 0.9;
	frontierStation.economy.government = "confederation";
	frontierStation.economy.industries = {};
	frontierStation.economy.imports = { "electronics", "food", "machinery" };
	frontierStation.economy.exports = { "fuel" };
	addListing(frontierStation, "ore", 15, 40, 15);
	addListing(frontierStation, "metal", 18, 45, 18);
	addListing(frontierStation, "fuel", 70, 30, 70);
	addListing(frontierStation, "food", 20, 80, 20);
	addListing(frontierStation, "machinery", 12, 70, 12);
	addListing(frontierStation, "electronics", 8, 100, 8);
	addListing(frontierStation, "luxury_goods", 10, 60, 10);
	const auto frontierLocId = AddLocation(std::move(frontierStation));

	ShipHull freighter;
	freighter.key = "merchant_freighter";
	freighter.name = "Merchant Freighter";
	freighter.role = "trader";
	freighter.mass = 220;
	freighter.cargo_capacity = 40;
	freighter.fuel_capacity = 30;
	freighter.engine_power = 1.2;
	freighter.shield_strength = 150;
	freighter.weapon_power = 0.2;
	freighter.hull_strength = 320;
	freighter.pioneer_model = "lodos";

	Outfit engine;
	engine.key = "basic_engine";
	engine.name = "Basic Engine";
	engine.category = "engine";
	engine.mass = 8;
	engine.engine_power = 0.4;

	SimShip npc;
	npc.name = "TSV Merchant";
	npc.hull = freighter;
	npc.outfits = { engine };
	npc.faction = fedId;
	npc.location = miningId;
	npc.credits = 50000;
	npc.is_npc_trader = true;
	npc.RefreshCargoCapacity();
	AddShip(std::move(npc));

	ShipHull playerHull = freighter;
	playerHull.key = "player_courier";
	playerHull.name = "Courier";
	playerHull.role = "player";
	playerHull.cargo_capacity = 30;
	playerHull.pioneer_model = "kanara";

	SimShip player;
	player.name = "Player";
	player.hull = playerHull;
	player.outfits = { engine };
	player.faction = fedId;
	player.location = industrialId;
	player.credits = 25000;
	player.is_player = true;
	player.RefreshCargoCapacity();
	AddShip(std::move(player));

	Contract contract;
	contract.title = "Deliver Electronics";
	contract.issuer = "Industrial World Port Authority";
	contract.issuer_faction = fedId;
	contract.origin = industrialId;
	contract.destination = frontierLocId;
	contract.commodity = m_commodities.FindByKey("electronics")->id;
	contract.quantity = 20;
	contract.reward = 12000;
	contract.deadline = m_clock.current_time + 3600.0;
	contract.status = ContractStatus::Available;
	OfferContract(std::move(contract));

	(void)miningId;
}

std::uint64_t WorldSimulation::StateFingerprint() const
{
	std::uint64_t h = 14695981039346656037ULL;
	auto mix = [&](std::uint64_t v) {
		h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
	};
	mix(static_cast<std::uint64_t>(m_clock.current_time * 1000.0));
	mix(m_ships.size());
	mix(m_locations.size());
	for (const auto &loc : m_locations) {
		mix(loc.id);
		for (const auto &kv : loc.market.Listings()) {
			mix(kv.first);
			mix(static_cast<std::uint64_t>(kv.second.supply * 100));
			mix(static_cast<std::uint64_t>(kv.second.demand * 100));
			mix(static_cast<std::uint64_t>(kv.second.available_quantity * 100));
			mix(static_cast<std::uint64_t>(kv.second.BuyPrice() * 100));
		}
	}
	for (const auto &ship : m_ships) {
		mix(ship.id);
		mix(ship.location);
		mix(static_cast<std::uint64_t>(ship.credits));
		mix(static_cast<std::uint64_t>(ship.cargo.Used() * 100));
		mix(static_cast<std::uint64_t>(ship.status));
	}
	for (const auto &c : m_contracts) {
		mix(c.id);
		mix(static_cast<std::uint64_t>(c.status));
		mix(static_cast<std::uint64_t>(c.reward));
	}
	for (const auto &kv : m_reputation.All()) {
		mix(kv.first);
		mix(static_cast<std::uint64_t>(kv.second * 100));
	}
	return h;
}

std::string WorldSimulation::FormatDebugPanel() const
{
	std::ostringstream out;
	std::string systemName = "Unknown";
	if (!m_locations.empty()) {
		systemName = m_locations.front().pioneer_system.empty() ? "New Albion" : m_locations.front().pioneer_system;
	}
	out << "SYSTEM: " << systemName << "\n";
	out << "SIM TIME: " << static_cast<int>(m_clock.current_time) << "s\n";

	for (const auto &loc : m_locations) {
		out << "\nMARKET — " << loc.name << "\n";
		out << "--------------------------------\n";
		out << "Commodity       Supply    Price\n";
		for (const auto &kv : loc.market.Listings()) {
			const auto *c = m_commodities.FindById(kv.first);
			const std::string name = c ? c->name : "?";
			out.width(14);
			out << std::left << name;
			out.width(8);
			out << std::right << static_cast<int>(kv.second.supply);
			out.width(10);
			out << static_cast<int>(std::lround(kv.second.BuyPrice())) << "\n";
		}
	}

	out << "\nNPC TRADERS\n";
	out << "--------------------------------\n";
	for (const auto &ship : m_ships) {
		if (!ship.is_npc_trader) {
			continue;
		}
		out << ship.name << "\n";
		if (ship.active_trade) {
			const auto *c = m_commodities.FindById(ship.active_trade->commodity);
			out << "  " << (c ? c->name : "?") << ": " << static_cast<int>(ship.cargo.Quantity(ship.active_trade->commodity)) << "\n";
			if (ship.destination) {
				if (const auto *d = GetLocation(*ship.destination)) {
					out << "  Destination: " << d->name << "\n";
				}
			}
		}
		out << "  Status: " << ToString(ship.status) << "\n";
		out << "  Credits: " << ship.credits << "\n";
	}

	out << "\nCONTRACTS\n";
	out << "--------------------------------\n";
	for (const auto &c : m_contracts) {
		const auto *com = m_commodities.FindById(c.commodity);
		const auto *o = GetLocation(c.origin);
		const auto *d = GetLocation(c.destination);
		out << c.title << " [" << ToString(c.status) << "]\n";
		out << "  " << (com ? com->name : "?") << " x" << static_cast<int>(c.quantity) << "\n";
		out << "  Reward: " << c.reward << "\n";
		out << "  Origin: " << (o ? o->name : "?") << "\n";
		out << "  Destination: " << (d ? d->name : "?") << "\n";
	}

	out << "\nFACTION REPUTATION\n";
	out << "--------------------------------\n";
	for (const auto &f : m_factions.All()) {
		out << f.name << ": " << static_cast<int>(m_reputation.Get(f.id)) << "\n";
	}

	for (const auto &ship : m_ships) {
		if (ship.is_player) {
			out << "\nPLAYER CREDITS: " << ship.credits << "\n";
			out << "PLAYER CARGO USED: " << ship.cargo.Used() << "/" << ship.cargo.Capacity() << "\n";
			if (const auto *loc = GetLocation(ship.location)) {
				out << "PLAYER LOCATION: " << loc->name << "\n";
			}
		}
	}

	return out.str();
}

} // namespace wayfarer::sim

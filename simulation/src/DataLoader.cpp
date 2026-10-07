// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/DataLoader.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace wayfarer::sim {

DataLoader::DataLoader(std::string data_root) : m_root(std::move(data_root)) {}

static json ReadJsonFile(const fs::path &path, std::string &error)
{
	std::ifstream in(path);
	if (!in) {
		error = "Cannot open " + path.string();
		return {};
	}
	try {
		json j;
		in >> j;
		return j;
	} catch (const std::exception &e) {
		error = std::string("JSON error in ") + path.string() + ": " + e.what();
		return {};
	}
}

ShipHull DataLoader::LoadShipHullFile(const std::string &path, std::string &error)
{
	ShipHull hull;
	json j = ReadJsonFile(path, error);
	if (!error.empty()) {
		return hull;
	}
	hull.key = j.value("key", "");
	hull.name = j.value("name", hull.key);
	hull.role = j.value("role", "trader");
	hull.mass = j.value("mass", 100.0);
	hull.cargo_capacity = j.value("cargo_capacity", 20.0);
	hull.fuel_capacity = j.value("fuel_capacity", 20.0);
	hull.engine_power = j.value("engine_power", 1.0);
	hull.shield_strength = j.value("shield_strength", 100.0);
	hull.weapon_power = j.value("weapon_power", 0.0);
	hull.hull_strength = j.value("hull_strength", 200.0);
	hull.crew_capacity = j.value("crew_capacity", 2.0);
	hull.outfit_capacity = j.value("outfit_capacity", 20.0);
	hull.pioneer_model = j.value("pioneer_model", "");
	return hull;
}

Outfit DataLoader::LoadOutfitFile(const std::string &path, std::string &error)
{
	Outfit o;
	json j = ReadJsonFile(path, error);
	if (!error.empty()) {
		return o;
	}
	o.key = j.value("key", "");
	o.name = j.value("name", o.key);
	o.category = j.value("category", "utility");
	o.mass = j.value("mass", 0.0);
	o.outfit_space = j.value("outfit_space", 1.0);
	o.engine_power = j.value("engine_power", 0.0);
	o.shield_strength = j.value("shield_strength", 0.0);
	o.weapon_power = j.value("weapon_power", 0.0);
	o.cargo_bonus = j.value("cargo_bonus", 0.0);
	return o;
}

bool DataLoader::LoadCommodities(WorldSimulation &sim)
{
	const fs::path path = fs::path(m_root) / "commodities" / "commodities.json";
	json j = ReadJsonFile(path, m_error);
	if (!m_error.empty()) {
		return false;
	}
	if (!j.contains("commodities") || !j["commodities"].is_array()) {
		m_error = "commodities.json missing commodities array";
		return false;
	}
	for (const auto &item : j["commodities"]) {
		Commodity c;
		c.key = item.value("key", "");
		c.name = item.value("name", c.key);
		const std::string role = item.value("role", "other");
		if (role == "food") c.role = CommodityRole::Food;
		else if (role == "raw") c.role = CommodityRole::RawMaterial;
		else if (role == "refined") c.role = CommodityRole::RefinedMaterial;
		else if (role == "fuel") c.role = CommodityRole::Fuel;
		else if (role == "industrial") c.role = CommodityRole::Industrial;
		else if (role == "high_tech") c.role = CommodityRole::HighTech;
		else if (role == "luxury") c.role = CommodityRole::Luxury;
		else c.role = CommodityRole::Other;
		c.base_price = item.value("base_price", 100.0);
		c.mass_per_unit = item.value("mass_per_unit", 1.0);
		sim.Commodities().Add(c);
	}
	return true;
}

bool DataLoader::LoadFactions(WorldSimulation &sim)
{
	const fs::path path = fs::path(m_root) / "factions" / "factions.json";
	json j = ReadJsonFile(path, m_error);
	if (!m_error.empty()) {
		return false;
	}
	if (!j.contains("factions") || !j["factions"].is_array()) {
		m_error = "factions.json missing factions array";
		return false;
	}
	for (const auto &item : j["factions"]) {
		Faction f;
		f.key = item.value("key", "");
		f.name = item.value("name", f.key);
		f.government = item.value("government", "");
		f.economic_strength = item.value("economic_strength", 1.0);
		f.military_strength = item.value("military_strength", 1.0);
		if (item.contains("colour") && item["colour"].is_array() && item["colour"].size() >= 3) {
			f.colour.r = item["colour"][0].get<float>();
			f.colour.g = item["colour"][1].get<float>();
			f.colour.b = item["colour"][2].get<float>();
		}
		if (item.contains("territory")) {
			for (const auto &t : item["territory"]) {
				f.territory.push_back(t.get<std::string>());
			}
		}
		const auto id = sim.Factions().Add(f);
		if (item.contains("starting_reputation")) {
			sim.Reputation().Set(id, item["starting_reputation"].get<double>());
		}
	}
	return true;
}

bool DataLoader::LoadAll(WorldSimulation &sim)
{
	if (!LoadCommodities(sim)) {
		return false;
	}
	if (!LoadFactions(sim)) {
		return false;
	}
	return true;
}

} // namespace wayfarer::sim

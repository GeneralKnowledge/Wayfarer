// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace wayfarer::sim {

struct FactionColour {
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
};

struct Faction {
	FactionId id = InvalidFactionId;
	std::string key;
	std::string name;
	FactionColour colour;
	std::string government; // key into governments data
	double economic_strength = 1.0;
	double military_strength = 1.0;
	std::vector<std::string> territory; // location keys
};

/// Player (or actor) standing with factions. Stage 0: no full diplomacy.
class ReputationBoard {
public:
	void Set(FactionId id, double value);
	double Get(FactionId id) const;
	void Adjust(FactionId id, double delta);
	const std::unordered_map<FactionId, double> &All() const { return m_rep; }
	void Clear() { m_rep.clear(); }

private:
	std::unordered_map<FactionId, double> m_rep;
};

class FactionRegistry {
public:
	FactionId Add(Faction f);
	const Faction *FindByKey(const std::string &key) const;
	const Faction *FindById(FactionId id) const;
	const std::vector<Faction> &All() const { return m_items; }
	void Clear();

private:
	std::vector<Faction> m_items;
	FactionId m_next = 1;
};

} // namespace wayfarer::sim

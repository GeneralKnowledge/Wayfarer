// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/Faction.hpp"

namespace wayfarer::sim {

void ReputationBoard::Set(FactionId id, double value)
{
	m_rep[id] = value;
}

double ReputationBoard::Get(FactionId id) const
{
	auto it = m_rep.find(id);
	return it == m_rep.end() ? 0.0 : it->second;
}

void ReputationBoard::Adjust(FactionId id, double delta)
{
	m_rep[id] = Get(id) + delta;
}

FactionId FactionRegistry::Add(Faction f)
{
	f.id = m_next++;
	m_items.push_back(std::move(f));
	return m_items.back().id;
}

const Faction *FactionRegistry::FindByKey(const std::string &key) const
{
	for (const auto &f : m_items) {
		if (f.key == key) {
			return &f;
		}
	}
	return nullptr;
}

const Faction *FactionRegistry::FindById(FactionId id) const
{
	for (const auto &f : m_items) {
		if (f.id == id) {
			return &f;
		}
	}
	return nullptr;
}

void FactionRegistry::Clear()
{
	m_items.clear();
	m_next = 1;
}

} // namespace wayfarer::sim

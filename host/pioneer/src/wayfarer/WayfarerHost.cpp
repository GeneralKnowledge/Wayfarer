// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "WayfarerHost.h"

namespace wayfarer {

WayfarerHost &WayfarerHost::Get()
{
	static WayfarerHost host;
	return host;
}

void WayfarerHost::StartStage0()
{
	m_sim = std::make_unique<sim::WorldSimulation>(sim::SimulationConfig{ 2026, 77, 1.0 });
	m_sim->BootstrapStage0Demo();
	m_adapter = sim::PioneerWorldAdapter{};
	for (const auto &loc : m_sim->GetLocations()) {
		m_adapter.BindLocation(loc.id, loc.pioneer_system, loc.pioneer_body_id);
	}
	for (const auto &ship : m_sim->GetShips()) {
		m_adapter.BindShip(ship.id, ship.is_player ? "player" : ship.name);
	}
	m_accum = 0.0;
	m_active = true;
}

void WayfarerHost::Shutdown()
{
	m_active = false;
	m_sim.reset();
	m_adapter = sim::PioneerWorldAdapter{};
	m_accum = 0.0;
}

void WayfarerHost::OnGameTimeStep(float step)
{
	if (!m_active || !m_sim) {
		return;
	}
	// Map Pioneer seconds into discrete sim ticks (1 sim second each).
	m_accum += static_cast<double>(step);
	while (m_accum >= 1.0) {
		m_sim->Tick();
		m_accum -= 1.0;
	}
}

std::string WayfarerHost::DebugPanelText() const
{
	if (!m_active || !m_sim) {
		return "Wayfarer simulation inactive";
	}
	return m_sim->FormatDebugPanel();
}

bool WayfarerHost::AcceptCurrentContract()
{
	if (!m_active || !m_sim) {
		return false;
	}
	auto *player = m_sim->GetPlayerShip();
	if (!player || m_sim->GetContracts().empty()) {
		return false;
	}
	auto &c = m_sim->GetContracts().front();
	player->location = c.origin;
	return m_sim->AcceptContract(c.id, player->id);
}

bool WayfarerHost::DeliverCurrentContract()
{
	if (!m_active || !m_sim) {
		return false;
	}
	auto *player = m_sim->GetPlayerShip();
	if (!player || m_sim->GetContracts().empty()) {
		return false;
	}
	auto &c = m_sim->GetContracts().front();
	player->location = c.destination;
	return m_sim->DeliverContract(c.id, player->id);
}

bool WayfarerHost::PlayerTravelTo(const std::string &location_key)
{
	if (!m_active || !m_sim) {
		return false;
	}
	auto *player = m_sim->GetPlayerShip();
	auto *loc = m_sim->FindLocationByKey(location_key);
	if (!player || !loc) {
		return false;
	}
	player->location = loc->id;
	return true;
}

int WayfarerHost::NpcCount() const
{
	if (!m_sim) {
		return 0;
	}
	int n = 0;
	for (const auto &s : m_sim->GetShips()) {
		if (s.is_npc_trader) {
			++n;
		}
	}
	return n;
}

static const sim::SimShip *NpcAt(const sim::WorldSimulation &sim, int index)
{
	int n = 0;
	for (const auto &s : sim.GetShips()) {
		if (s.is_npc_trader) {
			if (n == index) {
				return &s;
			}
			++n;
		}
	}
	return nullptr;
}

std::string WayfarerHost::NpcName(int index) const
{
	if (!m_sim) {
		return {};
	}
	const auto *s = NpcAt(*m_sim, index);
	return s ? s->name : std::string{};
}

std::string WayfarerHost::NpcStatus(int index) const
{
	if (!m_sim) {
		return {};
	}
	const auto *s = NpcAt(*m_sim, index);
	return s ? sim::ToString(s->status) : std::string{};
}

std::string WayfarerHost::NpcDestinationName(int index) const
{
	if (!m_sim) {
		return {};
	}
	const auto *s = NpcAt(*m_sim, index);
	if (!s || !s->destination) {
		return {};
	}
	const auto *loc = m_sim->GetLocation(*s->destination);
	return loc ? loc->name : std::string{};
}

double WayfarerHost::NpcProgress(int index) const
{
	if (!m_sim) {
		return 0.0;
	}
	auto visuals = m_adapter.CollectNpcVisuals(*m_sim);
	if (index < 0 || index >= static_cast<int>(visuals.size())) {
		return 0.0;
	}
	return visuals[static_cast<size_t>(index)].progress;
}

std::string WayfarerHost::NpcCargoSummary(int index) const
{
	if (!m_sim) {
		return {};
	}
	const auto *s = NpcAt(*m_sim, index);
	if (!s || !s->active_trade) {
		return {};
	}
	const auto *c = m_sim->Commodities().FindById(s->active_trade->commodity);
	if (!c) {
		return {};
	}
	return c->name + ": " + std::to_string(static_cast<int>(s->cargo.Quantity(c->id)));
}

} // namespace wayfarer

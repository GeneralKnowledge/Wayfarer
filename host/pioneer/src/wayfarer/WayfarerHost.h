// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "wayfarer/sim/PioneerWorldAdapter.hpp"
#include "wayfarer/sim/WorldSimulation.hpp"

#include <memory>
#include <string>

namespace wayfarer {

/// Pioneer-side host for the headless WorldSimulation.
/// Render/physics remain Pioneer's; this owns living economy/NPC/contract state.
class WayfarerHost {
public:
	static WayfarerHost &Get();

	void StartStage0();
	void Shutdown();
	bool IsActive() const { return m_active; }

	/// Advance simulation from Pioneer game time (seconds).
	void OnGameTimeStep(float step);

	wayfarer::sim::WorldSimulation &Sim() { return *m_sim; }
	const wayfarer::sim::WorldSimulation &Sim() const { return *m_sim; }
	wayfarer::sim::PioneerWorldAdapter &Adapter() { return m_adapter; }

	std::string DebugPanelText() const;

	bool AcceptCurrentContract();
	bool DeliverCurrentContract();
	bool PlayerTravelTo(const std::string &location_key);

	/// NPC visual helpers for Lua.
	int NpcCount() const;
	std::string NpcName(int index) const;
	std::string NpcStatus(int index) const;
	std::string NpcDestinationName(int index) const;
	double NpcProgress(int index) const;
	std::string NpcCargoSummary(int index) const;

private:
	WayfarerHost() = default;

	bool m_active = false;
	double m_accum = 0.0;
	std::unique_ptr<wayfarer::sim::WorldSimulation> m_sim;
	wayfarer::sim::PioneerWorldAdapter m_adapter;
};

} // namespace wayfarer

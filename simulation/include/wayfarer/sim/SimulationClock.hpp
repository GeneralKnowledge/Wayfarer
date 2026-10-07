// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace wayfarer::sim {

/// Explicit simulation clock, independent of rendering.
struct SimulationClock {
	double current_time = 0.0;   // simulation seconds
	double tick_duration = 1.0;  // seconds advanced per Tick()
	bool paused = false;
	double time_scale = 1.0;     // architectural hook for N× speed

	void Advance()
	{
		if (!paused) {
			current_time += tick_duration * time_scale;
		}
	}

	void Reset(double t = 0.0)
	{
		current_time = t;
	}
};

} // namespace wayfarer::sim

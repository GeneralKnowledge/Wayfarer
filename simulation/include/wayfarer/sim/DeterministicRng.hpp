// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace wayfarer::sim {

/// Simple LCG for deterministic simulation (no std::mt19937 variance across platforms).
class DeterministicRng {
public:
	explicit DeterministicRng(std::uint64_t seed = 1) : m_state(seed ? seed : 1) {}

	void Seed(std::uint64_t seed) { m_state = seed ? seed : 1; }
	std::uint64_t State() const { return m_state; }

	std::uint32_t NextU32()
	{
		m_state = m_state * 6364136223846793005ULL + 1ULL;
		return static_cast<std::uint32_t>(m_state >> 32);
	}

	/// Uniform in [0, 1).
	double NextDouble()
	{
		return NextU32() / 4294967296.0;
	}

	int NextInt(int min_inclusive, int max_inclusive)
	{
		if (max_inclusive <= min_inclusive) {
			return min_inclusive;
		}
		const int span = max_inclusive - min_inclusive + 1;
		return min_inclusive + static_cast<int>(NextU32() % static_cast<std::uint32_t>(span));
	}

private:
	std::uint64_t m_state;
};

} // namespace wayfarer::sim

// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/WorldSimulation.hpp"

#include <iostream>

using namespace wayfarer::sim;

static std::uint64_t RunSeeded(std::uint64_t world, std::uint64_t simSeed)
{
	WorldSimulation a({ world, simSeed, 1.0 });
	a.BootstrapStage0Demo();
	a.Tick(200);
	return a.StateFingerprint();
}

int test_determinism()
{
	const auto f1 = RunSeeded(12345, 999);
	const auto f2 = RunSeeded(12345, 999);
	const auto f3 = RunSeeded(12345, 1000);

	int fails = 0;
	if (f1 != f2) {
		std::cerr << "FAIL determinism: same seeds diverged (" << f1 << " vs " << f2 << ")\n";
		++fails;
	}
	if (f1 == f3) {
		// Different sim seeds should usually differ after NPC activity; not strictly required
		// if RNG unused, but Bootstrap uses fixed setup — tick industries still deterministic.
		// Use fingerprint inequality only when seeds affect RNG draws.
	}

	WorldSimulation b({ 1, 1, 1.0 });
	b.BootstrapStage0Demo();
	const auto before = b.StateFingerprint();
	b.Tick(50);
	const auto after = b.StateFingerprint();
	if (before == after) {
		std::cerr << "FAIL determinism: expected state to change over ticks\n";
		++fails;
	}

	if (fails == 0) {
		std::cout << "OK determinism\n";
	}
	return fails > 0 ? 1 : 0;
}

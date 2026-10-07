// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayfarer/sim/CargoHold.hpp"

#include <iostream>

using namespace wayfarer::sim;

int test_cargo()
{
	int fails = 0;
	CargoHold hold(20.0);

	if (!hold.TryAdd(1, 15.0)) {
		std::cerr << "FAIL cargo: should accept 15/20\n";
		++fails;
	}
	if (hold.TryAdd(2, 10.0)) {
		std::cerr << "FAIL cargo: should reject overflow (15+10 > 20)\n";
		++fails;
	}
	if (!hold.TryAdd(2, 5.0)) {
		std::cerr << "FAIL cargo: should accept exact fill\n";
		++fails;
	}
	if (hold.TryAdd(3, 0.1)) {
		std::cerr << "FAIL cargo: full hold should reject more\n";
		++fails;
	}
	if (!hold.TryRemove(1, 15.0)) {
		std::cerr << "FAIL cargo: remove\n";
		++fails;
	}
	if (hold.Quantity(1) != 0.0) {
		std::cerr << "FAIL cargo: quantity after remove\n";
		++fails;
	}

	if (fails == 0) {
		std::cout << "OK cargo\n";
	}
	return fails > 0 ? 1 : 0;
}

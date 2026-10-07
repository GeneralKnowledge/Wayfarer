// Copyright © 2026 Wayfarer Contributors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdlib>
#include <iostream>

int test_market();
int test_cargo();
int test_trade();
int test_contract();
int test_determinism();

int main()
{
	int failures = 0;
	failures += test_market();
	failures += test_cargo();
	failures += test_trade();
	failures += test_contract();
	failures += test_determinism();

	if (failures == 0) {
		std::cout << "All Wayfarer simulation tests passed.\n";
		return EXIT_SUCCESS;
	}
	std::cerr << failures << " test suite(s) failed.\n";
	return EXIT_FAILURE;
}

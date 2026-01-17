#include <iostream>

#include "Random.h"

int main() {
	auto rand {Random::get<double>()};
	std::cout << rand << std::endl;

	return 0;
}

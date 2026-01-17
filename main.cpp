#include <iostream>

#include "Random.h"
#include "Simulation.h"

int main() {
	Simulation<double> sim {};

	sim.runMany(1'000'000);

	std::cout << sim.getAverageRatio() << std::endl;
	return 0;
}

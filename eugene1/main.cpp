#include <iostream>

#include "common/Timer.h"

#include "Simulation.h"

int main() {
	Timer timer {};
	Simulation<double> sim {};

	sim.runMany(10'000'000);

	std::cout << "Average ratio: " << sim.getAverageRatio() << std::endl;
	return 0;
}

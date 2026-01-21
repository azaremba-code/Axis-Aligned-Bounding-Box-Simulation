#include <iostream>

#include "Simulation.h"
#include "common/Timer.h"

int main() {
	Timer timer {};
	Simulation<double> sim {50'000'000};

	sim.run();

	std::cout << "Average ratio: " << sim.getAverageRatio() << std::endl;
	return 0;
}

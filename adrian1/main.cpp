#include <iostream>

#include "Simulation.h"
#include "Timer.h"

int main() {
	Timer timer {};
	Simulation<double> sim {};

	sim.runMany(50'000'000);

	std::cout << "Average ratio: " << sim.getAverageRatio() << std::endl;
	return 0;
}

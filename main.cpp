#include <iostream>

#include "Random.h"
#include "Simulation.h"
#include "Timer.h"

int main() {
	Timer timer {};
	Simulation<double> sim {};

	sim.runMany(10'000'000);

	std::cout << "Average ratio: " << sim.getAverageRatio() << std::endl;
	return 0;
}

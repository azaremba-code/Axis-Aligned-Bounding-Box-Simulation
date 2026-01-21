#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <sched.h>

#include <argparse/argparse.hpp>

#include "Simulation.h"
#include "common/Timer.h"
#include "common/Concurrency.h"

int main(int argc, char* argv[]) {
	// handle command line argument options
	int nsims = 1'000'000'000;
	int mxthreads = 30;
	int ngon = 3;

	argparse::ArgumentParser program("eugene2");
	program.add_argument("-n", "--nsims").help("number of simulations").default_value(nsims).scan<'i', int>();
	program.add_argument("-t", "--mxthreads").help("maximum number of threads").default_value(mxthreads).scan<'i', int>();
	program.add_argument("-g", "--ngon").help("number of points of the polygon").default_value(ngon).scan<'i', int>();

	try {
		program.parse_args(argc, argv);
	} catch (const std::runtime_error& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		return 1;
	}

	nsims = program.get<int>("--nsims");
	mxthreads = program.get<int>("--mxthreads");
	ngon = program.get<int>("--ngon");

    // determine the number of cores on the machine
    // int numCores = std::thread::hardware_concurrency();
    // std::cout << "Number of cores available: " << numCores << std::endl;

	int numSockets = Concurrency::get_num_physical_cpus();
	int numOfPhysicalCores = Concurrency::get_num_physical_cores();
	int numAvailableCores = Concurrency::get_num_available_cores();
	int coresToUse = std::min(numAvailableCores, numOfPhysicalCores);
	std::cout << "CPU sockets: " << numSockets << ", physical cores: " << numOfPhysicalCores << ", available cores: " << numAvailableCores << ", cores to use: " << coresToUse << ", hyperthreading enabled: " << Concurrency::is_hyperthreading_enabled() << std::endl;

	Concurrency::print_physical_core_mapping();

	// we will 0 to the OS
	// core 1 for the main thread (probably same physical core as the OS
	// the rest are for the threads, which will be pinned to even-numbered cores, up to the number of available physical cores

	if (numAvailableCores >= 2) {
		if (Concurrency::pin_to_core(1)) {
			std::cout << "Main thread pinned to core 1 " << std::endl;
		} else {
			std::cerr << "Failed to pin main thread to core 1" << std::endl;
		}
	}

    // determine the number of threads to use
    int numThreads = std::max(1, std::min(mxthreads, coresToUse - 1));  // leave one core for the OS
	int numRunsPerThread = nsims / numThreads;
	int runsAdjustment = nsims - numRunsPerThread * numThreads;
    std::cout << "Will use " << numThreads << " threads to run " << nsims << " simulations with " << numRunsPerThread << " runs per thread and " << std::endl;
    // std::cout << "Runs adjustment: " << runsAdjustment << std::endl;

	Timer timer {};

	// create the threads
    std::vector<std::thread> threads;
	std::vector<Simulation<double>::floatType> ratiosSums(numThreads, 0);
	std::vector<int> runCounts(numThreads, 0);
	
    for (int i = 0; i < numThreads; i++) {
		int numRuns = numRunsPerThread + ((i==0)? runsAdjustment : 0);
        threads.emplace_back([i, numRuns, ngon, &ratiosSums, &runCounts]() {
			int coreId = 2*(i + 1);
			if (!Concurrency::pin_to_core(coreId)) {
				std::cerr << "Failed to pin thread " << i << " to core " << coreId << std::endl;
			}
			// Create Simulation object AFTER pinning, so it detects the correct core
			Simulation<double> sim(numRuns, ngon);
            sim.run();
			// Store results
			ratiosSums[i] = sim.getRatiosSum();
			runCounts[i] = sim.getRunCount();
        });
    }

	// join the threads
	for (auto& thread : threads) {
		thread.join();
	}

	// calculate the average ratio
	Simulation<double>::floatType ratiosSum = 0;
	int runCount = 0;
	for (int i = 0; i < numThreads; i++) {
		ratiosSum += ratiosSums[i];
		runCount += runCounts[i];
	}
	std::cout << "Average ratio: " << ratiosSum / runCount << std::endl;

	return 0;
}

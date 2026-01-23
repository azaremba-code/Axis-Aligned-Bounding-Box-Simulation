#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <pthread.h>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <argparse/argparse.hpp>

#include "common/Concurrency.h"
#include "common/Timer.h"
#include "simulation/ISimulation.h"

#include "SimulationAdrian1.h"
#include "SimulationEugene1.h"

#define VERBOSE_OUTPUT(msg) if (verbose) { std::cout << msg << std::endl; }

#define INFO_OUTPUT(msg) { std::cout << msg << std::endl; }

#define ERROR_OUTPUT(msg) { std::cerr << msg << std::endl; }

int main(int argc, char* argv[]) {
	// handle command line argument options
	int nsims = 1'000'000'000;
	int mxthreads = 30;
	int ngon = 3;
	std::string simulationName = "adrian1";
	bool verbose = false;

	argparse::ArgumentParser program("eugene2");
	program.add_argument("-n", "--nsims").help("number of simulations").default_value(nsims).scan<'i', int>();
	program.add_argument("-t", "--mxthreads").help("maximum number of threads").default_value(mxthreads).scan<'i', int>();
	program.add_argument("-g", "--ngon").help("number of points of the polygon").default_value(ngon).scan<'i', int>();
	program.add_argument("-s", "--simulation").help("simulation name, e.g. adrian1 or eugene1").default_value(simulationName);
	program.add_argument("-v", "--verbose").help("verbose output").default_value(verbose).implicit_value(true);

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
	simulationName = program.get<std::string>("--simulation");
	verbose = program.get<bool>("--verbose");

    if (simulationName != "adrian1" && simulationName != "eugene1") {
        ERROR_OUTPUT("Invalid simulation name: " << simulationName);
        return 1;
    }

	int numSockets = Concurrency::get_num_physical_cpus();
	int numOfPhysicalCores = Concurrency::get_num_physical_cores();
	int numAvailableCores = Concurrency::get_num_available_cores();
	int coresToUse = std::min(numAvailableCores, numOfPhysicalCores);
	VERBOSE_OUTPUT("CPU sockets: " << numSockets << ", physical cores: " << numOfPhysicalCores << ", available cores: " << numAvailableCores << ", cores to use: " << coresToUse << ", hyperthreading enabled: " << Concurrency::is_hyperthreading_enabled());

	if (verbose) {
		Concurrency::print_physical_core_mapping();
	}

	// get the physical core id mapping
	auto physicalCoreIdMapping = Concurrency::get_physical_core_mapping();
	std::map<int, int> physicalToLogicalCoreMapping {};
	int physicalCoreId = 0;
	for (const auto& [socketId, coreId] : physicalCoreIdMapping) {
		physicalToLogicalCoreMapping[physicalCoreId] = coreId.front();
		physicalCoreId++;
	}

	// we will give core 0 to the OS
	// core 1 for the main thread (probably same physical core as the OS)
	// the rest are for the threads, which will be pinned to even-numbered cores, up to the number of available physical cores

	if (numAvailableCores >= 2) {
		if (Concurrency::pin_to_core(1)) {
			VERBOSE_OUTPUT("Main thread pinned to core 1 ");
		} else {
			VERBOSE_OUTPUT("Failed to pin main thread to core 1");
		}
	}

    // determine the number of threads to use
    int numThreads = std::max(1, std::min(mxthreads, coresToUse - 1));  // leave one core for the OS
	if (numThreads != mxthreads) {
		INFO_OUTPUT("WARN: Number of threads adjusted from " << mxthreads << " to " << numThreads << " for optimal performance");
	}
	int numRunsPerThread = nsims / numThreads;
	int runsAdjustment = nsims - numRunsPerThread * numThreads;
    VERBOSE_OUTPUT("Will use " << numThreads << " threads to run " << nsims << " simulations with " << numRunsPerThread << " runs per thread and " << runsAdjustment << " runs adjustment");
    // std::cout << "Runs adjustment: " << runsAdjustment << std::endl;

    INFO_OUTPUT("Using simulation: " << simulationName);

	Timer timer {};

	// create the threads
    std::vector<std::thread> threads;
	// verctor of pairs of sums and run counts
	std::vector<std::pair<double, int>> results(numThreads);
	
    for (int i = 0; i < numThreads; i++) {
		int numRuns = numRunsPerThread + ((i==0)? runsAdjustment : 0);
        threads.emplace_back([i, ngon, numRuns, &results, simulationName, &physicalToLogicalCoreMapping]() {
			auto coreId = physicalToLogicalCoreMapping[i+1]; // shifting by 1 to give the main thread core 0
			if (!Concurrency::pin_to_core(coreId)) {
				ERROR_OUTPUT("Failed to pin thread " << i << " to core " << coreId);
			}
			// the sim pointer below should be a unique pointer
			std::unique_ptr<simulation::ISimulation<double>> sim {nullptr};
			if (simulationName == "adrian1") {
				sim = std::make_unique<SimulationAdrian1<double>>(numRuns, ngon);
			} else if (simulationName == "eugene1") {
				sim = std::make_unique<SimulationEugene1<double>>(numRuns, ngon);
			} else {
				ERROR_OUTPUT("Invalid simulation name: " << simulationName);
				exit(-1);
			}
			sim->run();
			// Store results
			results[i] = std::make_pair(sim->getSumOfRatios(), sim->getRunCount());
        });
    }

	// join the threads
	for (auto& thread : threads) {
		thread.join();
	}

	timer.stop();

	double totalRatiosSum = 0;
	int totalRunCount = 0;
	for (auto& result : results) {
		totalRatiosSum += result.first;
		totalRunCount += result.second;
	}
	INFO_OUTPUT("Average ratio: " << totalRatiosSum / totalRunCount);

	timer.printTime("total");

	return 0;
}

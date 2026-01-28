#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <pthread.h>
#include <sched.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <argparse/argparse.hpp>

#include "common/Concurrency.h"
#include "common/Timer.h"
#include "simulation/ISimulation.h"

#include "SimulationAdrian1.h"
#include "SimulationEugene1.h"
#include "SimulationEugene2.h"
#include "SimulationEugene3.h"
#include "SimulationEugene4.h"
#include "SimulationEugene5.h"

// TODO: Change this to a constexpr function instead of macro for better safety
#define VERBOSE_OUTPUT(msg) if (verbose) { std::cout << msg << std::endl; }

#define INFO_OUTPUT(msg) { std::cout << msg << std::endl; }

#define ERROR_OUTPUT(msg) { std::cerr << msg << std::endl; }

int main1(int argc, char* argv[]) {
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

	constexpr std::array<const char*, 6> validSimulations = {"adrian1", "eugene1", "eugene2", "eugene3", "eugene4", "eugene5"};
    if (std::find(validSimulations.begin(), validSimulations.end(), simulationName) == validSimulations.end()) {
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
			} else if (simulationName == "eugene2") {
				sim = std::make_unique<SimulationEugene2<double>>(numRuns, ngon);
			} else if (simulationName == "eugene3") {
				sim = std::make_unique<SimulationEugene3<double>>(numRuns, ngon);
			} else if (simulationName == "eugene4") {
				sim = std::make_unique<SimulationEugene4<double>>(numRuns, ngon);
			} else if (simulationName == "eugene5") {
				sim = std::make_unique<SimulationEugene5<double>>(numRuns, ngon);
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

int main2(int argc, char* argv[]) {
	constexpr int defaultNSims {1'000'000'000};
	constexpr int defaultMaxThreads {30};
	constexpr int defaultNgon {3};
	constexpr std::string_view defaultSimulationName {"adrian1"};
	constexpr bool defaultVerbose {false};

    argparse::ArgumentParser program("eugene2");
    program.add_argument("-n", "--nsims").help("number of simulations").default_value(defaultNSims).scan<'i', int>();
    program.add_argument("-t", "--mxthreads").help("maximum number of threads").default_value(defaultMaxThreads).scan<'i', int>();
    program.add_argument("-g", "--ngon").help("number of points of the polygon").default_value(defaultNgon).scan<'i', int>();
    program.add_argument("-s", "--simulation").help("simulation name, e.g. adrian1 or eugene1").default_value(defaultSimulationName);
    program.add_argument("-v", "--verbose").help("verbose output").default_value(defaultVerbose).implicit_value(true);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    const int nSims {program.get<int>("--nsims")};
    const int maxThreads {program.get<int>("--mxthreads")};
    const int ngon {program.get<int>("--ngon")};
    const std::string simulationName {program.get<std::string>("--simulation")};
    const bool verbose {program.get<bool>("--verbose")};

	using namespace std::literals::string_literals; // for ""s suffix, to enable CTAD for std::array
	constexpr std::array<const char*, 2> validNames {"adrian1", "eugene1"};
	const bool isValidName = std::find(validNames.begin(), validNames.end(), simulationName) != validNames.end();

    if (!isValidName) {
        std::cerr << "Invalid simulation name: " << simulationName << "\n";
        return 1;
    }

	const int numSockets {Concurrency::get_num_physical_cpus()};
	const int numOfPhysicalCores {Concurrency::get_num_physical_cores()};
	const int numAvailableCores {Concurrency::get_num_available_cores()};
	const int coresToUse {std::min(numAvailableCores, numOfPhysicalCores)};
	VERBOSE_OUTPUT("CPU sockets: " << numSockets << ", physical cores: " << numOfPhysicalCores << ", available cores: " << numAvailableCores << ", cores to use: " << coresToUse << ", hyperthreading enabled: " << Concurrency::is_hyperthreading_enabled());

	if (verbose) {
        Concurrency::print_physical_core_mapping();
    }

    // get the physical core id mapping
    const auto& physicalCoreIdMapping {Concurrency::get_physical_core_mapping()};
    std::map<int, int> physicalToLogicalCoreMapping {}; // TODO: Possibly use unordered_map here? Unless we need ordering
    int physicalCoreId {0};
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
    const int numThreads {std::max(1, std::min(maxThreads, coresToUse - 1))};  // leave one core for the OS
    if (numThreads != maxThreads) {
        INFO_OUTPUT("WARN: Number of threads adjusted from " << maxThreads << " to " << numThreads << " for optimal performance");
    }
    const int numRunsPerThread {nSims / numThreads};
    const int runsAdjustment {nSims - numRunsPerThread * numThreads};
    VERBOSE_OUTPUT("Will use " << numThreads << " threads to run " << nSims << " simulations with " << numRunsPerThread << " runs per thread and " << runsAdjustment << " runs adjustment");
    // std::cout << "Runs adjustment: " << runsAdjustment << std::endl;

    INFO_OUTPUT("Using simulation: " << simulationName);

    Timer timer {};

	std::vector<std::unique_ptr<simulation::ISimulation<double>>> sims {};
	sims.reserve(numThreads);

	for (int i {0}; i < numThreads; i++) {
		const int numRuns = numRunsPerThread + ((i==0)? runsAdjustment : 0);
		if (simulationName == "adrian1") {
			sims.push_back(std::make_unique<SimulationAdrian1<double>>(numRuns, ngon));
		} else if (simulationName == "eugene1") {
			sims.push_back(std::make_unique<SimulationEugene1<double>>(numRuns, ngon));
		} else { // TODO: Verify that this else is unneeded - should never happen (we already checked for this)
			ERROR_OUTPUT("Invalid simulation name: " << simulationName);
			exit(-1);
		}
	}

    // create the threads
    std::vector<std::thread> threads {};
	threads.reserve(numThreads);
	
	for (int i {0}; i < numThreads; i++) {
		threads.emplace_back([i, &sims, &physicalToLogicalCoreMapping] {
			auto coreId = physicalToLogicalCoreMapping[i+1]; // shifting by 1 to give the main thread core 0
            if (!Concurrency::pin_to_core(coreId)) {
                ERROR_OUTPUT("Failed to pin thread " << i << " to core " << coreId);
            }

			sims[i]->run();
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	timer.stop();

	double totalRatiosSum {0.0};
	int totalRunCount {0};
	for (const auto& sim : sims) {
		totalRatiosSum += sim->getSumOfRatios();
		totalRunCount += sim->getRunCount();
	}
	INFO_OUTPUT("Average ratio: " << totalRatiosSum / totalRunCount);

    timer.printTime("total");
	
	return 0;
}

int main(int argc, char* argv[]) {
	return main1(argc, argv);
}

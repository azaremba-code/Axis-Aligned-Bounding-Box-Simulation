#include <iostream>
#include <stdexcept>
#include <string>

#include <argparse/argparse.hpp>

#include "common/Timer.h"
#include "cuda_simulation_eugene1.cuh"

namespace {

constexpr int kDefaultRuns = 1'000'000;
constexpr int kDefaultNgon = 3;
constexpr std::uint64_t kDefaultSeed = 1234ULL;

std::string getPrecisionLabel(const std::string& value) {
	if (value == "float" || value == "double") {
		return value;
	}
	throw std::runtime_error("precision must be 'float' or 'double'");
}

} // namespace

int main(int argc, char* argv[]) {
	int nsims = kDefaultRuns;
	int ngon = kDefaultNgon;
	std::uint64_t seed = kDefaultSeed;
	std::string precision = "double";

	argparse::ArgumentParser program("cuda_eugene1");
	program.add_argument("-n", "--nsims").help("number of simulations").default_value(nsims).scan<'i', int>();
	program.add_argument("-g", "--ngon").help("number of polygon points").default_value(ngon).scan<'i', int>();
	program.add_argument("--seed").help("RNG seed").default_value(seed).scan<'u', std::uint64_t>();
	program.add_argument("-p", "--precision")
		.help("precision: float or double")
		.default_value(precision);

	try {
		program.parse_args(argc, argv);
	} catch (const std::runtime_error& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		return 1;
	}

	nsims = program.get<int>("--nsims");
	ngon = program.get<int>("--ngon");
	seed = program.get<std::uint64_t>("--seed");
	precision = getPrecisionLabel(program.get<std::string>("--precision"));

	cuda_sim::CudaSimConfig config {};
	config.runCount = nsims;
	config.polygonPointCount = ngon;
	config.seed = seed;

	Timer timer {};

	if (precision == "float") {
		const auto result = cuda_sim::runCudaEugene1<float>(config);
		std::cout << "Average ratio: " << result.averageRatio << std::endl;
	} else {
		const auto result = cuda_sim::runCudaEugene1<double>(config);
		std::cout << "Average ratio: " << result.averageRatio << std::endl;
	}

	timer.stop();
	timer.printTime("total");

	return 0;
}

#ifndef CUDA_SIMULATION_EUGENE1_CUH
#define CUDA_SIMULATION_EUGENE1_CUH

#include <cstdint>

namespace cuda_sim {

constexpr int kMaxPolygonPoints = 32;

struct CudaSimConfig {
	int runCount {};
	int polygonPointCount {3};
	std::uint64_t seed {1234ULL};
};

template <typename T>
struct CudaSimResult {
	T sumOfRatios {};
	T averageRatio {};
};

template <typename T>
CudaSimResult<T> runCudaEugene1(const CudaSimConfig& config);

} // namespace cuda_sim

#endif

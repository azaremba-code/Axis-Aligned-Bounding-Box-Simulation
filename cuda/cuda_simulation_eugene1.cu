#include "cuda_simulation_eugene1.cuh"

#include <stdexcept>
#include <string>
#include <type_traits>

#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <thrust/device_vector.h>
#include <thrust/reduce.h>

namespace cuda_sim {

namespace {

template <typename T>
struct Point {
	T x {};
	T y {};
};

inline void checkCuda(cudaError_t result, const char* context) {
	if (result != cudaSuccess) {
		throw std::runtime_error(std::string("CUDA error at ") + context + ": " +
		                         cudaGetErrorString(result));
	}
}

__device__ inline float deviceAbs(float value) {
	return fabsf(value);
}

__device__ inline double deviceAbs(double value) {
	return fabs(value);
}

template <typename T>
__device__ inline T uniformUnit(curandStatePhilox4_32_10_t& state) {
	if constexpr (std::is_same_v<T, double>) {
		return curand_uniform_double(&state);
	}
	return curand_uniform(&state);
}

template <typename T>
__device__ inline T computeRatio(curandStatePhilox4_32_10_t& state, int polygonPointCount) {
	Point<T> points[kMaxPolygonPoints];
	for (int i = 0; i < polygonPointCount; ++i) {
		const T x = static_cast<T>(1.0) + uniformUnit<T>(state);
		const T y = static_cast<T>(1.0) + uniformUnit<T>(state);
		points[i] = Point<T> {x, y};
	}

	T area = static_cast<T>(0);
	for (int i = 0; i < polygonPointCount; ++i) {
		const Point<T>& curr = points[i];
		const Point<T>& next = points[(i + 1) % polygonPointCount];
		area += curr.x * next.y - next.x * curr.y;
	}
	area = deviceAbs(area) / static_cast<T>(2.0);

	T minX = points[0].x;
	T minY = points[0].y;
	T maxX = points[0].x;
	T maxY = points[0].y;
	for (int i = 1; i < polygonPointCount; ++i) {
		const Point<T>& point = points[i];
		minX = point.x < minX ? point.x : minX;
		minY = point.y < minY ? point.y : minY;
		maxX = point.x > maxX ? point.x : maxX;
		maxY = point.y > maxY ? point.y : maxY;
	}

	const T boundingBoxArea = (maxX - minX) * (maxY - minY);
	return area / boundingBoxArea;
}

template <typename T>
__global__ void eugene1Kernel(T* ratios, int runCount, int polygonPointCount, std::uint64_t seed) {
	const int tid = blockIdx.x * blockDim.x + threadIdx.x;
	if (tid >= runCount) {
		return;
	}

	curandStatePhilox4_32_10_t rng {};
	curand_init(seed, static_cast<std::uint64_t>(tid), 0, &rng);
	ratios[tid] = computeRatio<T>(rng, polygonPointCount);
}

template <typename T>
CudaSimResult<T> runCudaEugene1Impl(const CudaSimConfig& config) {
	if (config.runCount <= 0) {
		return CudaSimResult<T> {};
	}
	if (config.polygonPointCount > kMaxPolygonPoints) {
		throw std::runtime_error("polygonPointCount exceeds kMaxPolygonPoints");
	}

	thrust::device_vector<T> ratios(static_cast<std::size_t>(config.runCount));

	const int threadsPerBlock = 256;
	const int blocks = (config.runCount + threadsPerBlock - 1) / threadsPerBlock;

	eugene1Kernel<<<blocks, threadsPerBlock>>>(
		thrust::raw_pointer_cast(ratios.data()),
		config.runCount,
		config.polygonPointCount,
		config.seed);

	checkCuda(cudaGetLastError(), "kernel launch");
	checkCuda(cudaDeviceSynchronize(), "kernel sync");

	const T sumOfRatios = thrust::reduce(ratios.begin(), ratios.end(), static_cast<T>(0));
	const T averageRatio = sumOfRatios / static_cast<T>(config.runCount);

	return CudaSimResult<T> {sumOfRatios, averageRatio};
}

} // namespace

template <typename T>
CudaSimResult<T> runCudaEugene1(const CudaSimConfig& config) {
	return runCudaEugene1Impl<T>(config);
}

template CudaSimResult<float> runCudaEugene1<float>(const CudaSimConfig& config);
template CudaSimResult<double> runCudaEugene1<double>(const CudaSimConfig& config);

} // namespace cuda_sim

#ifndef SIMULATION_EUGENE2_H
#define SIMULATION_EUGENE2_H

#include <cassert>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <concepts>
#include <random>
#include <vector>
#include <utility>

#include "simulation/ISimulation.h"
#include "hwy/highway.h"


template <std::floating_point FloatType>
class SimulationEugene2 : public simulation::ISimulation<FloatType> {
public:

	SimulationEugene2(int runCount, int polygonPointCount = 3) :
		simulation::ISimulation<FloatType>(runCount, polygonPointCount),
		m_ratiosSum {0}
	{}


	FloatType getAverageRatio() const override {
		assert(simulation::ISimulation<FloatType>::getRunCount() > 0 && "Must run at least once.");
		return m_ratiosSum / simulation::ISimulation<FloatType>::getRunCount();
	}


	void run() override {
		const int runCount = simulation::ISimulation<FloatType>::getRunCount();
		const int numPoints = simulation::ISimulation<FloatType>::getPolygonPointCount();
		const int totalPoints = runCount * numPoints;
		
		// Generate all random numbers upfront in a single batch
		std::vector<FloatType> allXCoords;
		std::vector<FloatType> allYCoords;
		allXCoords.reserve(totalPoints);
		allYCoords.reserve(totalPoints);
		
		for (int i {0}; i < totalPoints; i++) {
			allXCoords.push_back(m_dist(m_mt));
			allYCoords.push_back(m_dist(m_mt));
		}
		
		// Process all simulations in a vectorized batch
		FloatType sumOfRatios = 0.0;
		
		for (int simIdx = 0; simIdx < runCount; ++simIdx) {
			// Extract coordinates for this simulation
			const int offset = simIdx * numPoints;
			const FloatType* xCoords = allXCoords.data() + offset;
			const FloatType* yCoords = allYCoords.data() + offset;
			
			// Calculate polygon area
			FloatType polygonArea = getPolygonAreaVectorized(xCoords, yCoords, numPoints);
			
			// Calculate bounding box using vectorized operations
			const auto& [bottomLeft, topRight] = getBoundingBoxCornersVectorized(
				xCoords, yCoords, numPoints);
			const FloatType width = topRight.x - bottomLeft.x;
			const FloatType height = topRight.y - bottomLeft.y;
			const FloatType boundingBoxArea = width * height;
			
			const FloatType ratio = polygonArea / boundingBoxArea;
			sumOfRatios += ratio;
		}
		
		m_ratiosSum = sumOfRatios;
	}

	FloatType getSumOfRatios() const override {
		return m_ratiosSum;
	}

private:
	struct Point {
		FloatType x {};
		FloatType y {};
	};

	FloatType m_ratiosSum {};
	std::mt19937 m_mt {std::random_device{}()};
	std::uniform_real_distribution<FloatType> m_dist {1.0, 2.0};

	static FloatType getPolygonAreaVectorized(
		const FloatType* xCoords,
		const FloatType* yCoords,
		int numPoints) {
		// Use Highway for vectorized polygon area calculation (shoelace formula)
		// For each point i: area += x[i] * y[i+1] - x[i+1] * y[i]
		// Then handle wrap-around: area += x[last] * y[0] - x[0] * y[last]
		
		FloatType area = 0.0;
		
		// Process pairs sequentially - vectorization is tricky due to dependency on i+1
		// For now, use simple loop to avoid bounds issues
		for (int i = 0; i < numPoints - 1; ++i) {
			area += xCoords[i] * yCoords[i + 1] - xCoords[i + 1] * yCoords[i];
		}
		
		// Handle wrap-around: last point with first point
		area += xCoords[numPoints - 1] * yCoords[0] - xCoords[0] * yCoords[numPoints - 1];
		
		return std::abs(area) / static_cast<FloatType>(2.0);
	}

	static std::pair<Point, Point> getBoundingBoxCornersVectorized(
		const FloatType* xCoords,
		const FloatType* yCoords,
		int numPoints) {
		assert(numPoints > 0);

		// Use Highway for vectorized min/max operations directly on x/y arrays
		namespace hn = hwy::HWY_NAMESPACE;
		const hn::ScalableTag<FloatType> d;
		const size_t N = hn::Lanes(d);

		// Initialize with first element
		FloatType minX = xCoords[0];
		FloatType maxX = xCoords[0];
		FloatType minY = yCoords[0];
		FloatType maxY = yCoords[0];

		if (N > 0 && numPoints > 1) {
			size_t i = 0;
			
			// Initialize reduction vectors with first values
			auto vMinX = hn::Set(d, xCoords[0]);
			auto vMaxX = hn::Set(d, xCoords[0]);
			auto vMinY = hn::Set(d, yCoords[0]);
			auto vMaxY = hn::Set(d, yCoords[0]);
			
			// Process vectors of coordinates directly from arrays
			for (; i + N <= static_cast<size_t>(numPoints); i += N) {
				auto vx = hn::Load(d, xCoords + i);
				auto vy = hn::Load(d, yCoords + i);
				
				// Update min/max vectors using vectorized operations
				vMinX = hn::Min(vMinX, vx);
				vMaxX = hn::Max(vMaxX, vx);
				vMinY = hn::Min(vMinY, vy);
				vMaxY = hn::Max(vMaxY, vy);
			}
			
			// Reduce vectors to scalars using Highway's built-in reduction functions
			minX = std::min(minX, hn::GetLane(hn::MinOfLanes(d, vMinX)));
			maxX = std::max(maxX, hn::GetLane(hn::MaxOfLanes(d, vMaxX)));
			minY = std::min(minY, hn::GetLane(hn::MinOfLanes(d, vMinY)));
			maxY = std::max(maxY, hn::GetLane(hn::MaxOfLanes(d, vMaxY)));
			
			// Handle remaining elements
			for (; i < static_cast<size_t>(numPoints); ++i) {
				minX = std::min(minX, xCoords[i]);
				maxX = std::max(maxX, xCoords[i]);
				minY = std::min(minY, yCoords[i]);
				maxY = std::max(maxY, yCoords[i]);
			}
		} else {
			// Fallback for small arrays or when SIMD is not available
			for (int i = 1; i < numPoints; ++i) {
				minX = std::min(minX, xCoords[i]);
				maxX = std::max(maxX, xCoords[i]);
				minY = std::min(minY, yCoords[i]);
				maxY = std::max(maxY, yCoords[i]);
			}
		}

		Point bottomLeft {minX, minY};
		Point topRight {maxX, maxY};

		return {bottomLeft, topRight};
	}
};

#endif

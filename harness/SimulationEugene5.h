#ifndef SIMULATION_EUGENE5_H
#define SIMULATION_EUGENE5_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <limits>
#include <random>
#include <vector>

#include "simulation/ISimulation.h"
/**
 * @brief Eugene5 simulation
 * This simulation uses a to minimize random number generation, just like Eugene4.  However,
 * it is not limited to 3 point polygons and can work on any number of points.
 */
template <std::floating_point FloatType>
class SimulationEugene5 : public simulation::ISimulation<FloatType> {
public:

	SimulationEugene5(int runCount, int polygonPointCount = 3) :
		simulation::ISimulation<FloatType>(runCount, polygonPointCount),
		m_ratiosSum {0} {
	}


	FloatType getAverageRatio() const override {
		assert(simulation::ISimulation<FloatType>::getRunCount() > 0 && "Must run at least once.");
		return m_ratiosSum / simulation::ISimulation<FloatType>::getRunCount();
	}

	void run() override {
		 auto runCount {simulation::ISimulation<FloatType>::getRunCount()};
		const auto pointCount {simulation::ISimulation<FloatType>::getPolygonPointCount()};
		const auto coordsCount {pointCount * 2};
		std::vector<FloatType> coords {};
		coords.resize(coordsCount);
		
		// init
		for (int i {0}; i < coordsCount; i++) {
			coords[i] = m_dist(m_mt);
		}

		auto shift {0};
		auto ratioSum {0.0};
		for (int r {0}; r<runCount; ++r) {
			FloatType area {0};
			auto bottomLeftX {std::numeric_limits<FloatType>::max()};
			auto bottomLeftY {std::numeric_limits<FloatType>::max()};
			auto topRightX {std::numeric_limits<FloatType>::min()};
			auto topRightY {std::numeric_limits<FloatType>::min()};

			for (int i {0}; i < pointCount; i++) {
				auto currIdx = shift + 2*i;
				auto currX = coords[(currIdx + 0) % coordsCount];
				auto currY = coords[(currIdx + 1) % coordsCount];
				auto nextX = coords[(currIdx + 2) % coordsCount];
				auto nextY = coords[(currIdx + 3) % coordsCount];
				area += currX * nextY - nextX * currY;

				bottomLeftX = std::min(bottomLeftX, currX);
				bottomLeftY = std::min(bottomLeftY, currY);
				topRightX = std::max(topRightX, currX);
				topRightY = std::max(topRightY, currY);
			}
			area = std::abs(area) / static_cast<FloatType>(2.0);
			auto boundingBoxArea = (topRightX - bottomLeftX) * (topRightY - bottomLeftY);
			ratioSum += area / boundingBoxArea;
			
			coords[shift] = m_dist(m_mt); // update last coord
			shift = (shift + 1) % coordsCount; // shift window

		}
		m_ratiosSum = ratioSum;
	}

	FloatType getSumOfRatios() const override {
		return m_ratiosSum;
	}

private:
	FloatType m_ratiosSum {};
	std::mt19937 m_mt {std::random_device{}()};
	std::uniform_real_distribution<FloatType> m_dist {1.0, 2.0};
};

#endif

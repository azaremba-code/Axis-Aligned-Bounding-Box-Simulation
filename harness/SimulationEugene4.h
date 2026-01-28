#ifndef SIMULATION_EUGENE4_H
#define SIMULATION_EUGENE4_H

#include <cassert>
#include <cmath>
#include <algorithm>
#include <concepts>
#include <random>

#include "simulation/ISimulation.h"

/**
 * @brief Eugene4 simulation
 * This simulation works on exactly 3 point polygons.  We use a sliding window to minimize
 * random number generation.
 * @note We do not use array to minimize memory usage.  The expection is that the compile will 
 *       optimize the locals and use registers for everything.
 */
template <std::floating_point FloatType>
class SimulationEugene4 : public simulation::ISimulation<FloatType> {
public:

	SimulationEugene4(int runCount, int polygonPointCount = 3) :
		simulation::ISimulation<FloatType>(runCount, polygonPointCount),
		m_ratiosSum {0} {
		assert(polygonPointCount == 3 && "This simulation only supports 3-point polygons.");
	}


	FloatType getAverageRatio() const override {
		assert(simulation::ISimulation<FloatType>::getRunCount() > 0 && "Must run at least once.");
		return m_ratiosSum / simulation::ISimulation<FloatType>::getRunCount();
	}


	void run() override {
		auto runCount = simulation::ISimulation<FloatType>::getRunCount();

		auto ratioSum {0.0};

		auto aX = m_dist(m_mt);
		auto aY = m_dist(m_mt);
		auto bX = m_dist(m_mt);
		auto bY = m_dist(m_mt);
		auto cX = m_dist(m_mt);
		auto cY = m_dist(m_mt);

		for (int i {runCount}; i; --i) {
			auto polygonArea = std::abs(aX * (bY - cY) + bX * (cY - aY) + cX * (aY - bY)) / 2.0;
			auto width = std::max(std::abs(aX - bX), std::max(std::abs(aX - cX), std::abs(bX - cX)));
			auto height = std::max(std::abs(aY - bY), std::max(std::abs(aY - cY), std::abs(bY - cY)));
			auto boundingBoxArea = width * height;
			auto ratio = polygonArea / boundingBoxArea;
			ratioSum += ratio;

			aX = aY;
			aY = bX;
			bX = bY;
			bY = cX;
			cX = cY;
			cY = m_dist(m_mt);
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

#ifndef SIMULATION_EUGENE4_H
#define SIMULATION_EUGENE4_H

#include <cassert>
#include <cmath>
#include <algorithm>
#include <concepts>
#include <random>

#include "simulation/ISimulation.h"
#include "rng/Xorshift128PlusSSE2.h"

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

		// auto aX = 0.0;
		// auto aY = 0.0;
		// auto aX = m_dist(m_mt);
		// auto aY = m_dist(m_mt);
		// auto bX = m_dist(m_mt);
		// auto bY = m_dist(m_mt);
		// auto cX = m_dist(m_mt);
		// auto cY = m_dist(m_mt);
		__m128d pt = m_rng.nextPoint(1.0, 2.0);  // one (x,y) per call, all in registers
		auto aX = rng::Xorshift128PlusSSE2::getX(pt);
		auto aY = rng::Xorshift128PlusSSE2::getY(pt);
		pt = m_rng.nextPoint(1.0, 2.0);  // one (x,y) per call, all in registers
		auto bX = rng::Xorshift128PlusSSE2::getX(pt);
		auto bY = rng::Xorshift128PlusSSE2::getY(pt);
		pt = m_rng.nextPoint(1.0, 2.0);  // one (x,y) per call, all in registers
		auto cX = rng::Xorshift128PlusSSE2::getX(pt);
		auto cY = rng::Xorshift128PlusSSE2::getY(pt);

		for (int i {runCount}; i; --i) {
			auto polygonArea = std::abs(aX * (bY - cY) + bX * (cY - aY) + cX * (aY - bY)) / 2.0;
			auto width = std::max(std::abs(aX - bX), std::max(std::abs(aX - cX), std::abs(bX - cX)));
			auto height = std::max(std::abs(aY - bY), std::max(std::abs(aY - cY), std::abs(bY - cY)));
			// auto polygonArea = std::abs(bX * cY - cX * bY) / 2.0;
			// auto width = std::max(bX, cX);
			// auto height = std::max(bY, cY);
			auto boundingBoxArea = width * height;
			auto ratio = polygonArea / boundingBoxArea;
			ratioSum += ratio;

			// aX = aY;
			// aY = bX;
			// bX = bY;
			// bY = cX;
			// cX = cY;
			// cY = m_dist(m_mt);

			// aX = m_dist(m_mt);
			// aY = m_dist(m_mt);
			// bX = m_dist(m_mt);
			// bY = m_dist(m_mt);
			// cX = m_dist(m_mt);
			// cY = m_dist(m_mt);

			pt = m_rng.nextPoint(1.0, 2.0);  // one (x,y) per call, all in registers
			aX = rng::Xorshift128PlusSSE2::getX(pt);
			aY = rng::Xorshift128PlusSSE2::getY(pt);			
			pt = m_rng.nextPoint(1.0, 2.0);  // one (x,y) per call, all in registers
			bX = rng::Xorshift128PlusSSE2::getX(pt);
			bY = rng::Xorshift128PlusSSE2::getY(pt);
			pt = m_rng.nextPoint(1.0, 2.0);  // one (x,y) per call, all in registers
			cX = rng::Xorshift128PlusSSE2::getX(pt);
			cY = rng::Xorshift128PlusSSE2::getY(pt);
		}
		m_ratiosSum = ratioSum;
	}

	FloatType getSumOfRatios() const override {
		return m_ratiosSum;
	}

private:
	FloatType m_ratiosSum {};
	std::mt19937 m_mt {std::random_device{}()};
	std::uniform_real_distribution<FloatType> m_dist {0.0, 1.0};

	rng::Xorshift128PlusSSE2 m_rng {42};
};

#endif

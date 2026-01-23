#ifndef SIMULATION_ADRIAN1_H
#define SIMULATION_ADRIAN1_H

#include <cassert>
#include <cmath>
#include <algorithm>
#include <concepts>
#include <random>
#include <vector>
#include <utility>

#include "simulation/ISimulation.h"

template <std::floating_point FloatType>
class SimulationAdrian1 : public simulation::ISimulation<FloatType> {
public:

	SimulationAdrian1(int runCount, int polygonPointCount = 3) :
		simulation::ISimulation<FloatType>(runCount, polygonPointCount),
		m_ratiosSum {0}
	{}


	FloatType getAverageRatio() const override {
		assert(simulation::ISimulation<FloatType>::getRunCount() > 0 && "Must run at least once.");
		return m_ratiosSum / simulation::ISimulation<FloatType>::getRunCount();
	}


	void run() override {
		for (int i {1}; i <= simulation::ISimulation<FloatType>::getRunCount(); ++i) {
			runOne();
		}
	}

	FloatType getSumOfRatios() const override {
		return m_ratiosSum;
	}

	private:
	void runOne() {
		std::vector<Point> points {};
		points.reserve(simulation::ISimulation<FloatType>::getPolygonPointCount());

		
		// std::generate_n(std::back_inserter(points), m_polygonPointCount, [this]() {
		// 	return Point {m_dist(m_mt), m_dist(m_mt)};
		// });
			
		for (int i {0}; i < simulation::ISimulation<FloatType>::getPolygonPointCount(); i++) {
			FloatType x {m_dist(m_mt)};
			FloatType y {m_dist(m_mt)};
			points.emplace_back(x, y);
		}
			
		FloatType polygonArea {getPolygonArea(points)};
		
		const auto& [bottomLeft, topRight] = getBoundingBoxCorners(points);
		const FloatType width {topRight.x - bottomLeft.x};
		const FloatType height {topRight.y - bottomLeft.y};
		FloatType boundingBoxArea {width * height};

		FloatType ratio {polygonArea / boundingBoxArea};
		m_ratiosSum += ratio;
	}

private:
	struct Point {
		FloatType x {};
		FloatType y {};
	};

	FloatType m_ratiosSum {};
	std::mt19937 m_mt {std::random_device{}()};
	std::uniform_real_distribution<FloatType> m_dist {1.0, 2.0};

	static FloatType getPolygonArea(const std::vector<Point>& points) {
		FloatType area {0};
		for (int i {0}; i < std::ssize(points); i++) {
			const Point& curr {points.data()[i]};
			const int nextIndex {(i + 1) % static_cast<int>(std::ssize(points))};
			const Point& next {points.data()[nextIndex]};

			area += curr.x * next.y - next.x * curr.y;
		}
		
		return std::abs(area) / static_cast<FloatType>(2.0);
	}

	static std::pair<Point, Point> getBoundingBoxCorners(const std::vector<Point>& points) {
		assert(points.size() > 0);

		Point bottomLeft {points[0]};
		Point topRight {points[0]};

		for (const auto& point : points) {
			bottomLeft.x = std::min(bottomLeft.x, point.x);
			bottomLeft.y = std::min(bottomLeft.y, point.y);

			topRight.x = std::max(topRight.x, point.x);
			topRight.y = std::max(topRight.y, point.y);
		}

		return {bottomLeft, topRight};
	}
};

#endif

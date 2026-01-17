#ifndef SIMULATION_H
#define SIMULATION_H

#include <cassert>
#include <cmath>

#include "Random.h"

template <std::floating_point FloatType>
class Simulation {
public:
	Simulation(int polygonPointCount = 3) :
		m_polygonPointCount {polygonPointCount},
		m_runCount {0},
		m_RatiosSum {0}
	{
		assert(m_polygonPointCount >= 3 && "Polygons must have at least 3 points.");
	}

	FloatType getAverageRatio() {
		assert(m_runCount > 0 && "Must run at least once.");
		return m_RatiosSum / m_runCount;
	}

	void runMany(int runCount = 1) {
		for (int i {1}; i <= runCount; i++) {
			run();
		}
	}

	void run() {
		std::vector<Point> points {};
		points.reserve(m_polygonPointCount);

		for (int i {0}; i < m_polygonPointCount; i++) {
			FloatType x {Random::get<FloatType>()};
			FloatType y {Random::get<FloatType>()};
			points.emplace_back(x, y);
		}
		
		FloatType polygonArea {getPolygonArea(points)};

		const auto& [bottomLeft, topRight] = getBoundingBoxCorners(points);
		const FloatType width {topRight.x - bottomLeft.x};
		const FloatType height {topRight.y - bottomLeft.y};
		FloatType boundingBoxArea {width * height};

		FloatType ratio {polygonArea / boundingBoxArea};
		m_RatiosSum += ratio;
		m_runCount++;	
	}

private:
	struct Point {
		FloatType x {};
		FloatType y {};
	};

	int m_polygonPointCount {};	
	int m_runCount {};
	FloatType m_RatiosSum {};

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

#ifndef SIMULATION_H
#define SIMULATION_H

#include <cassert>
#include <cmath>
#include <algorithm>
#include <concepts>
#include <iostream>
#include <random>
#include <vector>
#include <utility>
#include <thread>
#include <sched.h>

#include "common/Concurrency.h"

template <std::floating_point FloatType>
class Simulation {
public:
	typedef FloatType floatType;
	
	Simulation(int runCount, int polygonPointCount = 3) :
		m_polygonPointCount {polygonPointCount},
		m_runCount {runCount},
		m_RatiosSum {0}
	{
		assert(m_polygonPointCount >= 3 && "Polygons must have at least 3 points.");
		// current core number
		int coreNumber = Concurrency::get_current_core();
		bool isPinned = Concurrency::is_thread_pinned();
		std::cout << "Simulation initialized with " << m_polygonPointCount << " points and " << m_runCount << " runs on core " << coreNumber << " and is " << (isPinned?"":"not ") << "pinned" << std::endl;
	}

	FloatType getRunCount() {
		return m_runCount;
	}

	FloatType getRatiosSum() {
		return m_RatiosSum;
	}

	FloatType getAverageRatio() {
		assert(m_runCount > 0 && "Must run at least once.");
		return m_RatiosSum / m_runCount;
	}

	void run() {
		for (int i {1}; i <= m_runCount; ++i) {
			runOne();
		}
	}

	private:
	void runOne() {
		std::vector<Point> points {};
		points.reserve(m_polygonPointCount);

		
		// std::generate_n(std::back_inserter(points), m_polygonPointCount, [this]() {
		// 	return Point {m_dist(m_mt), m_dist(m_mt)};
		// });
			
		for (int i {0}; i < m_polygonPointCount; i++) {
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
		m_RatiosSum += ratio;
	}

private:
	struct Point {
		FloatType x {};
		FloatType y {};
	};

	int m_polygonPointCount {};	
	int m_runCount {};
	FloatType m_RatiosSum {};
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

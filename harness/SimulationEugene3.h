#ifndef SIMULATION_EUGENE3_H
#define SIMULATION_EUGENE3_H

#include <cassert>
#include <cmath>
#include <algorithm>
#include <concepts>
#include <random>
#include <vector>

#include "simulation/ISimulation.h"

template <std::floating_point FloatType>
std::vector<FloatType> operator+(const std::vector<FloatType>& left, const std::vector<FloatType>& right)
{
	assert(left.size() == right.size());

	std::vector<FloatType> result;
	result.resize(left.size());
	for (int i {0}; i < std::ssize(left); i++) {
		result[i] = left[i] + right[i];
	}
	return result;
}

template <std::floating_point FloatType>
std::vector<FloatType> operator-(const std::vector<FloatType>& left, const std::vector<FloatType>& right)
{
	assert(left.size() == right.size());

	std::vector<FloatType> result;
	result.resize(left.size());
	for (int i {0}; i < std::ssize(left); i++) {
		result[i] = left[i] - right[i];
	}
	return result;
}

template <std::floating_point FloatType>
std::vector<FloatType> operator*(const std::vector<FloatType>& left, const std::vector<FloatType>& right)
{
	assert(left.size() == right.size());

	std::vector<FloatType> result;
	result.resize(left.size());
	for (int i {0}; i < std::ssize(left); i++) {
		result[i] = left[i] * right[i];
	}
	return result;
}

template <std::floating_point FloatType>
std::vector<FloatType> operator/(const std::vector<FloatType>& left, const std::vector<FloatType>& right)
{
	assert(left.size() == right.size());

	std::vector<FloatType> result;
	result.resize(left.size());
	for (int i {0}; i < std::ssize(left); i++) {
		result[i] = left[i] / right[i];
	}
	return result;
}

template <std::floating_point FloatType>
std::vector<FloatType> min(const std::vector<FloatType>& left, const std::vector<FloatType>& right)
{
	assert(left.size() == right.size());

	std::vector<FloatType> result;
	result.resize(left.size());
	for (int i {0}; i < std::ssize(left); i++) {
		result[i] = std::min(left[i], right[i]);
	}
	return result;
}

template <std::floating_point FloatType>
std::vector<FloatType> max(const std::vector<FloatType>& left, const std::vector<FloatType>& right)
{
	assert(left.size() == right.size());

	std::vector<FloatType> result;
	result.resize(left.size());
	for (int i {0}; i < std::ssize(left); i++) {
		result[i] = std::max(left[i], right[i]);
	}
	return result;
}

template <std::floating_point FloatType>
void min_accumulate(std::vector<FloatType>& left, const std::vector<FloatType>& right)
{
	assert(left.size() == right.size());

	for (int i {0}; i < std::ssize(left); i++) {
		left.data()[i] = std::min(left.data()[i], right.data()[i]);
	}
}

template <std::floating_point FloatType>
void max_accumulate(std::vector<FloatType>& left, const std::vector<FloatType>& right)
{
	assert(left.size() == right.size());	

	for (int i {0}; i < std::ssize(left); i++) {
		left.data()[i] = std::max(left.data()[i], right.data()[i]);
	}
}


template <std::floating_point FloatType>
class SimulationEugene3 : public simulation::ISimulation<FloatType> {
public:

	SimulationEugene3(int runCount, int polygonPointCount = 3) :
		simulation::ISimulation<FloatType>(runCount, polygonPointCount),
		m_ratiosSum {0}
	{}


	FloatType getAverageRatio() const override {
		assert(simulation::ISimulation<FloatType>::getRunCount() > 0 && "Must run at least once.");
		return m_ratiosSum / simulation::ISimulation<FloatType>::getRunCount();
	}

    std::vector<std::vector<FloatType>> polygonXPoints;
    std::vector<std::vector<FloatType>> polygonYPoints;
	
	void run() override {

		// init polygon points
		int polygonPointCount = simulation::ISimulation<FloatType>::getPolygonPointCount();
		int runCount = simulation::ISimulation<FloatType>::getRunCount();

		polygonXPoints.resize(polygonPointCount);
		for (int i {0}; i < polygonPointCount; i++) {
			polygonXPoints[i].resize(runCount);
			for (int j {0}; j < runCount; j++) {
				polygonXPoints.data()[i].data()[j] = m_dist(m_mt);
			}
		}
		polygonYPoints.resize(polygonPointCount);
		for (int i {0}; i < polygonPointCount; i++) {
			polygonYPoints[i].resize(runCount);
			for (int j {0}; j < runCount; j++) {
				polygonYPoints.data()[i].data()[j] = m_dist(m_mt);
			}
		}

		// int polygonPointCount {3};
		// int runCount {2};
		// polygonXPoints.resize(polygonPointCount);
		// polygonYPoints.resize(polygonPointCount);
		// for (int i {0}; i < polygonPointCount; i++) {
		// 	polygonXPoints[i].resize(runCount);
		// 	polygonYPoints[i].resize(runCount);
		// }

		// polygonXPoints[0] = {0.0, 0.0};
		// polygonYPoints[0] = {0.0, 0.0};
		// polygonXPoints[1] = {1.0, 0.5};
		// polygonYPoints[1] = {0.0, 0.0};
		// polygonXPoints[2] = {1.0, 1.0};
		// polygonYPoints[2] = {1.0, 1.0};

		// calculate bounding box corners
		// setup bottom left points as a copy of the first polygon vector
		std::vector<FloatType> bottomLeftXPoints {polygonXPoints[0]};
		std::vector<FloatType> bottomLeftYPoints {polygonYPoints[0]};
		std::vector<FloatType> topRightXPoints {polygonXPoints[0]};
		std::vector<FloatType> topRightYPoints {polygonYPoints[0]};

		for (int p {1}; p < polygonPointCount; p++) {
			min_accumulate(bottomLeftXPoints, polygonXPoints[p]);
			min_accumulate(bottomLeftYPoints, polygonYPoints[p]);
			max_accumulate(topRightXPoints, polygonXPoints[p]);
			max_accumulate(topRightYPoints, polygonYPoints[p]);
		}

		// calc widths and heights
		std::vector<FloatType> widths {topRightXPoints - bottomLeftXPoints};
		std::vector<FloatType> heights {topRightYPoints - bottomLeftYPoints};
		// calc areas of the bounding boxes
		std::vector<FloatType> areas {widths * heights};


		// calc areas of the polygons
		std::vector<FloatType> polygonAreas {};
		polygonAreas.resize(runCount);
		for (int i {0}; i < runCount; i++) {
			//polygonAreas[i] = 0.0;
			for (int p {0}; p < polygonPointCount; p++) {
				auto currIdx = p;
				auto nextIdx = (p + 1) % polygonPointCount;
				polygonAreas.data()[i] += polygonXPoints.data()[currIdx].data()[i] * polygonYPoints.data()[nextIdx].data()[i] - polygonXPoints.data()[nextIdx].data()[i] * polygonYPoints.data()[currIdx].data()[i];
			}
			polygonAreas.data()[i] = std::abs(polygonAreas.data()[i]) / 2.0;
		}

		// calc ratios
		std::vector<FloatType> ratios {polygonAreas / areas};
		m_ratiosSum += std::reduce(ratios.begin(), ratios.end());

        // print every poligon as a sequence of (x,y) points, its box width and height, and its area, the polygon area, and the ratio, all on a single line per polygon
		// for (int i {0}; i < runCount; i++) {
		// 	std::cout << "Polygon: ";
		// 	for (int j {0}; j < polygonPointCount; j++) {
		// 		std::cout << "(" << polygonXPoints[j][i] << ", " << polygonYPoints[j][i] << ") ";
		// 	}
		// 	std::cout << "Width: " << widths[i] << ", Height: " << heights[i] << ", Area: " << areas[i] << ", Polygon Area: " << polygonAreas[i] << ", Ratio: " << ratios[i] << std::endl;
		// }
		// std::cout << std::endl;
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

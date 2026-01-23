#ifndef SIMULATION_ISIMULATION_H
#define SIMULATION_ISIMULATION_H

#include <cassert>
#include <concepts>

namespace simulation {

/**
 * Abstract base class for polygon simulation implementations.
 * Provides common functionality for calculating polygon-to-bounding-box ratios.
 */
template <std::floating_point FloatType>
class ISimulation {
public:
	/**
	 * Constructor for the simulation base.
	 * @param polygonPointCount Number of points in each polygon (must be >= 3).
	 */
	explicit ISimulation(int runCount, int polygonPointCount = 3) :
		m_runCount {runCount},
		m_polygonPointCount {polygonPointCount}
	{
		assert(m_polygonPointCount >= 3 && "Polygons must have at least 3 points.");
	}

	virtual ~ISimulation() = default;

    /**
     * Runs a complete simulation.
     */
    virtual void run() = 0;
    
	/**
	 * Gets the average ratio of polygon area to bounding box area. Effectively, the result of the simulation.
	 * @return The average ratio.
	 */
	virtual FloatType getAverageRatio() const {
		return getSumOfRatios() / getRunCount();
	}

    /**
	 * Gets the sum of all ratios calculated.  Needed for multi-threaded simulations.
	 * @return The sum of ratios.
	 */
	virtual FloatType getSumOfRatios() const = 0;

	/**
	 * Gets the total number of runs executed.
	 * @return The run count.
	 */
	int getRunCount() const {
		return m_runCount;
	}

	/**
	 * Gets the number of points per polygon.
	 * @return The polygon point count.
	 */
	int getPolygonPointCount() const {
		return m_polygonPointCount;
	}

protected:
	int m_runCount;
	int m_polygonPointCount {};
};

} // namespace simulation

#endif // SIMULATION_ISIMULATION_H

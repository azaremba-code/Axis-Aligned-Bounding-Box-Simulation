#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>

//RAII Timer
class Timer {
public:
	using DurationType = std::chrono::time_point<std::chrono::steady_clock>;
	using TimeElapsedType = std::chrono::duration<double>;
	Timer(bool start = true) : m_startTime {std::chrono::steady_clock::now()}, m_isRunning(start)
	{}

	void start() {
		if (m_isRunning) {
			return;
		}
		m_startTime = std::chrono::steady_clock::now();
		m_isRunning = true;
	}

	void stop() {
		if (!m_isRunning) {
			return;
		}
		m_isRunning = false;
		DurationType now {std::chrono::steady_clock::now()};
		m_timeElapsed += now - m_startTime;
	}

	TimeElapsedType getTimeElapsed() const {
		return m_timeElapsed;
	}

	void printTime(std::string_view name) {
		bool wasRunning = m_isRunning;
		if (wasRunning) {
			stop();
		} 
		std::cout << "Time taken [" << name << "]: " << m_timeElapsed.count() << std::endl;
		if (wasRunning) {
			start();
		}
	}

	void reset() {
		m_timeElapsed = TimeElapsedType::zero();
		m_isRunning = false;
	}

	bool isRunning() const {
		return m_isRunning;
	}

	~Timer() {
		if (m_isRunning) {
			stop();
			printTime("total");
		}
	}

private:
	DurationType m_startTime {};
	TimeElapsedType m_timeElapsed {TimeElapsedType::zero()};
	bool m_isRunning {false};
};

#endif

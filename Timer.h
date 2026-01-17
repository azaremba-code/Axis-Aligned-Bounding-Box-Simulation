#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>

//RAII Timer
class Timer {
public:
	using DurationType = std::chrono::time_point<std::chrono::steady_clock>;
	Timer() : m_creationTime {std::chrono::steady_clock::now()}
	{}

	~Timer() {
		DurationType deletionTime {std::chrono::steady_clock::now()};
		std::chrono::duration<double> timeElapsed {deletionTime - m_creationTime};
		std::cout << "Time taken: " << timeElapsed << std::endl;
	}

private:
	DurationType m_creationTime {};
};

#endif

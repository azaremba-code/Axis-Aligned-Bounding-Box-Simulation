#ifndef RANDOM_H
#define RANDOM_H

#include <random>
#include <concepts>

class Random {
public:
	Random() = delete;

	template <std::floating_point FloatType>
	static FloatType get(FloatType min = 1.0, FloatType max = 2.0) {
		return std::uniform_real_distribution {min, max} (mt);
	}

private:
	static inline std::random_device rd {};
	static inline std::mt19937 mt {rd()};
};
#endif

#pragma once

#include <random>

#undef INFINITY

static constexpr float INFINITY = std::numeric_limits<float>::infinity();
static constexpr float PI = 3.1415926535897932385;
static constexpr float EPSILON = 0.0001f;

inline float deg2rad(float d) {
	return d * PI / 180.0;
}

inline float rand_float(float min, float max) {
	static std::uniform_real_distribution<float> d(min, max);
	static std::mt19937 g;
	return d(g);
}

inline float rand_float() {
	return rand_float(0.0, 1.0);
}

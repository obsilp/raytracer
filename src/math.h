#pragma once

#include <random>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>
#include <glm/matrix.hpp>

#undef INFINITY

static constexpr float INFINITY = std::numeric_limits<float>::infinity();
static constexpr float PI = 3.1415926535897932385;
static constexpr float EPSILON = 0.0001f;

static inline float deg2rad(float d) {
	return d * PI / 180.0;
}

static inline float rand_float(float min, float max) {
	static std::uniform_real_distribution<float> d(min, max);
	static std::mt19937 g;
	return d(g);
}

static inline float rand_float() {
	return rand_float(0.0, 1.0);
}

static inline glm::vec3 uniform_sample_hemisphere(float u1, float u2) {
	auto sin_theta = glm::sqrt(u1);
	auto cos_theta = glm::sqrt(1.f - u1);
	auto phi = 2.f * PI * u2;
	return {sin_theta * glm::cos(phi), cos_theta, sin_theta * glm::sin(phi)};
}

static inline glm::vec3 align_tbn(const glm::vec3 &v, const glm::vec3 &n, const glm::vec3 &t) {
	auto b = glm::cross(n, t);
	return {
			v.x * t.x + v.y * n.x + v.z * b.x,
			v.x * t.y + v.y * n.y + v.z * b.y,
			v.x * t.z + v.y * n.z + v.z * b.z,
	};
}

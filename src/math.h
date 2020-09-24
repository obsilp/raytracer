#pragma once

#include <random>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>

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
	auto r = glm::sqrt(1.f - u1 * u1);
	auto phi = 2.f * PI * u2;
	return {glm::cos(phi) * r, u1, glm::sin(phi) * r};
}

static inline glm::vec3 create_tangent(const glm::vec3 &normal) {
	glm::vec3 tangent;
	if (glm::abs(normal.x) > glm::abs(normal.y))
		tangent = {normal.z, 0, -normal.x};
	else
		tangent = {0, -normal.z, normal.y};
	return glm::normalize(tangent);
}

static inline glm::vec3 align_nbt(const glm::vec3 &v, const glm::vec3 &n, const glm::vec3 &t) {
	auto b = glm::cross(n, t);
	return v.x * n + v.y * b + v.z * t;
}

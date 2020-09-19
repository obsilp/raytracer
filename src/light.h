#pragma once

#include <glm/vec3.hpp>

struct DirectionalLight {
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;
};

struct AreaLight {
	glm::vec3 color;
	float intensity;
	short u_samples;
	short v_samples;
	float max_random_offset;
};

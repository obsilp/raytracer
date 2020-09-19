#pragma once

#include <glm/vec3.hpp>

struct DirectionalLight {
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;
};

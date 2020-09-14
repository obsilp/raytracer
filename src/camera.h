#pragma once

#include <glm/vec3.hpp>

struct Camera {
	glm::vec3 position;
	glm::vec3 look_at;

	float vfov;
	float focal_length;

	glm::vec3 lower_left_corner;
	glm::vec3 span_horizontal;
	glm::vec3 span_vertical;
};

void init_camera(Camera &cam, int width, int height);

struct Ray ray_from_camera(const Camera &cam, float u, float v);

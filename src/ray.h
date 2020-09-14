#pragma once

#include <glm/vec3.hpp>

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

inline glm::vec3 ray_at(const Ray &ray, float t) {
	return ray.origin + ray.direction * t;
}

struct HitRecord {
	float distance;

	glm::vec3 position;
	glm::vec3 normal;

	bool front_facing;
};

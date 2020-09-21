#pragma once

#include <glm/vec3.hpp>

#include "material.h"
#include "scene.h"

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

inline glm::vec3 ray_at(const Ray &ray, float t) {
	return ray.origin + ray.direction * t;
}

glm::vec3 ray_color(const Ray &ray, const struct Camera &camera, const Scene &scene, Stats &stats);

struct HitRecord {
	EntityId entity_id;
	float distance;

	glm::vec3 position;
	glm::vec3 normal;

	bool front_facing;

	const Material *material;
};

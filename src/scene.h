#pragma once

#include <optional>
#include <vector>

#include <glm/vec3.hpp>

struct Sphere {
	glm::vec3 position;
	float radius;
};

struct Plane {
	glm::vec3 position;
	glm::vec3 normal;
};

struct Scene {
	std::vector<Sphere> spheres;
	std::vector<Plane> planes;
};

std::optional<struct HitRecord> hit_scene(const struct Ray &ray, const Scene &scene);

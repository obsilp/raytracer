#pragma once

#include <optional>
#include <vector>

#include <glm/vec3.hpp>

#include "light.h"
#include "material.h"
#include "math.h"

struct Sphere {
	glm::vec3 position;
	float radius;
};

struct Plane {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;

	float width = INFINITY;
	float height = INFINITY;
};

struct Scene {
	glm::vec3 ambient_light;

	std::vector<Sphere> spheres;
	std::vector<Material> sphere_materials;

	std::vector<Plane> planes;
	std::vector<Material> plane_materials;

	std::vector<DirectionalLight> directional_lights;
};

std::optional<struct HitRecord> hit_scene(const struct Ray &ray, const Scene &scene);

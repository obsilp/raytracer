#pragma once

#include <optional>
#include <vector>

#include <glm/vec2.hpp>
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
	glm::vec3 bi_tangent;

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

void make_box(Scene &scene, const Material &material, const glm::vec3 &position, const glm::vec3 &size);

void make_rect(Scene &scene, const Material &material, const glm::vec3 &position, const glm::vec3 &normal,
			   const glm::vec3 &tangent, const glm::vec2 &size);

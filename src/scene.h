#pragma once

#include <array>
#include <atomic>
#include <optional>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

#include "light.h"
#include "material.h"
#include "math.h"

using EntityId = unsigned short;
static const EntityId NULL_ENTITY = 0;

struct Sphere {
	EntityId id;

	glm::vec3 position;
	float radius;
};

struct Plane {
	EntityId id;

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bi_tangent;

	float width = INFINITY;
	float height = INFINITY;
};

struct Scene {
	EntityId next_entity_id = 1;

	glm::vec3 ambient_light;

	std::vector<Sphere> spheres;
	std::vector<Material> sphere_materials;

	std::vector<Plane> planes;
	std::vector<Material> plane_materials;

	std::vector<DirectionalLight> directional_lights;

	std::vector<Plane> area_lights;
	std::vector<AreaLight> area_light_data;
};

struct Stats {
	std::atomic<unsigned int> ray_count = 0;
};

std::optional<struct HitRecord> hit_scene(const struct Ray &ray, const Scene &scene, Stats &stats);

static EntityId add_sphere(Scene &scene, Sphere obj, const Material &material) {
	obj.id = scene.next_entity_id++;
	scene.spheres.emplace_back(obj);
	scene.sphere_materials.push_back(material);
	return obj.id;
}

static EntityId add_plane(Scene &scene, const Material &material, Plane obj) {
	obj.id = scene.next_entity_id++;
	scene.planes.emplace_back(obj);
	scene.plane_materials.push_back(material);
	return obj.id;
}

static void add_planes(Scene &scene, const Material &material, std::vector<Plane> objects) {
	for (auto obj : objects)
		add_plane(scene, material, obj);
}

static void add_area_light(Scene &scene, const AreaLight &light, Plane obj) {
	obj.id = add_plane(scene, make_mat_invisible(), obj);
	scene.area_lights.emplace_back(obj);
	scene.area_light_data.push_back(light);
}

std::vector<Plane> make_box(const glm::vec3 &position, const glm::vec3 &size,
							const glm::mat4 &transform = glm::mat4(1.0f));

Plane make_rect(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec3 &tangent, const glm::vec2 &size,
				const glm::mat4 &transform = glm::mat4(1.0f));

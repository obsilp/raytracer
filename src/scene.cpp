#include "scene.h"

#include <tuple>
#include <glm/glm.hpp>

#include "ray.h"
#include "math.h"

std::optional<float> intersect_sphere(const Ray &ray, const Sphere &sphere) {
	auto l = sphere.position - ray.origin;
	auto tca = glm::dot(l, ray.direction);

	auto radius2 = sphere.radius * sphere.radius;
	auto d2 = glm::dot(l, l) - tca * tca;
	if (d2 > radius2) return {};

	auto thc = glm::sqrt(radius2 - d2);
	auto t0 = tca - thc;
	auto t1 = tca + thc;

	if (t0 > t1) std::swap(t0, t1);

	if (t0 < 0) {
		t0 = t1;
		if (t0 < 0) return {};
	}

	return t0;
}

std::optional<float> intersect_plane(const Ray &ray, const Plane &plane) {
	auto d = glm::dot(plane.normal, ray.direction);
	if (glm::abs(d) < EPSILON) return {};

	auto p = plane.position - ray.origin;
	auto t = glm::dot(p, plane.normal) / d;
	if (t < 0) return {};

	p = ray_at(ray, t) - plane.position;

	if (glm::abs(glm::dot(p, plane.tangent)) > plane.height * .5f) return {};
	if (glm::abs(glm::dot(p, plane.bi_tangent)) > plane.width * .5f) return {};

	return t;
}


std::optional<HitRecord> hit_spheres(const Ray &ray, const Scene &scene, float closest) {
	int obj_idx = -1;
	for (auto i = 0; i < scene.spheres.size(); ++i) {
		auto hit_dst = intersect_sphere(ray, scene.spheres[i]);
		if (hit_dst && hit_dst < closest) {
			obj_idx = i;
			closest = hit_dst.value();
		}
	}

	if (obj_idx == -1) return {};

	auto pos = ray_at(ray, closest);
	auto normal = glm::normalize(pos - scene.spheres[obj_idx].position);
	auto front_facing = glm::dot(ray.direction, normal) < 0;

	if (!front_facing) normal = -normal;

	return HitRecord{
			.entity_id = scene.planes[obj_idx].id,
			.distance = closest,
			.position = pos,
			.normal = normal,
			.front_facing = front_facing,
			.material = &scene.sphere_materials[obj_idx],
	};
}

std::optional<HitRecord> hit_planes(const Ray &ray, const Scene &scene, float closest) {
	int obj_idx = -1;
	for (auto i = 0; i < scene.planes.size(); ++i) {
		auto hit_dst = intersect_plane(ray, scene.planes[i]);
		if (hit_dst && hit_dst < closest) {
			obj_idx = i;
			closest = hit_dst.value();
		}
	}

	if (obj_idx == -1) return {};

	auto pos = ray_at(ray, closest);
	auto normal = scene.planes[obj_idx].normal;
	auto front_facing = glm::dot(ray.direction, normal) < 0;
	if (!front_facing) normal = -normal;

	return HitRecord{
			.entity_id = scene.planes[obj_idx].id,
			.distance = closest,
			.position = pos,
			.normal = normal,
			.front_facing = front_facing,
			.material = &scene.plane_materials[obj_idx],
	};
}

std::optional<HitRecord> hit_scene(const Ray &ray, const Scene &scene) {
	auto sphere = hit_spheres(ray, scene, INFINITY);
	auto plane = hit_planes(ray, scene, sphere ? sphere->distance : INFINITY);

	if (plane) return plane;
	if (sphere) return sphere;

	return {};
}

inline glm::vec3 mat_mul(const glm::mat4 &m, const glm::vec3 &v) {
	return glm::vec3(m * glm::vec4(v, 1.f));
}

std::vector<Plane> make_box(const glm::vec3 &position, const glm::vec3 &size, const glm::mat4 &transform) {
	std::vector<Plane> planes;

	auto half_size = size * .5f;

	for (auto axis = 0; axis < 3; ++axis) {
		for (auto dir = -1; dir < 2; dir += 2) {
			glm::vec3 normal = {0, 0, 0};
			normal[axis] = dir;

			glm::vec3 tangent = {0, 0, 0};
			tangent[(axis + 1) % 3] = dir;

			auto bi_tangent = glm::normalize(glm::cross(normal, tangent));
			auto plane = make_rect(position + half_size * mat_mul(transform, normal),
								   normal,
								   tangent,
								   {glm::abs(glm::dot(bi_tangent, size)), glm::abs(glm::dot(tangent, size))},
								   transform
			);
			planes.emplace_back(plane);
		}
	}

	return planes;
}

Plane make_rect(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec3 &tangent, const glm::vec2 &size,
				const glm::mat4 &transform) {
	auto t_normal = glm::normalize(mat_mul(transform, normal));
	auto t_tangent = glm::normalize(mat_mul(transform, tangent));
	return Plane{
			.position = position,
			.normal = t_normal,
			.tangent = t_tangent,
			.bi_tangent = glm::normalize(glm::cross(t_normal, t_tangent)),
			.width = size.x,
			.height = size.y,
	};
}

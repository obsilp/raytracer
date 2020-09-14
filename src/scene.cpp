#include "scene.h"

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
	auto p = plane.position - ray.origin;
	auto t = glm::dot(p, plane.normal) / d;
	if (t < 0) return {};
	return t;
}


std::optional<HitRecord> hit_spheres(const Ray &ray, const Scene &scene, float closest) {
	std::optional<Sphere> sphere;

	for (const auto &obj : scene.spheres) {
		auto hit_dst = intersect_sphere(ray, obj);
		if (hit_dst && hit_dst < closest) {
			sphere = obj;
			closest = hit_dst.value();
		}
	}

	if (!sphere) return {};

	auto pos = ray_at(ray, closest);
	auto normal = glm::normalize(pos - sphere->position);
	auto front_facing = glm::dot(ray.direction, normal) < 0;

	if (!front_facing) normal = -normal;

	return HitRecord{
			.distance = closest,
			.position = pos,
			.normal = normal,
			.front_facing = front_facing,
	};
}

std::optional<HitRecord> hit_planes(const Ray &ray, const Scene &scene, float closest) {
	std::optional<Plane> plane;

	for (const auto &obj : scene.planes) {
		auto hit_dst = intersect_plane(ray, obj);
		if (hit_dst && hit_dst < closest) {
			plane = obj;
			closest = hit_dst.value();
		}
	}

	if (!plane) return {};

	auto pos = ray_at(ray, closest);
	auto normal = plane->normal;
	auto front_facing = glm::dot(ray.direction, normal) < 0;
	if (!front_facing) normal = -normal;

	return HitRecord{
			.distance = closest,
			.position = pos,
			.normal = normal,
			.front_facing = front_facing,
	};
}

std::optional<HitRecord> hit_scene(const Ray &ray, const Scene &scene) {
	auto sphere = hit_spheres(ray, scene, INFINITY);
	auto plane = hit_planes(ray, scene, sphere ? sphere->distance : INFINITY);

	if (plane) return plane;
	if (sphere) return sphere;

	return {};
}

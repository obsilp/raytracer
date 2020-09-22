#include "ray.h"

#include <glm/glm.hpp>

#include "camera.h"
#include "math.h"
#include "scene.h"

inline Ray secondary_ray(const glm::vec3 &origin, const glm::vec3 &direction) {
	return Ray{.origin = origin + direction * EPSILON, .direction = direction};
}

glm::vec3 blinn_phong(const HitRecord &hit, const Camera &camera, const glm::vec3 &to_light) {
	auto mat = hit.material->blinnPhong;

	auto dot_normal = glm::dot(hit.normal, to_light);
	auto diffuse = glm::clamp(dot_normal, 0.f, 1.f);
	diffuse *= mat.diffuse_intensity;

	auto specular = .0f;
	if (mat.shininess > 0.f && dot_normal > 0.001f) {
		auto to_cam = glm::normalize(camera.position - hit.position);
		auto half_way = glm::normalize(to_light + to_cam);
		specular = glm::max(glm::dot(hit.normal, half_way), 0.f);
		specular = glm::pow(specular, mat.shininess);
		specular *= mat.specular_intensity;
	}

	return (diffuse + specular) * hit.material->color;
}

glm::vec3 calc_surface_color(const HitRecord &hit, const Camera &camera, const glm::vec3 &to_light) {
	switch (hit.material->type) {
	default:
		return {0, 0, 0};

	case MaterialType::kBlinnPhong:
		return blinn_phong(hit, camera, to_light);
	}
}

glm::vec3 ray_color(const Ray &ray, const Camera &camera, const Scene &scene, Stats &stats, int max_depth) {
	if (max_depth <= 0)
		return glm::vec3(0);

	auto hit = hit_scene(ray, scene, stats);
	if (!hit || !hit->front_facing) return glm::vec3(0);

	if (hit->material->type == MaterialType::kUnlit)
		return hit->material->color;

	glm::vec3 direct_color(0.f);
	glm::vec3 indirect_color(0.f);

	// directional lights
	for (const auto &l : scene.directional_lights) {
		auto light_ray = secondary_ray(hit->position, -l.direction);
		auto light_hit = hit_scene(light_ray, scene, stats);
		if (light_hit) continue;

		auto surface_color = calc_surface_color(hit.value(), camera, -l.direction);
		direct_color += l.intensity * l.color * surface_color;
	}

	// area lights
	for (auto i = 0; i < scene.area_lights.size(); ++i) {
		const auto &plane = scene.area_lights[i];
		const auto &data = scene.area_light_data[i];
		const auto mro = data.max_random_offset;

		auto u_size = plane.width / static_cast<float>(data.u_samples);
		auto v_size = plane.height / static_cast<float>(data.v_samples);
		auto corner = plane.position - plane.bi_tangent * (plane.width * .5f) - plane.tangent * (plane.height * .5f);

		glm::vec3 area_color = {0, 0, 0};

		for (auto u = 0; u < data.u_samples; ++u) {
			for (auto v = 0; v < data.v_samples; ++v) {
				glm::vec2 offset = {
						(u + .5f + rand_float(-mro, mro)) * u_size,
						(v + .5f + rand_float(-mro, mro)) * v_size,
				};
				auto pos = corner + plane.bi_tangent * offset.x + plane.tangent * offset.y;
				auto dir = glm::normalize(pos - hit->position);

				// ignore every object above the light
				if (glm::dot(dir, plane.normal) > 0) continue;

				auto light_ray = secondary_ray(hit->position, dir);
				auto light_hit = hit_scene(light_ray, scene, stats);

				// only calculate light if ray intersects with the area light first
				if (light_hit && light_hit->entity_id != plane.id) continue;

				area_color += calc_surface_color(hit.value(), camera, dir);
			}
		}

		area_color *= data.color * data.intensity;
		area_color /= static_cast<float>(data.u_samples * data.v_samples);
		direct_color += area_color;
	}

	// indirect diffuse lighting
	if (max_depth > 1) {
		auto dir = uniform_sample_hemisphere(rand_float(), rand_float());
		auto tangent = create_tangent(hit->normal);
		dir = align_nbt(dir, hit->normal, tangent);
		dir = glm::normalize(dir);

		auto indirect_ray = secondary_ray(hit->position, dir);
		auto indirect = ray_color(indirect_ray, camera, scene, stats, max_depth - 1);
		auto cos0 = glm::max(0.f, glm::dot(hit->normal, dir));

		static const float p = 1.f / (2.f * PI);
		indirect_color += (hit->material->color / PI) * indirect * cos0 / p;
	}

	return glm::clamp(direct_color + indirect_color, 0.f, 1.f);
}

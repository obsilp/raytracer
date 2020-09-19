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

glm::vec3 ray_color(const Ray &ray, const Camera &camera, const Scene &scene) {
	auto hit = hit_scene(ray, scene);
	if (!hit || !hit->front_facing) return glm::vec3(0);

	auto color = scene.ambient_light * hit->material->color;

	for (const auto &l : scene.directional_lights) {
		auto light_ray = secondary_ray(hit->position, -l.direction);
		auto light_hit = hit_scene(light_ray, scene);
		if (light_hit) continue;

		glm::vec3 material_color(0);

		switch (hit->material->type) {
		case MaterialType::kAmbient:
			break;

		case MaterialType::kBlinnPhong:
			material_color = blinn_phong(hit.value(), camera, -l.direction);
			break;
		}

		color += l.intensity * l.color * material_color;
	}

	return color;
}

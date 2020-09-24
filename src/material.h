#pragma once

#include <glm/vec3.hpp>

enum class MaterialType {
	kUnlit = 0,
	kBlinnPhong,
};

struct MaterialBlinnPhong {
	float diffuse_intensity;
	float specular_intensity;
	float shininess;
};

struct Material {
	MaterialType type;

	glm::vec3 color;

	union {
		MaterialBlinnPhong blinnPhong;
	};
};

static inline Material make_mat_unlit(const glm::vec3 &color) {
	return Material{
			.type = MaterialType::kUnlit,
			.color = color,
	};
}

static inline Material make_mat_lambert(const glm::vec3 &color) {
	return Material{
			.type = MaterialType::kBlinnPhong,
			.color = color,
			.blinnPhong = {
					.diffuse_intensity = 1.f,
					.specular_intensity = 0.f,
					.shininess = 0.f,
			},
	};
}

#pragma once

#include <glm/vec3.hpp>

enum class MaterialType {
	kAmbient = 0,
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

inline Material make_lambert(const glm::vec3 &color) {
	return Material{
			.type = MaterialType::kBlinnPhong,
			.color = color,
			.blinnPhong = {
					.diffuse_intensity = 1.f,
					.specular_intensity = 1.f,
					.shininess = 20.f,
			},
	};
}

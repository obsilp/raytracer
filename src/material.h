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

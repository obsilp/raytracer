#include "camera.h"

#include <glm/glm.hpp>

#include "math.h"
#include "ray.h"

void init_camera(Camera &cam, int width, int height) {
	auto aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

	auto theta = deg2rad(cam.vfov);
	auto h = static_cast<float>(glm::tan(theta / 2.f));
	auto vp_height = 2.f * h;
	auto vp_width = aspect_ratio * vp_height;

	auto forward = glm::normalize(cam.look_at - cam.position);
	auto up = glm::vec3(0, 1, 0);
	auto right = glm::normalize(glm::cross(up, forward));
	up = glm::normalize(glm::cross(forward, right));

	cam.span_horizontal = vp_width * right;
	cam.span_vertical = vp_height * up;
	cam.lower_left_corner = cam.position + forward - cam.span_horizontal * .5f - cam.span_vertical * .5f;
}

Ray ray_from_camera(const Camera &cam, float u, float v) {
	auto target = cam.lower_left_corner + cam.span_horizontal * u + cam.span_vertical * v;
	return Ray{
			.origin = cam.position,
			.direction = glm::normalize(target - cam.position),
	};
}

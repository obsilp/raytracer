#include <iostream>

#include <glm/glm.hpp>
#include <random>

#include "bmp.h"

const float INF = std::numeric_limits<float>::infinity();
const float PI = 3.1415926535897932385;

inline float deg2rad(float d) {
	return d * PI / 180.0;
}

inline float rand_float() {
	static std::uniform_real_distribution<float> d(0.0, 1.0);
	static std::mt19937 g;
	return d(g);
}

struct Camera {
	glm::vec3 position;
	glm::vec3 look_at;

	float vfov;
	float focal_length;

	glm::vec3 lower_left_corner;
	glm::vec3 span_horizontal;
	glm::vec3 span_vertical;
};

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

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

glm::vec3 ray_at(const Ray &ray, float t) {
	return ray.origin + ray.direction * t;
}

struct HitRecord {
	bool hit;

	float depth;
	glm::vec3 position;
	glm::vec3 normal;
	bool front_facing;
};

struct Sphere {
	glm::vec3 position;
	float radius;
};

HitRecord hit_sphere(const Ray &ray, const Sphere &sphere) {
	auto L = sphere.position - ray.origin;
	auto tca = glm::dot(L, ray.direction);

	auto radius2 = sphere.radius * sphere.radius;
	auto d2 = glm::dot(L, L) - tca * tca;
	if (d2 > radius2) return HitRecord{.hit = false};

	auto thc = glm::sqrt(radius2 - d2);
	auto t0 = tca - thc;
	auto t1 = tca + thc;

	if (t0 > t1) std::swap(t0, t1);

	if (t0 < 0) {
		t0 = t1;
		if (t0 < 0) return HitRecord{.hit = false};
	}

	auto pos = ray_at(ray, t0);
	auto normal = glm::normalize(pos - sphere.position);
	return HitRecord{
			.hit = true,
			.depth = t0,
			.position = pos,
			.normal = normal,
			.front_facing = glm::dot(ray.direction, normal) < 0,
	};
}

#define WIDTH   (1920/2)
#define HEIGHT  (1080/2)

int main() {
	auto cam = Camera{
			.position = {0, 0, 0},
			.look_at = {0, 0, -1},

			.vfov = 90.0,
			.focal_length = 1.0,
	};
	init_camera(cam, WIDTH, HEIGHT);

	auto sphere = Sphere{
			.position = {0, 0, -3},
			.radius = 1,
	};

	FILE *f;
	long x, y;
	static char bmp[BMP_SIZE(WIDTH, HEIGHT)];

	bmp_init(bmp, WIDTH, HEIGHT);

	auto samples_base = 4;

	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH; x++) {
			auto color = glm::vec3();

			for (auto i = 0; i < samples_base * samples_base; ++i) {
				auto u = static_cast<float>(x) / WIDTH;
				auto v = static_cast<float>(y) / HEIGHT;

				auto pixel_size = 1.f / WIDTH / samples_base;
				u += (i % samples_base) * pixel_size;
				v += (i / samples_base) * pixel_size;

				auto r = ray_from_camera(cam, u, v);
				auto t = .5f * (r.direction.y + 1.f);
				auto c = (1.f - t) * glm::vec3(1.0, 1.0, 1.0) + t * glm::vec3(0.5, 0.7, 1.0);

				auto rec = hit_sphere(r, sphere);
				if (rec.hit) c = 0.5f * (rec.normal + glm::vec3(1, 1, 1));

				color += c;
			}

			auto c = color;
			c /= static_cast<float>(samples_base * samples_base);
			bmp_set(bmp, x, HEIGHT - y - 1, bmp_encode(c.r, c.g, c.b));
		}
	}

	f = fopen("test.bmp", "wb");
	fwrite(bmp, sizeof(bmp), 1, f);
	fclose(f);

	return 0;
}

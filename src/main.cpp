#include <iostream>
#include <random>
#include <chrono>

#include <glm/glm.hpp>
#include <optional>

#include "bmp.h"

#undef INFINITY

const float INFINITY = std::numeric_limits<float>::infinity();
const float PI = 3.1415926535897932385;
const float EPSILON = 0.0001f;

inline float deg2rad(float d) {
	return d * PI / 180.0;
}

inline float rand_float(float min, float max) {
	static std::uniform_real_distribution<float> d(min, max);
	static std::mt19937 g;
	return d(g);
}

inline float rand_float() {
	return rand_float(0.0, 1.0);
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
	float distance;

	glm::vec3 position;
	glm::vec3 normal;

	bool front_facing;
};

struct Sphere {
	glm::vec3 position;
	float radius;
};

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

struct Plane {
	glm::vec3 position;
	glm::vec3 normal;
};

std::optional<float> intersect_plane(const Ray &ray, const Plane &plane) {
	auto d = glm::dot(plane.normal, ray.direction);
	auto p = plane.position - ray.origin;
	auto t = glm::dot(p, plane.normal) / d;
	if (t < 0) return {};
	return t;
}

struct Scene {
	std::vector<Sphere> spheres;
	std::vector<Plane> planes;
};

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

// TODO replace
glm::vec3 random_in_unit_sphere() {
	while (true) {
		auto p = glm::vec3(
				rand_float(-1, 1),
				rand_float(-1, 1),
				rand_float(-1, 1));
		if (glm::dot(p, p) >= 1) continue;
		return p;
	}
}

glm::vec3 ray_color(const Ray &ray, const Scene &scene, int max_depth) {
	if (max_depth <= 0) return glm::vec3(0);

	auto rec = hit_scene(ray, scene);
	if (rec) {
		auto target = rec->position + rec->normal + random_in_unit_sphere();
		auto sub_ray = Ray{
				.origin = rec->position,
				.direction = glm::normalize(target - rec->position),
		};
		// TODO use cos?
		//return 0.5f * ray_color(sub_ray, scene, max_depth - 1);
		return 0.5f * (rec->normal + glm::vec3(1));
	}

	auto t = .5f * (ray.direction.y + 1.f);
	auto c = (1.f - t) * glm::vec3(1.0, 1.0, 1.0) + t * glm::vec3(0.5, 0.7, 1.0);

	return c;
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

	Scene scene;
	scene.spheres.push_back(Sphere{.position = {0, 0, -3}, .radius = 1});
	scene.planes.push_back(Plane{.position = {0, -1, 0}, .normal = {0, 1, 0}});

	FILE *f;
	long x, y;
	static char bmp[BMP_SIZE(WIDTH, HEIGHT)];

	bmp_init(bmp, WIDTH, HEIGHT);

	auto samples_base = 1;
	auto max_depth = 10;

	auto start = std::chrono::high_resolution_clock::now();

	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH; x++) {
			auto color = glm::vec3();

			for (auto i = 0; i < samples_base * samples_base; ++i) {
				auto u = static_cast<float>(x) / WIDTH;
				auto v = static_cast<float>(y) / HEIGHT;

				auto pixel_size_x = 1.f / WIDTH / samples_base;
				auto pixel_size_y = 1.f / HEIGHT / samples_base;
				u += ((i % samples_base) + .5f) * pixel_size_x;
				v += ((i / samples_base) + .5f) * pixel_size_y;

				auto r = ray_from_camera(cam, u, v);
				color += ray_color(r, scene, max_depth);
			}

			auto c = color;
			c /= static_cast<float>(samples_base * samples_base);
			bmp_set(bmp, x, HEIGHT - y - 1, bmp_encode(c.r, c.g, c.b));
		}
	}

	auto finish = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
	std::cout << duration << "ms" << std::endl;

	f = fopen("test.bmp", "wb");
	fwrite(bmp, sizeof(bmp), 1, f);
	fclose(f);

	return 0;
}

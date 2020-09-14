#include <iostream>
#include <chrono>

#include <glm/glm.hpp>

#include "bmp.h"

#include "camera.h"
#include "ray.h"
#include "scene.h"

struct Config {
	int width = 1920 / 2;
	int height = 1080 / 2;

	int samples_base = 1;
	int max_depth = 10;
};

glm::vec3 ray_color(const Ray &ray, const Scene &scene, int max_depth) {
	if (max_depth <= 0) return glm::vec3(0);

	auto rec = hit_scene(ray, scene);
	if (rec) {
		auto target = rec->position + rec->normal;
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

void generate_image(const Config &cfg, const Camera &cam, const Scene &scene, char *img_out) {
	auto pixel_size_x = 1.f / cfg.width / cfg.samples_base;
	auto pixel_size_y = 1.f / cfg.height / cfg.samples_base;
	auto samples2 = static_cast<float>(cfg.samples_base * cfg.samples_base);

	for (auto y = 0; y < cfg.height; y++) {
		for (auto x = 0; x < cfg.width; x++) {
			auto color = glm::vec3();

			for (auto i = 0; i < cfg.samples_base * cfg.samples_base; ++i) {
				auto u = static_cast<float>(x) / cfg.width;
				auto v = static_cast<float>(y) / cfg.height;

				u += ((i % cfg.samples_base) + .5f) * pixel_size_x;
				v += ((i / cfg.samples_base) + .5f) * pixel_size_y;

				auto r = ray_from_camera(cam, u, v);
				color += ray_color(r, scene, cfg.max_depth);
			}

			auto c = color / samples2;
			bmp_set(img_out, x, cfg.height - y - 1, bmp_encode(c.r, c.g, c.b));
		}
	}
}

int main() {
	auto cfg = Config{};

	auto cam = Camera{
			.position = {0, 0, 0},
			.look_at = {0, 0, -1},

			.vfov = 90.0,
			.focal_length = 1.0,
	};
	init_camera(cam, cfg.width, cfg.height);

	Scene scene;
	scene.spheres.push_back(Sphere{.position = {0, 0, -3}, .radius = 1});
	scene.planes.push_back(Plane{.position = {0, -1, 0}, .normal = {0, 1, 0}});

	char bmp[BMP_SIZE(cfg.width, cfg.height)];
	bmp_init(bmp, cfg.width, cfg.height);

	auto start = std::chrono::high_resolution_clock::now();

	generate_image(cfg, cam, scene, bmp);

	auto finish = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
	std::cout << duration << "ms" << std::endl;

	auto f = fopen("test.bmp", "wb");
	fwrite(bmp, sizeof(bmp), 1, f);
	fclose(f);

	return 0;
}

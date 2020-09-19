#include <iostream>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "bmp.h"

#include "camera.h"
#include "ray.h"
#include "scene.h"

struct Config {
	int width = 540;
	int height = 540;

	int samples_base = 1;
};

void generate_image(const Config &cfg, const Camera &cam, const Scene &scene, char *img_out) {
	auto pixel_size_x = 1.f / cfg.width / cfg.samples_base;
	auto pixel_size_y = 1.f / cfg.height / cfg.samples_base;
	auto samples2 = static_cast<float>(cfg.samples_base * cfg.samples_base);

	for (auto y = 0; y < cfg.height; y++) {
		for (auto x = 0; x < cfg.width; x++) {
			auto color = glm::vec3();

			for (auto i = 0; i < samples2; ++i) {
				auto u = static_cast<float>(x) / cfg.width;
				auto v = static_cast<float>(y) / cfg.height;

				u += ((i % cfg.samples_base) + .5f) * pixel_size_x;
				v += ((i / cfg.samples_base) + .5f) * pixel_size_y;

				auto r = ray_from_camera(cam, u, v);
				color += ray_color(r, cam, scene);
			}

			color /= samples2;
			color = glm::clamp(color, 0.f, 1.f);
			bmp_set(img_out, x, cfg.height - y - 1, bmp_encode(color.r, color.g, color.b));
		}
	}
}

int main() {
	auto cfg = Config{};

	auto cam = Camera{
			.position = {0, 5, -16},
			.look_at = {0, 5, 0}, // forward: 0 0 1

			.vfov = 50.0,
			.focal_length = 1.f,
	};
	init_camera(cam, cfg.width, cfg.height);

	Scene scene;
	scene.ambient_light = glm::vec3(.8f);

	// floor
	make_rect(scene, make_lambert({1, 1, 1}),
			  {0, 0, 0},
			  {0, 1, 0},
			  {0, 0, 1},
			  {10, 10}
	);
	// back
	make_rect(scene, make_lambert({1, 1, 1}),
			  {0, 5, 5},
			  {0, 0, -1},
			  {0, 1, 0},
			  {10, 10}
	);
	// ceiling
	make_rect(scene, make_lambert({1, 1, 1}),
			  {0, 10, 0},
			  {0, -1, 0},
			  {0, 0, 1},
			  {10, 10}
	);
	// left
	make_rect(scene, make_lambert({1, 0, 0}),
			  {-5, 5, 0},
			  {1, 0, 0},
			  {0, 1, 0},
			  {10, 10}
	);
	// right
	make_rect(scene, make_lambert({0, 1, 0}),
			  {5, 5, 0},
			  {-1, 0, 0},
			  {0, 1, 0},
			  {10, 10}
	);

	auto box_material = Material{
			.type = MaterialType::kBlinnPhong,
			.color = {1, 1, 0},
			.blinnPhong = {
					.diffuse_intensity = 1.f,
					.specular_intensity = 1.f,
			},
	};
	// left
	make_box(scene, box_material,
			 {-2, 3, 2},
			 {3, 6, 3},
			 glm::rotate(glm::mat4(1.f), glm::radians(-25.f), {0, 1, 0})
	);
	// right
	make_box(scene, box_material,
			 {1.5, 1.5, -2},
			 {3, 3, 3},
			 glm::rotate(glm::mat4(1.f), glm::radians(20.f), {0, 1, 0})
	);

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

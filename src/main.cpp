#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"
#include "config.h"
#include "ray.h"
#include "scene.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

struct RenderingTask {
	const Config &cfg;
	const Camera &cam;
	const Scene &scene;

	Stats stats;
	char *pixel_buffer;

	std::mutex queue_mutex;
	std::queue<glm::ivec4> rectangles;
};

void generate_image_part(RenderingTask &task) {
	auto pixel_size_x = 1.f / task.cfg.width / task.cfg.samples_base;
	auto pixel_size_y = 1.f / task.cfg.height / task.cfg.samples_base;
	auto samples2 = static_cast<float>(task.cfg.samples_base * task.cfg.samples_base);

	while (true) {
		glm::ivec4 rect;

		{
			const std::lock_guard guard(task.queue_mutex);
			if (task.rectangles.empty()) break;
			rect = task.rectangles.front();
			task.rectangles.pop();
		}

		for (auto y = rect.y; y < rect.w; y++) {
			for (auto x = rect.x; x < rect.z; x++) {
				auto color = glm::vec3();

				for (auto i = 0; i < samples2; ++i) {
					auto u = static_cast<float>(x) / task.cfg.width;
					auto v = static_cast<float>(y) / task.cfg.height;

					u += ((i % task.cfg.samples_base) + .5f) * pixel_size_x;
					v += ((i / task.cfg.samples_base) + .5f) * pixel_size_y;

					auto r = ray_from_camera(task.cam, u, v);
					color += ray_color(r, task.cam, task.scene, task.cfg, task.stats, task.cfg.max_depth);
				}

				color /= samples2;
				color = glm::clamp(color, 0.f, 1.f);

				auto p = task.pixel_buffer + ((task.cfg.height - y - 1) * task.cfg.width + x) * 3;
				p[0] = static_cast<char>(255.99f * color.r);
				p[1] = static_cast<char>(255.99f * color.g);
				p[2] = static_cast<char>(255.99f * color.b);
			}
		}
	}
}

void generate_image(RenderingTask &task) {
	const auto cores = std::thread::hardware_concurrency();
	std::cout << "core num: " << cores << std::endl;

	float rect_width = 100;
	float rect_height = 100;
	for (float x = 0; x < static_cast<float>(task.cfg.width) / rect_width; ++x) {
		for (float y = 0; y < static_cast<float>(task.cfg.height) / rect_height; ++y) {
			glm::ivec4 rect(x * rect_width, y * rect_height, 0, 0);
			rect.z = glm::min(static_cast<float>(task.cfg.width), rect.x + rect_width);
			rect.w = glm::min(static_cast<float>(task.cfg.height), rect.y + rect_height);
			task.rectangles.push(rect);
		}
	}

	std::vector<std::thread> threads;

	for (auto i = 0; i < cores; ++i) {
		std::thread t([&task]() { generate_image_part(task); });
		threads.emplace_back(std::move(t));
	}

	for (auto &t : threads) t.join();
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

	// floor
	add_plane(scene, make_mat_lambert({1, 1, 1}), make_rect(
			{0, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
			{10, 10}
	));
	// back
	add_plane(scene, make_mat_lambert({1, 1, 1}), make_rect(
			{0, 5, 5},
			{0, 0, -1},
			{0, 1, 0},
			{10, 10}
	));
	// ceiling
	add_plane(scene, make_mat_lambert({1, 1, 1}), make_rect(
			{0, 10, 0},
			{0, -1, 0},
			{0, 0, 1},
			{10, 10}
	));
	// left
	add_plane(scene, make_mat_lambert({1, 0, 0}), make_rect(
			{-5, 5, 0},
			{1, 0, 0},
			{0, 1, 0},
			{10, 10}
	));
	// right
	add_plane(scene, make_mat_lambert({0, 1, 0}), make_rect(
			{5, 5, 0},
			{-1, 0, 0},
			{0, 1, 0},
			{10, 10}
	));

	auto box_material = Material{
			.type = MaterialType::kBlinnPhong,
			.color = {1, 1, 1},
			.blinnPhong = {
					.diffuse_intensity = 1.f,
					.specular_intensity = 1.f,
			},
	};
	// left
	add_planes(scene, box_material, make_box(
			{-2, 3, 2},
			{3, 6, 3},
			glm::rotate(glm::mat4(1.f), glm::radians(-25.f), {0, 1, 0})
	));
	// right
	add_planes(scene, box_material, make_box(
			{1.5, 1.5, -2},
			{3, 3, 3},
			glm::rotate(glm::mat4(1.f), glm::radians(20.f), {0, 1, 0})
	));

	// light
	auto area_light = AreaLight{
			.color = {1, 1, 1},
			.intensity = 1.f,
			.u_samples = 3,
			.v_samples = 3,
			.max_random_offset = .3f,
	};
	add_area_light(scene, area_light, make_rect(
			{0, 9.9999f, 0},
			{0, -1, 0},
			{0, 0, 1},
			{1, 1}
	));

	auto bmp_size = cfg.width * cfg.height * 3;
	auto pixel_buffer = new char[bmp_size];
	memset(pixel_buffer, 0, bmp_size);

	RenderingTask task{
			.cfg = cfg,
			.cam = cam,
			.scene = scene,

			.pixel_buffer = pixel_buffer,
	};

	auto start = std::chrono::high_resolution_clock::now();
	generate_image(task);
	auto finish = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
	std::cout << duration << "ms - " << (duration / 1000.f / 60.f) << "m" << std::endl;
	std::cout << "rays: " << task.stats.ray_count << std::endl;

	stbi_write_bmp("test.bmp", cfg.width, cfg.height, 3, pixel_buffer);
	delete[] pixel_buffer;

	return 0;
}

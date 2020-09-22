#pragma once

struct Config {
	int width = 540;
	int height = 540;

	int samples_base = 4;
	int max_depth = 5;

	int ambient_occlusion_samples = 5;
};

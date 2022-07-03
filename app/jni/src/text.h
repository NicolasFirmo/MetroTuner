#pragma once

#include "utility/vec.hpp"

struct Text {
	std::string_view text{};
	Vec2<int> position{};
	float size = 1.0F;
	SDL_Color color{0xff, 0xff, 0xff, 0xff};

	static bool init(int ptSize);
	static void shutdown();

	static int ptSize;
	static TTF_Font *baseFont;
};
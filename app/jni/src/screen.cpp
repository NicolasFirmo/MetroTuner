#include "screen.h"

SDL_DisplayMode Screen::displayMode{};
SDL_Window *Screen::window = nullptr;
SDL_Renderer *Screen::renderer = nullptr;

bool Screen::init() {
	if (int error = SDL_GetCurrentDisplayMode(0, &displayMode); error) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not get display mode (%d): %s\n", error,
					 SDL_GetError());
		shutdown();
		return false;
	}
	window = SDL_CreateWindow("SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  displayMode.w, displayMode.h, SDL_WINDOW_OPENGL);
	if (window == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		shutdown();
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", SDL_GetError());
		shutdown();
		return false;
	}
	if (int error = SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); error) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not set draw blend mode (%d): %s\n", error,
					 SDL_GetError());
		shutdown();
		return false;
	}

	return true;
}
void Screen::shutdown() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

bool Screen::setDrawColor(const SDL_Color &color) {
	const auto &[r, g, b, a] = color;
	if (int error = SDL_SetRenderDrawColor(renderer, r, g, b, a); error) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not set clear color (%d): %s\n", error,
					 SDL_GetError());
		shutdown();
		return false;
	}

	return true;
}

bool Screen::clear() {
	if (int error = SDL_RenderClear(renderer); error) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not clear screen (%d): %s\n", error,
					 SDL_GetError());
		shutdown();
		return false;
	}
	return true;
}

bool Screen::draw(const Text &text, TTF_Font *font) {
	auto solid = TTF_RenderText_Solid(font, text.str.data(), text.color);
	if (!solid) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not render text:%s\n", TTF_GetError());
		shutdown();
		return false;
	}
	auto texture = SDL_CreateTextureFromSurface(renderer, solid);
	if (!texture) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create text texture from surcafe:%s\n",
					 SDL_GetError());
		shutdown();
		return false;
	}
	const SDL_Rect dstRect{.x = text.position.x,
						   .y = text.position.y,
						   .w = int(solid->w * text.size),
						   .h = int(solid->h * text.size)};
	SDL_FreeSurface(solid);

	SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
	if (int error = SDL_RenderCopy(renderer, texture, nullptr, &dstRect); error) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not draw text (%d): %s\n", error,
					 SDL_GetError());
		shutdown();
		return false;
	}

	return true;
}

bool Screen::drawLine(int xPos1, int yPos1, int xPos2, int yPos2) {
	if (int error = SDL_RenderDrawLine(renderer, xPos1, yPos1, xPos2, yPos2); error) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not draw line (%d): %s\n", error,
					 SDL_GetError());
		shutdown();
		return false;
	}
	return true;
}

void Screen::show() {
	SDL_RenderPresent(renderer);
}
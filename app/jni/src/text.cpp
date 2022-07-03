#include "text.h"

int Text::ptSize{};
TTF_Font *Text::baseFont = nullptr;

bool Text::init(int ptSize) {
	if (TTF_Init() == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize TTF: %s\n", TTF_GetError());
		shutdown();
		return false;
	}

	baseFont = TTF_OpenFont("fonts/MesloLGS NF Regular.ttf", ptSize);
	if (!baseFont) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not load font:%s\n", TTF_GetError());
		shutdown();
		return false;
	}
	Text::ptSize = ptSize;

	return true;
}
void Text::shutdown() {
	TTF_CloseFont(baseFont);
	TTF_Quit();
}
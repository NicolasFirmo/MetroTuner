#include <SDL.h>
#include <SDL_image.h>
#include <android/log.h>

int main(int /*argc*/, char * /*argv*/[]) {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	const auto width = displayMode.w;
	const auto height = displayMode.h;

	SDL_Window *window = SDL_CreateWindow(
		"SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_OPENGL);

	if (window == nullptr) {
		__android_log_print(ANDROID_LOG_ERROR, "SDL",
							"Could not create window: %s", SDL_GetError());
		return 1;
	}

	SDL_Renderer *renderer =
		SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xff);

	SDL_RenderClear(renderer);

	SDL_Rect rect{.w = 500, .h = 500};
	rect.x = (width - rect.w) / 2;
	rect.y = (height - rect.h) / 2;

	SDL_Surface *loadedImage = IMG_Load("images/diapason.png");
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
	SDL_FreeSurface(loadedImage);

	SDL_RenderCopy(renderer, texture, nullptr, &rect);

	SDL_RenderPresent(renderer);

	const Uint32 timeToClose = 8000;
	SDL_Delay(timeToClose);

	SDL_DestroyWindow(window);

	SDL_Quit();
}
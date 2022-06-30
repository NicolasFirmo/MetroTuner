int main(int /*argc*/, char * /*argv*/[]) {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	const auto width = displayMode.w;
	const auto height = displayMode.h;

	SDL_Window *window = SDL_CreateWindow(
		"SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width,
		height, SDL_WINDOW_OPENGL);

	if (window == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Renderer *renderer =
		SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xff);

	SDL_RenderClear(renderer);

	SDL_Surface *loadedImage = IMG_Load("images/diapason.png");
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
	SDL_FreeSurface(loadedImage);

	if (TTF_Init() == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init: %s\n",
					 TTF_GetError());
		return 1;
	}

	TTF_Font *font = TTF_OpenFont("fonts/MesloLGS NF Regular.ttf", 64);
	if (!font) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load font:%s\n",
					 TTF_GetError());
		return 1;
	}

	SDL_Color textColor = {.r = 0xff, .g = 0xff, .b = 0xff, .a = 0xff};
	SDL_Surface *solid =
		TTF_RenderText_Solid(font, "MetroTuner Initialized", textColor);
	SDL_Rect topRect{.w = solid->w, .h = solid->h};

	SDL_Texture *solidTexture = SDL_CreateTextureFromSurface(renderer, solid);
	SDL_FreeSurface(solid);

	SDL_Rect centerRect{.w = 512, .h = 512};
	centerRect.x = (width - centerRect.w) / 2;
	centerRect.y = (height - centerRect.h) / 2;

	bool quit = false;
	SDL_Event event;
	while (!quit && SDL_WaitEvent(&event)) {
		SDL_RenderClear(renderer);
		switch (event.type) {
		case SDL_QUIT: {
			quit = true;
			break;
		}
		case SDL_FINGERDOWN: {
			SDL_SetTextureColorMod(texture, 0xff, 0xff, 0xff);
			break;
		}
		default: {
			SDL_SetTextureColorMod(texture, 0x00, 0x00, 0x00);
		}
		}
		SDL_RenderCopy(renderer, texture, nullptr, &centerRect);
		SDL_RenderCopy(renderer, solidTexture, nullptr, &topRect);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyWindow(window);

	SDL_Quit();
}
#include "app.h"

#include "text.h"

bool App::running = false;
SDL_DisplayMode App::displayMode{};
SDL_Window *App::window = nullptr;
SDL_Renderer *App::renderer = nullptr;

App::ExitCode App::init() {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GetCurrentDisplayMode(0, &displayMode);

	window = SDL_CreateWindow("SDL2 window", SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED, displayMode.w,
							  displayMode.h, SDL_WINDOW_OPENGL);

	if (window == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Could not create window: %s\n", SDL_GetError());
		return ExitCode::applicationError;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (TTF_Init() == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init: %s\n",
					 TTF_GetError());
		return ExitCode::applicationError;
	}

	return ExitCode::success;
}

App::ExitCode App::run() {
	running = true;

	setRendererDrawColor({.r = 0x20, .g = 0x20, .b = 0x20, .a = 0x00});

	TTF_Font *baseFont = TTF_OpenFont("fonts/MesloLGS NF Regular.ttf", 64);
	if (!baseFont) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load font:%s\n",
					 TTF_GetError());
		return ExitCode::applicationError;
	}

	std::vector<SDL_Color> colors(5);
	colors[0] = SDL_Color{.r = 0x00, .g = 0x00, .b = 0x00, .a = 0x00};
	colors[1] = SDL_Color{.r = 0xff, .g = 0x00, .b = 0x00, .a = 0x00};
	colors[2] = SDL_Color{.r = 0x00, .g = 0xff, .b = 0x00, .a = 0x00};
	colors[3] = SDL_Color{.r = 0x00, .g = 0x00, .b = 0xff, .a = 0x00};
	colors[4] = SDL_Color{.r = 0xff, .g = 0xff, .b = 0xff, .a = 0x00};
	size_t colorIndex = 0;

	SDL_Texture *icon = nullptr;
	SDL_Rect iconRect{};
	if (SDL_Surface *loadedImage = IMG_Load("images/diapason.png");
		loadedImage) {
		iconRect.w = loadedImage->w;
		iconRect.h = loadedImage->h;
		iconRect.x = (displayMode.w - iconRect.w) / 2;
		iconRect.y = (displayMode.h - iconRect.h) / 2;
		icon = SDL_CreateTextureFromSurface(renderer, loadedImage);
		SDL_FreeSurface(loadedImage);
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Couldn't load PNG image: %s", SDL_GetError());
		return ExitCode::applicationError;
	}
	SDL_SetTextureColorMod(icon, 0x00, 0x00, 0x00);

	Text greetings("MetroTuner Initialized");
	greetings.setPosition(0, 0);
	greetings.render(renderer, baseFont);

	SDL_Event event;
	int touchCount = 0;
	while (running && SDL_WaitEvent(&event)) {
		SDL_RenderClear(renderer);
		switch (event.type) {
		case SDL_QUIT: {
			running = false;
			break;
		}
		case SDL_FINGERDOWN: {
			++touchCount;
			break;
		}
		case SDL_FINGERUP: {
			--touchCount;
			greetings.setPosition(0, 0);
			break;
		}
		case SDL_FINGERMOTION: {
			const auto fingerXPos = event.tfinger.x;
			const auto fingerYPos = event.tfinger.y;
			greetings.setPosition((fingerXPos * displayMode.w) / 2,
								  (fingerYPos * displayMode.h) / 2);
			greetings.setColor({.r = Uint8(0xff * fingerXPos),
								.g = Uint8(0xff * fingerYPos),
								.b = 0xff,
								.a = 0xff});
			break;
		}
		}

		{
			const auto [r, g, b, a] =
				colors[touchCount < colors.size() ? touchCount
												  : colors.size() - 1];
			SDL_SetTextureColorMod(icon, r, g, b);
		}

		greetings.render(renderer, baseFont);

		SDL_RenderCopy(renderer, icon, nullptr, &iconRect);
		SDL_RenderCopy(renderer, greetings.getTexture(), nullptr,
					   greetings.getDstRectConstPointer());
		SDL_RenderPresent(renderer);
	}

	shutdown();

	return ExitCode::success;
}

void App::shutdown() {
	SDL_DestroyWindow(window);

	SDL_Quit();
}

void App::setRendererDrawColor(const SDL_Color& color) {
	const auto &[r, g, b, a] = color;
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}
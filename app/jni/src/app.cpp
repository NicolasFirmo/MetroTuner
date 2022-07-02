#include "app.h"

#include "text.h"

bool App::running = false;

SDL_DisplayMode App::displayMode{};
SDL_Window *App::window = nullptr;
SDL_Renderer *App::renderer = nullptr;

TTF_Font *App::baseFont = nullptr;

Microphone<float> App::microphone{};

App::ExitCode App::init() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

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

	baseFont = TTF_OpenFont("fonts/MesloLGS NF Regular.ttf", 64);
	if (!baseFont) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load font:%s\n",
					 TTF_GetError());
		return ExitCode::applicationError;
	}

	SDL_AudioSpec desired, obtained;
	SDL_zero(desired);
	desired.freq = 48000;
	desired.format = AUDIO_F32;
	desired.channels = 1;
	desired.samples = 4096;
	desired.callback = onAudioCapturing;

	if (microphone.init(desired, obtained) == 0) {
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio: %s",
					 SDL_GetError());
		return ExitCode::audioError;
	}
	if (obtained.channels != desired.channels) {
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "We didn't get a mono device.");
		return ExitCode::audioError;
	}
	if (obtained.freq != desired.freq) {
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "We didn't get a 48000Hz audio.");
		return ExitCode::audioError;
	}
	if (obtained.format != desired.format) {
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO,
					 "We didn't get Float32 audio format.");
		return ExitCode::audioError;
	}
	SDL_PauseAudioDevice(microphone.id(), 0); // start audio capturing.

	return ExitCode::success;
}

static float rms = 0;
App::ExitCode App::run() {
	running = true;

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

	Text greetings{"MetroTuner Initialized"};
	greetings.setPosition(0, 0);
	greetings.render(renderer, baseFont);

	auto makeMicRmsString = [](float rms) -> std::string {
		std::stringstream stringStream;
		stringStream << "Mic audio rms: " << rms;
		return stringStream.str();
	};

	Text micStatus{makeMicRmsString(rms).c_str()};
	micStatus.setPosition(0, greetings.getDstRectConstPointer()->h);
	micStatus.render(renderer, baseFont);

	int touchCount = 0;
	std::thread eventLoop{[&]() {
		SDL_Event event;
		while (running && SDL_WaitEvent(&event)) {
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
				break;
			}
			case SDL_FINGERMOTION: {
				break;
			}
			}
		}
	}};

	while (running) {
		setRendererDrawColor({.r = 0x20, .g = 0x20, .b = 0x20, .a = 0x00});
		SDL_RenderClear(renderer);

		{
			const auto [r, g, b, a] =
				colors[touchCount < colors.size() ? touchCount
												  : colors.size() - 1];
			SDL_SetTextureColorMod(icon, r, g, b);
		}

		microphone.startReading();
		{
			setRendererDrawColor({.r = 0x20, .g = 0xff, .b = 0x20, .a = 0x00});

			int lastSampleAmplitude =
				microphone.samples()[0] * 200 + displayMode.h - 800;
			int lastTCoord = 0;
			for (size_t i = 1; i < microphone.numOfSamples; i++) {
				const int sampleAmplitude =
					microphone.samples()[i] * 200 + displayMode.h - 800;
				const auto tCoord = displayMode.w * i / microphone.numOfSamples;
				SDL_RenderDrawLine(renderer, lastTCoord, lastSampleAmplitude,
								   tCoord, sampleAmplitude);
				lastSampleAmplitude = sampleAmplitude;
				lastTCoord = tCoord;
			}
		}
		microphone.finshReading();

		micStatus.setText(makeMicRmsString(rms).c_str());
		micStatus.render(renderer, baseFont);

		SDL_RenderCopy(renderer, icon, nullptr, &iconRect);
		SDL_RenderCopy(renderer, greetings.getTexture(), nullptr,
					   greetings.getDstRectConstPointer());
		SDL_RenderCopy(renderer, micStatus.getTexture(), nullptr,
					   micStatus.getDstRectConstPointer());
		SDL_RenderPresent(renderer);
	}

	eventLoop.join();

	SDL_DestroyTexture(icon);

	shutdown();
	return ExitCode::success;
}

void App::shutdown() {
	microphone.shutdown();
	TTF_CloseFont(baseFont);
	TTF_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void App::onAudioCapturing(void *userdata, Uint8 *stream, int len) {
	if (!microphone.tryToWrite())
		return;
	SDL_memcpy(microphone.samples().data(), stream, len);

	float squaredSum = 0;
	for (auto &&sample : microphone.samples())
		squaredSum += sample * sample;

	rms = std::sqrt(squaredSum / microphone.numOfSamples);

	microphone.finshWriting();
}

void App::setRendererDrawColor(const SDL_Color &color) {
	const auto &[r, g, b, a] = color;
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}
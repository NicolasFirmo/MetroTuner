#include "app.h"

#include "audio.h"
#include "screen.h"
#include "text.h"

#include "utility/float-to-uint8.h"

bool App::running = false;

App::ExitCode App::init() {
	if (int error = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); error) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize SDL (%d): %s\n", error,
					 SDL_GetError());
		shutdown();
		return ExitCode::applicationError;
	}

	if (!Screen::init()) {
		shutdown();
		return ExitCode::applicationError;
	}

	if (!Text::init(64)) {
		shutdown();
		return ExitCode::applicationError;
	}

	if (!Audio::init()) {
		shutdown();
		return ExitCode::audioError;
	}

	return ExitCode::success;
}

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
	if (SDL_Surface *loadedImage = IMG_Load("images/diapason.png"); loadedImage) {
		iconRect.w = loadedImage->w;
		iconRect.h = loadedImage->h;
		iconRect.x = (Screen::width() - iconRect.w) / 2;
		iconRect.y = (Screen::height() - iconRect.h) / 2;
		icon = SDL_CreateTextureFromSurface(Screen::getRenderer(), loadedImage);
		SDL_FreeSurface(loadedImage);
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load PNG image: %s", SDL_GetError());
		return ExitCode::applicationError;
	}
	SDL_SetTextureColorMod(icon, 0x00, 0x00, 0x00);

	auto makeFloatLogString = [](std::string_view text, float value, int precision = 3) -> std::string {
		std::stringstream stringStream;
		stringStream << text << ": " << std::fixed << std::setprecision(precision) << value;
		return stringStream.str();
	};

	Text greetings{"MetroTuner Initialized", {0, 0}};
	Text micStatus{"Mic audio rms", {0, Text::ptSize}};
	Text pitchLog{"Frequency (hz)", {0, micStatus.position.y + Text::ptSize}};

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
		Screen::setDrawColor({.r = 0x20, .g = 0x20, .b = 0x20, .a = 0xff});
		Screen::clear();

		{
			const auto [r, g, b, a] =
				colors[touchCount < colors.size() ? touchCount : colors.size() - 1];
			SDL_SetTextureColorMod(icon, r, g, b);
		}

		auto rms = Audio::microphone.rms();

		Audio::microphone.startReading();
		{
			Screen::draw(Audio::microphone.signalOnTime(), 200, Screen::height() * 1 / 4,
						 {.r = floatToUint8(std::max(.2F, rms * 5.0F)),
						  .g = floatToUint8(5.0F - rms * 10.0F),
						  .b = 0x40,
						  .a = 0xff});
		}
		Audio::microphone.finshReading();

		const float pitch = Audio::microphone.dominantFrequency();

		Screen::draw<Screen::ScaleMode::logarithmic>(Audio::microphone.signalOnFreq(), 1,
													 Screen::height() * 3 / 4,
													 {.r = 0x40, .g = 0x7f, .b = 0xff, .a = 0xff});

		Screen::draw(greetings);
		micStatus.str = makeFloatLogString("Mic audio rms", rms, 5);
		Screen::draw(micStatus);
		pitchLog.str = makeFloatLogString("Frequency (hz)", pitch, 2);
		Screen::draw(pitchLog);

		SDL_RenderCopy(Screen::getRenderer(), icon, nullptr, &iconRect);

		Screen::show();
	}
	eventLoop.join();

	SDL_DestroyTexture(icon);

	shutdown();
	return ExitCode::success;
}

void App::shutdown() {
	Audio::shutdown();
	Text::shutdown();
	Screen::shutdown();
	SDL_Quit();
}
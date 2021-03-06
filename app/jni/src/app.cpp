#include "app.h"

#include "audio.h"
#include "screen.h"
#include "text.h"

#include "notes.h"
#include "timer.h"

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

void App::shutdown() {
	Audio::shutdown();
	Text::shutdown();
	Screen::shutdown();
	SDL_Quit();
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

	Text greetings{.str = "MetroTuner Initialized", .position = {0, 0}};
	Text fpsLog{.position = {0, Text::ptSize}};
	Text micStatus{.position = {0, fpsLog.position.y + Text::ptSize}};
	Text pitchLog{.position = {0, micStatus.position.y + Text::ptSize}};
	Text noteLog{.position = {0, pitchLog.position.y + Text::ptSize}};

	std::thread eventLoop{onEvent};

	Timer timer;
	while (running) {
		const auto deltaT = timer.getCount();
		timer.startCounting();

		Screen::setDrawColor({.r = 0x20, .g = 0x20, .b = 0x20, .a = 0xff});
		Screen::clear();

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
		const Note note{pitch};

		Screen::draw<Screen::ScaleMode::logarithmic>(Audio::microphone.signalOnFreq(), 1,
													 Screen::height() * 3 / 4,
													 {.r = 0x40, .g = 0x7f, .b = 0xff, .a = 0xff});

		Screen::draw(greetings);
		micStatus.str = fmt::format("Mic audio rms: {:.5f}", rms);
		Screen::draw(micStatus);
		fpsLog.str = fmt::format("Frame rate: {:.2f}fps", 1.0f / deltaT);
		Screen::draw(fpsLog);
		pitchLog.str = fmt::format("Frequency: {:.2f}hz", pitch);
		Screen::draw(pitchLog);
		noteLog.str = fmt::format("Note {} {:.2f} cents", note.getSharpName(), note.getCents());
		Screen::draw(noteLog);

		SDL_RenderCopy(Screen::getRenderer(), icon, nullptr, &iconRect);

		Screen::show();
	}
	eventLoop.join();

	SDL_DestroyTexture(icon);

	shutdown();
	return ExitCode::success;
}

void App::onEvent() {
	SDL_Event event;
	while (running && SDL_WaitEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT: {
			running = false;
			break;
		}
		}
	}
}
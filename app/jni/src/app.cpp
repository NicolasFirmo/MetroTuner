#include "app.h"

#include "text.h"

#include "utility/float-to-uint8.h"

bool App::running = false;

SDL_DisplayMode App::displayMode{};
SDL_Window *App::window = nullptr;
SDL_Renderer *App::renderer = nullptr;

TTF_Font *App::baseFont = nullptr;

Microphone<float> App::microphone{};

App::ExitCode App::init() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	SDL_GetCurrentDisplayMode(0, &displayMode);

	window = SDL_CreateWindow("SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  displayMode.w, displayMode.h, SDL_WINDOW_OPENGL);

	if (window == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window: %s\n", SDL_GetError());
		return ExitCode::applicationError;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	if (TTF_Init() == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init: %s\n", TTF_GetError());
		return ExitCode::applicationError;
	}

	baseFont = TTF_OpenFont("fonts/MesloLGS NF Regular.ttf", 64);
	if (!baseFont) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load font:%s\n", TTF_GetError());
		return ExitCode::applicationError;
	}

	SDL_AudioSpec desired, obtained;
	SDL_zero(desired);
	desired.freq = microphone.sampleRate;
	desired.format = AUDIO_F32;
	desired.channels = 1;
	desired.samples = microphone.numOfSamples;
	desired.callback = onAudioCapturing;

	if (microphone.init(desired, obtained) == 0) {
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio: %s", SDL_GetError());
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
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "We didn't get Float32 audio format.");
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
	if (SDL_Surface *loadedImage = IMG_Load("images/diapason.png"); loadedImage) {
		iconRect.w = loadedImage->w;
		iconRect.h = loadedImage->h;
		iconRect.x = (displayMode.w - iconRect.w) / 2;
		iconRect.y = (displayMode.h - iconRect.h) / 2;
		icon = SDL_CreateTextureFromSurface(renderer, loadedImage);
		SDL_FreeSurface(loadedImage);
	} else {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load PNG image: %s", SDL_GetError());
		return ExitCode::applicationError;
	}
	SDL_SetTextureColorMod(icon, 0x00, 0x00, 0x00);

	auto makeFloatLogString = [](std::string_view text, float rms) -> std::string {
		std::stringstream stringStream;
		stringStream << text << ": " << rms;
		return stringStream.str();
	};

	Text greetings{"MetroTuner Initialized"};
	greetings.setPosition(0, 0);
	greetings.render(renderer, baseFont);

	Text micStatus{makeFloatLogString("Mic audio rms", rms).c_str()};
	micStatus.setPosition(0, greetings.getDstRectConstPointer()->h);
	micStatus.render(renderer, baseFont);

	Text pitchLog{makeFloatLogString("Frequency (hz)", 0).c_str()};
	pitchLog.setPosition(0, micStatus.getDstRectConstPointer()->y +
								micStatus.getDstRectConstPointer()->h);
	pitchLog.render(renderer, baseFont);

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

	fftwf_complex *freqSignal = nullptr;
	freqSignal =
		(fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * (microphone.numOfSamples / 2 + 1));
	fftwf_plan plan = nullptr;
	plan = fftwf_plan_dft_r2c_1d(microphone.numOfSamples, microphone.samples().data(), freqSignal,
								 FFTW_ESTIMATE);

	std::vector<float> hannWindow(microphone.numOfSamples);
	std::generate(hannWindow.begin(), hannWindow.end(), [i = 0]() mutable {
		const float hannBid =
			0.5f * (1.0f - cos(float(2 * M_PI) * (i++ + 1) / (microphone.numOfSamples + 1)));
		return hannBid;
	});
	while (running) {
		setRendererDrawColor({.r = 0x20, .g = 0x20, .b = 0x20, .a = 0xff});
		SDL_RenderClear(renderer);

		{
			const auto [r, g, b, a] =
				colors[touchCount < colors.size() ? touchCount : colors.size() - 1];
			SDL_SetTextureColorMod(icon, r, g, b);
		}

		SDL_Color timeSignalColor{.r = floatToUint8(std::max(.2f, rms * 5.0f)),
								  .g = floatToUint8(5.0f - rms * 10.0f),
								  .b = 0x40,
								  .a = 0xff};
		microphone.startReading();
		{
			std::transform(hannWindow.begin(), hannWindow.end(), microphone.samples().begin(),
						   microphone.samples().begin(), std::multiplies{});

			renderSignal(std::span{microphone.samples()}, 200, displayMode.h - 800,
						 timeSignalColor);
			fftwf_execute(plan);
		}
		microphone.finshReading();

		const float pitch =
			getDominantFrequency(std::span{freqSignal, microphone.numOfSamples / 2 + 1});

		renderSignal<fftwf_complex, microphone.numOfSamples / 2 + 1, true>(
			std::span<fftwf_complex, microphone.numOfSamples / 2 + 1>{
				freqSignal, microphone.numOfSamples / 2 + 1},
			1, 800, {.r = 0x40, .g = 0x7f, .b = 0xff, .a = 0xff});

		micStatus.setText(makeFloatLogString("Mic audio rms", rms).c_str());
		micStatus.render(renderer, baseFont);

		pitchLog.setText(makeFloatLogString("Frequency (hz)", pitch).c_str());
		pitchLog.render(renderer, baseFont);

		SDL_RenderCopy(renderer, icon, nullptr, &iconRect);
		SDL_RenderCopy(renderer, greetings.getTexture(), nullptr,
					   greetings.getDstRectConstPointer());
		SDL_RenderCopy(renderer, micStatus.getTexture(), nullptr,
					   micStatus.getDstRectConstPointer());
		SDL_RenderCopy(renderer, pitchLog.getTexture(), nullptr, pitchLog.getDstRectConstPointer());
		SDL_RenderPresent(renderer);
	}
	fftwf_destroy_plan(plan);
	fftwf_free(freqSignal);
	fftwf_cleanup();
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

template <typename SignalType, size_t Len, bool Log>
requires std::is_same_v<SignalType, float> || std::is_same_v<SignalType, fftwf_complex>
void App::renderSignal(std::span<SignalType, Len> signal, const int amplitudeHeight, const int yPos,
					   const SDL_Color &color) {
	setRendererDrawColor(color);

	int lastSampleAmplitude;
	if constexpr (std::is_same_v<SignalType, fftwf_complex>)
		lastSampleAmplitude = signal[0][0] * amplitudeHeight + yPos;
	else
		lastSampleAmplitude = signal[0] * amplitudeHeight + yPos;

	int lastTCoord = 0;
	const size_t numOfSamples = signal.size();
	const auto logDenominator = std::log(float(numOfSamples));
	for (size_t i = 1; i < numOfSamples; i++) {
		int sampleAmplitude;
		if constexpr (std::is_same_v<SignalType, fftwf_complex>)
			sampleAmplitude = signal[i][0] * amplitudeHeight + yPos;
		else
			sampleAmplitude = signal[i] * amplitudeHeight + yPos;

		int tCoord;
		if constexpr (Log) {
			tCoord = displayMode.w * std::log(float(i)) / logDenominator;
		} else {
			tCoord = displayMode.w * i / numOfSamples;
		}

		SDL_RenderDrawLine(renderer, lastTCoord, lastSampleAmplitude, tCoord, sampleAmplitude);

		lastSampleAmplitude = sampleAmplitude;
		lastTCoord = tCoord;
	}
}

float App::getDominantFrequency(std::span<fftwf_complex> signal) {
	size_t dominantBid = 0;
	float maxAmplitude2 = 0;
	for (size_t i = 0; i < signal.size(); i++) {
		const float freqAmplitude2 = signal[i][0] * signal[i][0];
		if (freqAmplitude2 > maxAmplitude2) {
			maxAmplitude2 = freqAmplitude2;
			dominantBid = i;
		}
	}
	return (float)microphone.sampleRate * dominantBid / signal.size() / 2;
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
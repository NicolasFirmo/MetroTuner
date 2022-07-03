#pragma once

#include "microphone.hpp"

#include "utility/fftwf-complex-signal.h"

class App {
public:
	enum class ExitCode { success = 0, applicationError, audioError };

	static ExitCode init();
	static ExitCode run();
	static void shutdown();

private:
	static void onAudioCapturing(void *userdata, Uint8 *stream, int len);

	static void setRendererDrawColor(const SDL_Color &color);
	enum class ScaleMode { linear = 0, logarithmic };
	template <ScaleMode Scale = ScaleMode::linear, class Container>
	static void renderSignal(const Container &signal, int amplitudeHeight, int yPos,
							 const SDL_Color &color);
	static float getDominantFrequency(FftwfComplexSignal signal);

	static bool running;

	static SDL_DisplayMode displayMode;
	static SDL_Window *window;
	static SDL_Renderer *renderer;

	static TTF_Font *baseFont;

	static Microphone<float> microphone;
};
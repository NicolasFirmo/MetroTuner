#pragma once

#include "microphone.hpp"

class App {
public:
	enum class ExitCode { success = 0, applicationError, audioError };

	static ExitCode init();
	static ExitCode run();
	static void shutdown();

private:
	static void onAudioCapturing(void *userdata, Uint8 *stream, int len);

	static void setRendererDrawColor(const SDL_Color &color);
	template <typename SignalType, size_t Len, bool Log = false>
	requires std::is_same_v<SignalType, float> || std::is_same_v<SignalType, fftwf_complex>
	static void renderSignal(std::span<SignalType, Len> signal, int amplitudeHeight, int yPos,
							 const SDL_Color &color);
	static float getDominantFrequency(std::span<fftwf_complex> signal);

	static bool running;

	static SDL_DisplayMode displayMode;
	static SDL_Window *window;
	static SDL_Renderer *renderer;

	static TTF_Font *baseFont;

	static Microphone<float> microphone;
};
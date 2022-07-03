#pragma once

#include "text.h"

struct Screen {
	static bool init();
	static void shutdown();

	static auto width() { return displayMode.w; }
	static auto height() { return displayMode.h; }
	static auto refreshRate() { return displayMode.refresh_rate; }

	static bool setDrawColor(const SDL_Color &color);
	static bool clear();

	static bool draw(const Text &text, TTF_Font *font = Text::baseFont);
	enum class ScaleMode { linear = 0, logarithmic };
	template <ScaleMode Scale = ScaleMode::linear, class Container>
	static bool draw(const Container &signal, int amplitudeHeight, int yPos,
					 const SDL_Color &color);

	static bool drawLine(int xPos1, int yPos1, int xPos2, int yPos2);

	static void show();

	// testing only
	static auto getRenderer() { return renderer; }

	static SDL_DisplayMode displayMode;
	static SDL_Window *window;
	static SDL_Renderer *renderer;
};

template <Screen::ScaleMode Scale, class Container>
bool Screen::draw(const Container &signal, const int amplitudeHeight, const int yPos,
				  const SDL_Color &color) {
	setDrawColor(color);

	int lastSampleAmplitude;
	if constexpr (std::is_same_v<typename Container::value_type, fftwf_complex>)
		lastSampleAmplitude = signal[0][0] * amplitudeHeight + yPos;
	else
		lastSampleAmplitude = signal[0] * amplitudeHeight + yPos;

	int lastTCoord = 0;
	const size_t numOfSamples = signal.size();
	const auto logDenominator = std::log(float(numOfSamples));
	for (size_t i = 1; i < numOfSamples; i++) {
		int sampleAmplitude;
		if constexpr (std::is_same_v<typename Container::value_type, fftwf_complex>)
			sampleAmplitude = signal[i][0] * amplitudeHeight + yPos;
		else
			sampleAmplitude = signal[i] * amplitudeHeight + yPos;

		int tCoord;
		if constexpr (Scale == ScaleMode::logarithmic) {
			tCoord = displayMode.w * std::log(float(i)) / logDenominator;
		} else {
			tCoord = displayMode.w * i / numOfSamples;
		}

		if (!drawLine(lastTCoord, lastSampleAmplitude, tCoord, sampleAmplitude))
			return false;

		lastSampleAmplitude = sampleAmplitude;
		lastTCoord = tCoord;
	}

	return true;
}
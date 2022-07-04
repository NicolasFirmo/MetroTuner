#pragma once

#include "utility/pointer-view.h"

class Microphone {
public:
	SDL_AudioDeviceID init(int numOfSamples, int sampleRate);
	void startCapturing() const;
	void shutdown() const;

	float dominantFrequency(size_t resolution = 9);
	float rms();

	[[nodiscard]] auto id() const { return id_; }
	[[nodiscard]] auto sampleRate() const { return specs_.freq; }
	[[nodiscard]] auto numOfChannels() const { return specs_.channels; }
	[[nodiscard]] auto numOfSamples() const { return specs_.samples; }

	[[nodiscard]] auto &signalOnTime() { return signalOnTime_; }
	[[nodiscard]] auto &signalOnFreq() { return signalOnFreq_; }

	void startReading();
	void finshReading();

	static void onCapturing(void *userdata, Uint8 *stream, int len);

private:
	bool tryToWrite();
	void finshWriting();

	SDL_AudioDeviceID id_ = 0;
	SDL_AudioSpec specs_{};

	std::vector<float> signalOnTime_{};
	std::vector<float> hannWindow_{};
	PointerView<fftwf_complex> signalOnFreq_{};
	fftwf_plan fftwfPlan_ = nullptr;

	std::condition_variable conditionVariable_{};
	std::mutex mutex_{};
	int numOfReadingThreads_ = 0;
	bool wasRead_ = true;
};
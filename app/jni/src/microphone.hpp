#pragma once

template <typename DataType>
class Microphone {
public:
	using dataType = DataType;

	SDL_AudioDeviceID init(int numOfSamples, int sampleRate, SDL_AudioCallback callBackFn);
	void startCapturing() const;
	void shutdown() const;

	[[nodiscard]] auto id() const { return id_; }
	[[nodiscard]] auto sampleRate() const { return specs_.freq; }
	[[nodiscard]] auto numOfChannels() const { return specs_.channels; }
	[[nodiscard]] auto numOfSamples() const { return specs_.samples; }

	[[nodiscard]] auto &samples() { return samples_; }

	void startReading();
	void finshReading();

	bool tryToWrite();
	void finshWriting();

private:
	SDL_AudioDeviceID id_ = 0;
	SDL_AudioSpec specs_{};

	std::vector<DataType> samples_{};

	std::condition_variable conditionVariable_{};
	std::mutex mutex_{};
	int numOfReadingThreads_ = 0;
	bool wasRead_ = true;
};

template <typename DataType>
SDL_AudioDeviceID Microphone<DataType>::init(int numOfSamples, int sampleRate,
											 SDL_AudioCallback callBackFn) {
	static_assert(std::is_same_v<DataType, float>, "Only float DataType supported for now!");

	samples_.resize(numOfSamples);

	SDL_AudioSpec desired;
	SDL_zero(desired);
	desired.freq = sampleRate;
	if constexpr (std::is_same_v<DataType, float>)
		desired.format = AUDIO_F32;
	desired.channels = 1;
	desired.samples = numOfSamples;
	desired.callback = callBackFn;

	return id_ = SDL_OpenAudioDevice(nullptr, 1, &desired, &specs_, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
}
template <typename DataType>
void Microphone<DataType>::startCapturing() const {
	SDL_PauseAudioDevice(id_, 0);
}
template <typename DataType>
void Microphone<DataType>::shutdown() const {
	SDL_CloseAudioDevice(id_);
}

template <typename DataType>
void Microphone<DataType>::startReading() {
	std::unique_lock lock(mutex_);
	conditionVariable_.wait(lock, [this] { return !wasRead_; });
	++numOfReadingThreads_;
}
template <typename DataType>
void Microphone<DataType>::finshReading() {
	--numOfReadingThreads_;
	wasRead_ = numOfReadingThreads_ == 0;
}

template <typename DataType>
bool Microphone<DataType>::tryToWrite() {
	if (!wasRead_)
		return false;
	mutex_.lock();
	wasRead_ = false;
	return true;
}
template <typename DataType>
void Microphone<DataType>::finshWriting() {
	mutex_.unlock();
	conditionVariable_.notify_all();
}
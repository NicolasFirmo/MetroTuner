#pragma once

template <typename DataType>
class Microphone {
public:
	using dataType = DataType;

	static constexpr int numOfSamples = 1024*16;
	static constexpr int sampleRate = 48000;

	SDL_AudioDeviceID init(const SDL_AudioSpec &desired,
						   SDL_AudioSpec &obtained) {
		samples_.resize(numOfSamples);
		id_ = SDL_OpenAudioDevice(nullptr, 1, &desired, &obtained,
								  SDL_AUDIO_ALLOW_FORMAT_CHANGE);
		return id_;
	}
	void shutdown() { SDL_CloseAudioDevice(id_); }

	[[nodiscard]] auto id() const { return id_; }
	[[nodiscard]] auto &samples() { return samples_; }

	void startReading() {
		std::unique_lock lock(mutex_);
		conditionVariable_.wait(lock, [this] { return !wasRead_; });
		++numOfReadingThreads_;
	}
	void finshReading() {
		--numOfReadingThreads_;
		wasRead_ = !isBeingRead();
	}
	bool isBeingRead() { return numOfReadingThreads_ > 0; }

	bool tryToWrite() {
		if (!wasRead_)
			return false;
		mutex_.lock();
		wasRead_ = false;
		return true;
	}
	void finshWriting() {
		mutex_.unlock();
		conditionVariable_.notify_all();
	}

private:
	SDL_AudioDeviceID id_ = 0;
	std::vector<DataType> samples_{};

	std::condition_variable conditionVariable_{};
	std::mutex mutex_{};
	int numOfReadingThreads_ = 0;
	bool wasRead_ = true;
};
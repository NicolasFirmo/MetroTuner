#include "microphone.h"

SDL_AudioDeviceID Microphone::init(int numOfSamples, int sampleRate) {
	signalOnTime_.resize(numOfSamples);
	hannWindow_.resize(numOfSamples);
	std::generate(hannWindow_.begin(), hannWindow_.end(), [i = 0, numOfSamples]() mutable {
		return 0.5F * (1.0F - cos(float(2 * M_PI) * (i++ + 1) / (numOfSamples + 1)));
	});

	signalOnFreq_.ptr =
		static_cast<fftwf_complex *>(fftwf_malloc(sizeof(fftwf_complex) * (numOfSamples / 2 + 1)));
	signalOnFreq_.len = size_t(numOfSamples / 2 + 1);
	fftwfPlan_ = fftwf_plan_dft_r2c_1d(numOfSamples, signalOnTime_.data(), signalOnFreq_.data(),
									   FFTW_ESTIMATE);

	SDL_AudioSpec desired;
	SDL_zero(desired);
	desired.freq = sampleRate;
	desired.format = AUDIO_F32;
	desired.channels = 1;
	desired.samples = numOfSamples;
	desired.userdata = this;
	desired.callback = onCapturing;

	return id_ = SDL_OpenAudioDevice(nullptr, 1, &desired, &specs_, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
}
void Microphone::startCapturing() const {
	SDL_PauseAudioDevice(id_, 0);
}
void Microphone::shutdown() const {
	fftwf_free(signalOnFreq_.ptr);
	fftwf_destroy_plan(fftwfPlan_);
	fftwf_cleanup();
	SDL_CloseAudioDevice(id_);
}

float Microphone::dominantFrequency(const size_t resolution) {
	size_t dominantBid = 0;
	float maxAmplitude2 = 0;
	for (size_t i = 0; i < signalOnFreq_.size(); i++) {
		const float freqAmplitude2 = signalOnFreq_[i][0] * signalOnFreq_[i][0];
		if (freqAmplitude2 > maxAmplitude2) {
			maxAmplitude2 = freqAmplitude2;
			dominantBid = i;
		}
	}

	float dominantFrequency = .0F;
	float amplitudeSum = .0F;
	for (size_t i = dominantBid - resolution / 2; i < dominantBid - resolution / 2 + resolution;
		 i++) {
		if (i > signalOnFreq_.size())
			continue;

		amplitudeSum += signalOnFreq_[i][0] * signalOnFreq_[i][0];
		dominantFrequency += i * signalOnFreq_[i][0] * signalOnFreq_[i][0];
	}
	dominantFrequency /= amplitudeSum;

	return (float)sampleRate() / 2 * dominantFrequency / (signalOnFreq_.size() - 1);
}
float Microphone::rms() {
	startReading();
	float squaredSum = 0;
	for (auto &&sample : signalOnTime_)
		squaredSum += sample * sample;
	finshReading();
	return std::sqrt(squaredSum / numOfSamples());
}

void Microphone::startReading() {
	std::unique_lock lock(mutex_);
	conditionVariable_.wait(lock, [this] { return !wasRead_; });
	++numOfReadingThreads_;
}
void Microphone::finshReading() {
	--numOfReadingThreads_;
	wasRead_ = numOfReadingThreads_ == 0;
}

bool Microphone::tryToWrite() {
	if (!wasRead_)
		return false;
	mutex_.lock();
	wasRead_ = false;
	return true;
}
void Microphone::finshWriting() {
	mutex_.unlock();
	conditionVariable_.notify_all();
}

void Microphone::onCapturing(void *userdata, Uint8 *stream, int len) {
	Microphone &microphone = *static_cast<Microphone *>(userdata);

	if (!microphone.tryToWrite())
		return;

	auto &signalOnTime = microphone.signalOnTime_;
	auto &hannWindow = microphone.hannWindow_;
	auto &fftwfPlan = microphone.fftwfPlan_;

	SDL_memcpy(signalOnTime.data(), stream, len);

	std::transform(hannWindow.begin(), hannWindow.end(), signalOnTime.begin(), signalOnTime.begin(),
				   std::multiplies{});
	fftwf_execute(fftwfPlan);

	microphone.finshWriting();
}
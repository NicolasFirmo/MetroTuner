#include "audio.h"

Microphone Audio::microphone;

bool Audio::init() {
	if (!microphone.init(1024 * 4, 48000)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not open microphone: %s", SDL_GetError());
		shutdown();
		return false;
	}
	if (microphone.numOfChannels() != 1) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Only mono sound capturing is supported!");
		shutdown();
		return false;
	}
	microphone.startCapturing();

	return true;
}
void Audio::shutdown() {
	microphone.shutdown();
}
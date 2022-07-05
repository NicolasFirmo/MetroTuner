#pragma once

class Timer {
public:
	void startCounting();
	[[nodiscard]] double getCount() const;

private:
	uint64_t startTime_ = SDL_GetPerformanceCounter();
};
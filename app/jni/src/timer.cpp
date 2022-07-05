#include "timer.h"

void Timer::startCounting() {
	startTime_ = SDL_GetPerformanceCounter();
}
double Timer::getCount() const {
	return static_cast<double>((SDL_GetPerformanceCounter() - startTime_) /
								 static_cast<double>(SDL_GetPerformanceFrequency()));
}
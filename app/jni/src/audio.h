#pragma once

#include "microphone.h"

struct Audio {
	static bool init();
	static void shutdown();

	static Microphone microphone;
};
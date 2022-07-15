#pragma once

class App {
public:
	enum class ExitCode { success = 0, applicationError, audioError };

	static ExitCode init();
	static ExitCode run();
	static void shutdown();

private:
	static void onEvent();

	static bool running;
};
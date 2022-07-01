#pragma once

class App {
public:
	enum class ExitCode { success = 0, applicationError };

	static ExitCode init();
	static ExitCode run();
	static void shutdown();

private:
	static void setRendererDrawColor(const SDL_Color &color);

	static bool running;
	static SDL_DisplayMode displayMode;
	static SDL_Window *window;
	static SDL_Renderer *renderer;
};
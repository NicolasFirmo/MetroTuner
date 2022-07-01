#include "app.h"

int main(int /*argc*/, char * /*argv*/[]) {
	if (const auto exitCode = App::init(); exitCode != App::ExitCode::success)
		return static_cast<int>(exitCode);

	const auto exitCode = App::run();
	return static_cast<int>(exitCode);
}
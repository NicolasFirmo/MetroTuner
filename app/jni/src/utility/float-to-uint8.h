#pragma once

[[nodiscard]] constexpr Uint8 floatToUint8(float normalized) {
	return 0xff * std::clamp(normalized, 0.0f, 1.0f);
}
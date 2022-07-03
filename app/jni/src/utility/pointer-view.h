#pragma once

template <class ValueType>
struct PointerView {
	using value_type = ValueType;

	ValueType *ptr = nullptr;
	size_t len{};

	[[nodiscard]] auto data() { return ptr; }
	[[nodiscard]] auto size() const { return len; }

	[[nodiscard]] auto &operator[](size_t index) { return ptr[index]; }
	[[nodiscard]] auto &operator[](size_t index) const { return ptr[index]; }
};
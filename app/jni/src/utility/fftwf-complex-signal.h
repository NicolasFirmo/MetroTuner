#pragma once

class FftwfComplexSignal {
public:
	using value_type = fftwf_complex;

	FftwfComplexSignal(size_t len);
	~FftwfComplexSignal();

	FftwfComplexSignal(const FftwfComplexSignal &other);
	FftwfComplexSignal(FftwfComplexSignal &&other) noexcept = default;
	FftwfComplexSignal &operator=(const FftwfComplexSignal &other);
	FftwfComplexSignal &operator=(FftwfComplexSignal &&other) noexcept;

	[[nodiscard]] auto *data() { return data_; }
	[[nodiscard]] auto size() const { return len_; }

	[[nodiscard]] auto &operator[](size_t index) { return data_[index]; }
	[[nodiscard]] auto &operator[](size_t index) const { return data_[index]; }

private:
	fftwf_complex *data_ = nullptr;
	size_t len_{};
};
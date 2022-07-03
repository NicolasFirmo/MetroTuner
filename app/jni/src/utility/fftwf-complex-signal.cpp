#include "fftwf-complex-signal.h"

FftwfComplexSignal::FftwfComplexSignal(size_t len)
	: len_(len),
	  data_((fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * len)) {}
FftwfComplexSignal::~FftwfComplexSignal() {
	if (data_)
		fftwf_free(data_);
}

FftwfComplexSignal::FftwfComplexSignal(const FftwfComplexSignal &other)
	: len_(other.len_),
	  data_((fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * other.len_)) {
	SDL_memcpy(data_, other.data_, sizeof(fftwf_complex) * other.len_);
}
FftwfComplexSignal &FftwfComplexSignal::operator=(const FftwfComplexSignal &other) {
	if (this == &other)
		return *this;

	fftwf_free(data_);

	len_ = other.len_;
	data_ = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * other.len_);
	SDL_memcpy(data_, other.data_, sizeof(fftwf_complex) * other.len_);
}
FftwfComplexSignal &FftwfComplexSignal::operator=(FftwfComplexSignal &&other) noexcept {
	if (this == &other)
		return *this;

	fftwf_free(data_);

	len_ = other.len_;
	data_ = other.data_;
	other.data_ = nullptr;
}

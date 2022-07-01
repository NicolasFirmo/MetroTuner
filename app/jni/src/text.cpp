#include "text.h"

Text::Text(const char *text, const SDL_Color &textColor)
	: text_(text), textColor_(textColor) {}
Text::~Text() {
	SDL_DestroyTexture(texture_);
}

Text::Text(const Text &other)
	: text_(other.text_), texture_(nullptr), dstRect_(other.dstRect_),
	  textColor_(other.textColor_) {}
Text::Text(Text &&other) noexcept
	: text_(std::move(other.text_)), texture_(std::move(other.texture_)),
	  dstRect_(std::move(other.dstRect_)),
	  textColor_(std::move(other.textColor_)) {}
Text &Text::operator=(const Text &other) {
	text_ = other.text_;
	dstRect_ = other.dstRect_;
	textColor_ = other.textColor_;
}
Text &Text::operator=(Text &&other) noexcept {
	text_ = std::move(other.text_);
	texture_ = std::move(other.texture_);
	dstRect_ = std::move(other.dstRect_);
	textColor_ = std::move(other.textColor_);
}

void Text::setText(const char *text) {
	text_ = text;
}

void Text::setPosition(int xPos, int yPos) {
	dstRect_.x = xPos;
	dstRect_.y = yPos;
}

void Text::setColor(const SDL_Color &textColor) {
	textColor_ = textColor;
}

void Text::render(SDL_Renderer *renderer, TTF_Font *font) {
	SDL_Surface *solid = TTF_RenderText_Solid(font, text_.c_str(), textColor_);
	dstRect_.w = solid->w;
	dstRect_.h = solid->h;
	texture_ = SDL_CreateTextureFromSurface(renderer, solid);
	SDL_FreeSurface(solid);
}

SDL_Texture *Text::getTexture() {
	return texture_;
}
const SDL_Rect *Text::getDstRectConstPointer() const {
	return &dstRect_;
}
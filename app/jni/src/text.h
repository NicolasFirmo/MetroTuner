#pragma once

class Text {
public:
	explicit Text(const char *text, const SDL_Color &textColor = {0xff, 0xff, 0xff, 0xff});
	~Text();

	Text(const Text &);
	Text(Text &&) noexcept;
	Text &operator=(const Text &);
	Text &operator=(Text &&) noexcept;

	void setText(const char *text);
	void setPosition(int xPos, int yPos);
	void setColor(const SDL_Color &textColor);

	void render(SDL_Renderer *renderer, TTF_Font *font);

	[[nodiscard]] SDL_Texture *getTexture();
	[[nodiscard]] const SDL_Rect *getDstRectConstPointer() const;

private:
	std::string text_;

	SDL_Texture *texture_ = nullptr;
	SDL_Rect dstRect_{};
	SDL_Color textColor_{};
};
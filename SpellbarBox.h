#ifndef SPELLBAR_BOX_H
#define SPELLBAR_BOX_H

#include <SDL.h>
#include <vector>
#include <iostream>

#include "Sprite.h"

class SpellbarBox
{
	private:
		Sprite boxSprite, spellSprite;
		SDL_Renderer* renderer;
		Uint64 currTime, lastTime;
		int cooldownTime;

	public:
		bool onCooldown;
		std::vector<SDL_Texture*>* cooldownNumberSprites;

		SpellbarBox(SDL_Renderer* argRenderer, const std::string& boxSpritePath, const std::string& spellSpritePath, int boxX, int boxY, int boxWidth, int boxHeight, int argCooldownTime, std::vector<SDL_Texture*>* argCooldownNumberSprites)
		{
			renderer = argRenderer;

			SDL_Surface* surface = SDL_LoadBMP(boxSpritePath.c_str());
			if (surface == NULL) {
				std::cout << "ERROR: SpellbarBox: boxSprite: couldn't load surface" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			if (texture == NULL) {
				std::cout << "ERROR: SpellbarBox: boxSprite: couldn't create texture" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			boxSprite.texture = texture;

			surface = SDL_LoadBMP(spellSpritePath.c_str());
			if (surface == NULL) {
				std::cout << "ERROR: SpellbarBox: spellSprite: couldn't load surface" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			texture = SDL_CreateTextureFromSurface(renderer, surface);
			if (texture == NULL) {
				std::cout << "ERROR: SpellbarBox: boxSprite: couldn't create texture" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			spellSprite.texture = texture;

			boxSprite.rect.x = boxX;
			boxSprite.rect.y = boxY;
			boxSprite.rect.w = boxWidth;
			boxSprite.rect.h = boxHeight;

			spellSprite.rect.x = boxX + 10;
			spellSprite.rect.y = boxY + 10;
			spellSprite.rect.w = boxWidth - 17;
			spellSprite.rect.h = boxHeight - 17;

			currTime = lastTime = 0;

			cooldownTime = argCooldownTime;

			onCooldown = false;

			cooldownNumberSprites = argCooldownNumberSprites;
		}

		void startCooldown(void)
		{
			onCooldown = true;
			lastTime = SDL_GetTicks64();
		}

		void Draw(void)
		{
			SDL_RenderCopy(renderer, boxSprite.texture, NULL, &boxSprite.rect);
			SDL_RenderCopy(renderer, spellSprite.texture, NULL, &spellSprite.rect);

			if (onCooldown) {
				currTime = SDL_GetTicks64();

				int timeElapsed = currTime - lastTime;
				int timeRemaining = 0;

				if (timeElapsed < cooldownTime) {
					SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
					SDL_RenderDrawRect(renderer, &(spellSprite.rect));
					SDL_RenderFillRect(renderer, &(spellSprite.rect));

					timeRemaining = cooldownTime - timeElapsed;

					if (timeRemaining >= 4000 && timeRemaining <= 5000) {
						SDL_RenderCopy(renderer, (*cooldownNumberSprites)[4], NULL, &(spellSprite.rect));
					}
					else if (timeRemaining >= 3000 && timeRemaining < 4000) {
						SDL_RenderCopy(renderer, (*cooldownNumberSprites)[3], NULL, &(spellSprite.rect));
					}
					else if (timeRemaining >= 2000 && timeRemaining < 3000) {
						SDL_RenderCopy(renderer, (*cooldownNumberSprites)[2], NULL, &(spellSprite.rect));
					}
					else if (timeRemaining >= 1000 && timeRemaining < 2000) {
						SDL_RenderCopy(renderer, (*cooldownNumberSprites)[1], NULL, &(spellSprite.rect));
					}
					else if (timeRemaining >= 0 && timeRemaining < 1000) {
						SDL_RenderCopy(renderer, (*cooldownNumberSprites)[0], NULL, &(spellSprite.rect));
					}
				}
				else {
					onCooldown = false;
				}
			}
		}
};


#endif
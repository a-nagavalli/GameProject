#ifndef HEALTHBAR_H
#define HEALTHBAR_H

#include "Sprite.h"
#include <SDL.h>
#include <iostream>

class HealthBar
{
	public:
		Sprite sprite;
		SDL_Renderer* renderer;
		int maxWidth;

		HealthBar() {}

		HealthBar(SDL_Renderer* argRenderer, const std::string& spritePath, int x, int y, int width, int height)
		{
			renderer = argRenderer;
			SDL_Surface* surface = SDL_LoadBMP(spritePath.c_str());
			if (surface == NULL) {
				std::cout << "ERROR: HealthBar: couldn't load surface" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			if (texture == NULL) {
				std::cout << "ERROR: HealthBar: couldn't create texture" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			sprite.texture = texture;

			sprite.rect.x = x;
			sprite.rect.y = y;
			sprite.rect.w = width;
			sprite.rect.h = height;

			maxWidth = width;
		}

		void updateHealth(int hp, int totalHealth, int newX, int newY)
		{
			double ratio = hp * 1.0 / totalHealth;		// get percentage of health remaining (as decimal)
			int newWidth = round(ratio * maxWidth);		// use that percentage to determine what the new width of the health bar should be
			sprite.rect.w = newWidth;

			sprite.rect.x = newX;
			sprite.rect.y = newY;
		}

		void Draw(void)
		{
			SDL_RenderCopy(renderer, sprite.texture, NULL, &sprite.rect);
		}
};


#endif
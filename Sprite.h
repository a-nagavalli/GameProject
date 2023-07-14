#ifndef SPRITE_H
#define SPRITE_H

#include <string>
#include <SDL.h>

struct Sprite 
{
	SDL_Surface* surface;
	SDL_Texture* texture;
	SDL_Rect rect;
};


#endif

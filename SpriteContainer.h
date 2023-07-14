#ifndef SPRITE_CONTAINER_H
#define SPRITE_CONTAINER_H

#include <vector>
#include <SDL.h>


struct SpriteContainer
{
	std::vector<std::vector<SDL_Texture*>>* fireballSpritesPtr;
	//std::vector<SDL_Texture*>* targetedSpritesPtr;
	//SpriteContainer() {}

	/*SpriteContainer(std::vector<std::vector<SDL_Texture*>>* argFireballSprites)
	{
		fireballSpritesPtr = argFireballSprites;
	}*/
};




#endif

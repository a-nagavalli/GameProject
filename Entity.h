#ifndef ENTITY_H
#define ENTITY_H

#include <sstream>
#include <cctype>
#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include "Sprite.h"

enum SpriteDirections		// 8 directions representing which way a sprite is facing
{
	SP_NORTH,
	SP_NORTHEAST,
	SP_EAST,
	SP_SOUTHEAST,
	SP_SOUTH,
	SP_SOUTHWEST,
	SP_WEST,
	SP_NORTHWEST
};

class Entity	// an Entity is any movable character/object
{
	protected:
		std::vector<std::vector<SDL_Texture*>>* texturesPtr;
		SDL_Renderer* renderer;

		// movement
		int moveSpeedDelayHoriz, moveSpeedDelayVert, moveSpeedHoriz, moveSpeedVert; 
		bool canMoveHorizontal, canMoveVertical; 
		Uint64 startTimeHoriz, lastTimeHoriz, startTimeVert, lastTimeVert; 
		
		// sprite animations
		Uint64 lastTime, startTime;
		SpriteDirections lastDirectionAnim;
		int spriteIndex;
		bool canSwitchSprite;
		SpriteDirections lastDirection;
		int spriteSwitchDelay;

	public:
		Sprite sprite;
		bool movingLeft, movingRight, movingUp, movingDown; 
		bool shouldBeDestroyed;

		/// <summary>
		/// Create an Entity object
		/// </summary>
		/// <param name="argPath">Must be the path of a text file containing the paths of 8 sprites, each separated by a newline</param>
		Entity(SDL_Renderer* const& argRenderer, int xPos, int yPos, int width, int height,	std::vector<std::vector<SDL_Texture*>>* argTexturesPtr)
		{
			texturesPtr = argTexturesPtr;
			renderer = argRenderer;

			sprite.surface = nullptr;
			sprite.texture = (*texturesPtr)[SP_SOUTH][0];

			sprite.rect.x = xPos;
			sprite.rect.y = yPos;
			sprite.rect.w = width;
			sprite.rect.h = height;

			movingLeft = movingRight = movingUp = movingDown = false;
			moveSpeedDelayHoriz = moveSpeedDelayVert = moveSpeedHoriz = moveSpeedVert = 0;

			canMoveHorizontal = canMoveVertical = true;
			startTimeHoriz = lastTimeHoriz = startTimeVert = lastTimeVert = 0;

			lastTime = startTime = 0;
			lastDirectionAnim = SP_NORTH;
			spriteIndex = 0;
			canSwitchSprite = true;
			lastDirection = SpriteDirections::SP_SOUTH;
			spriteSwitchDelay = 0;

			shouldBeDestroyed = false;
		}

		void Draw(void)
		{
			SDL_RenderCopy(renderer, sprite.texture, NULL, &sprite.rect);
		}

		void updateMovementBools(Uint64 deltaTime)
		{
			lastTimeHoriz = lastTimeVert = SDL_GetTicks64();
			if ((lastTimeHoriz - startTimeHoriz) >= moveSpeedDelayHoriz) {	
				canMoveHorizontal = true;
			}

			if ((lastTimeVert - startTimeVert) >= moveSpeedDelayVert) {
				canMoveVertical = true;
			}

			if (movingRight && canMoveHorizontal && !movingLeft) {

				if (movingUp) {		// we already know it's moving to the right and not to the left; check the other possible directions
					lastDirection = SP_NORTHEAST;
					spriteAnim(SP_NORTHEAST);
				}
				else if (movingDown) {
					lastDirection = SP_SOUTHEAST;
					spriteAnim(SP_SOUTHEAST);
				}
				else {		// else, it must be moving to the right only
					lastDirection = SP_EAST;
					spriteAnim(SP_EAST);
				}

				sprite.rect.x += moveSpeedHoriz * deltaTime;

				if ((sprite.rect.x + sprite.rect.w) >= 800) {		// keep characters within bounds of screen
					sprite.rect.x = 800 - sprite.rect.w;
				}

				canMoveHorizontal = false;
				startTimeHoriz = SDL_GetTicks64();
			}

			if (movingLeft && canMoveHorizontal && !movingRight) {

				if (movingUp) {
					lastDirection = SP_NORTHWEST;
					spriteAnim(SP_NORTHWEST);
				}
				else if (movingDown) {
					lastDirection = SP_SOUTHWEST;
					spriteAnim(SP_SOUTHWEST);
				}
				else {
					lastDirection = SP_WEST;
					spriteAnim(SP_WEST);
				}

				sprite.rect.x -= moveSpeedHoriz * deltaTime;

				if (sprite.rect.x <= 0) {
					sprite.rect.x = 0;
				}

				canMoveHorizontal = false;
				startTimeHoriz = SDL_GetTicks64();
			}

			if (movingDown && canMoveVertical && !movingUp) {

				if (movingRight) {
					lastDirection = SP_SOUTHEAST;
					spriteAnim(SP_SOUTHEAST);
				}
				else if (movingLeft) {
					lastDirection = SP_SOUTHWEST;
					spriteAnim(SP_SOUTHWEST);
				}
				else {
					lastDirection = SP_SOUTH;
					spriteAnim(SP_SOUTH);
				}

				sprite.rect.y += moveSpeedVert * deltaTime;

				if ((sprite.rect.y + sprite.rect.h) >= 640) {
					sprite.rect.y = 640 - sprite.rect.h;
				}

				canMoveVertical = false;
				startTimeVert = SDL_GetTicks64();
			}

			if (movingUp && canMoveVertical && !movingDown) {

				if (movingRight) {
					lastDirection = SP_NORTHEAST;
					spriteAnim(SP_NORTHEAST);
				}
				else if (movingLeft) {
					lastDirection = SP_NORTHWEST;
					spriteAnim(SP_NORTHWEST);
				}
				else {
					lastDirection = SP_NORTH;
					spriteAnim(SP_NORTH);
				}

				sprite.rect.y -= moveSpeedVert * deltaTime;

				if (sprite.rect.y <= 0) {
					sprite.rect.y = 0;
				}

				canMoveVertical = false;
				startTimeVert = SDL_GetTicks64();
			}

			if (!movingUp && !movingDown && !movingLeft && !movingRight) {
				sprite.texture = (*texturesPtr)[lastDirection][0];
			}
		}

		virtual void Update(Uint64 deltaTime)
		{
			updateMovementBools(deltaTime);	
		}

		void setMoveSpeedDelayX(int speed)	// time before next input (horizontal or vertical) should have an effect
		{
			moveSpeedDelayHoriz = speed;
		}

		void setMoveSpeedDelayY(int speed)
		{
			moveSpeedDelayVert = speed;
		}

		void setMoveSpeedX(int speed)	// how many pixels will the player move per movement input
		{
			moveSpeedHoriz = speed;
		}

		void setMoveSpeedY(int speed)
		{
			moveSpeedVert = speed;
		}

		void setSpriteSwitchDelay(int num)
		{
			spriteSwitchDelay = num;
		}

		void spriteAnim(SpriteDirections direction)
		{			
			lastTime = SDL_GetTicks64();		// the current time

			if (direction != lastDirectionAnim) {	// if this is a new direction inputted
				spriteIndex = 0;
				canSwitchSprite = true;
			}
			else if ((lastTime - startTime) >= 100) {		// if this is the same direction as the last inputted AND at least spriteDelay milliseconds have passed since last sprite switch
				canSwitchSprite = true;
			}

			lastDirectionAnim = direction;		// update last recorded direction

			if (canSwitchSprite) {
				switch (direction) {
					case SP_NORTH:
						if (spriteIndex >= (*texturesPtr)[SP_NORTH].size() - 1) {	// make sure spriteIndex doesn't go out of range
							spriteIndex = 0;
						}
						sprite.texture = (*texturesPtr)[SP_NORTH][spriteIndex++];
						startTime = SDL_GetTicks64();
						canSwitchSprite = false;
						break;
					case SP_NORTHEAST:
						if (spriteIndex >= (*texturesPtr)[SP_NORTHEAST].size() - 1) {	// make sure spriteIndex doesn't go out of range
							spriteIndex = 0;
						}
						sprite.texture = (*texturesPtr)[SP_NORTHEAST][spriteIndex++];
						startTime = SDL_GetTicks64();
						canSwitchSprite = false;
						break;
					case SP_EAST:
						if (spriteIndex >= (*texturesPtr)[SP_EAST].size() - 1) {	// make sure spriteIndex doesn't go out of range
							spriteIndex = 0;
						}
						sprite.texture = (*texturesPtr)[SP_EAST][spriteIndex++];
						startTime = SDL_GetTicks64();
						canSwitchSprite = false;
						break;
					case SP_SOUTHEAST:
						if (spriteIndex >= (*texturesPtr)[SP_SOUTHEAST].size() - 1) {	// make sure spriteIndex doesn't go out of range
							spriteIndex = 0;
						}
						sprite.texture = (*texturesPtr)[SP_SOUTHEAST][spriteIndex++];
						startTime = SDL_GetTicks64();
						canSwitchSprite = false;
						break;
					case SP_SOUTH:
						if (spriteIndex >= (*texturesPtr)[SP_SOUTH].size() - 1) {	// make sure spriteIndex doesn't go out of range
							spriteIndex = 0;
						}
						sprite.texture = (*texturesPtr)[SP_SOUTH][spriteIndex++];
						startTime = SDL_GetTicks64();
						canSwitchSprite = false;
						break;
					case SP_SOUTHWEST:
						if (spriteIndex >= (*texturesPtr)[SP_SOUTHWEST].size() - 1) {	// make sure spriteIndex doesn't go out of range
							spriteIndex = 0;
						}
						sprite.texture = (*texturesPtr)[SP_SOUTHWEST][spriteIndex++];
						startTime = SDL_GetTicks64();
						canSwitchSprite = false;
						break;
					case SP_WEST:
						if (spriteIndex >= (*texturesPtr)[SP_WEST].size() - 1) {	// make sure spriteIndex doesn't go out of range
							spriteIndex = 0;
						}
						sprite.texture = (*texturesPtr)[SP_WEST][spriteIndex++];
						startTime = SDL_GetTicks64();
						canSwitchSprite = false;
						break;
					case SP_NORTHWEST:
						if (spriteIndex >= (*texturesPtr)[SP_NORTHWEST].size() - 1) {	// make sure spriteIndex doesn't go out of range
							spriteIndex = 0;
						}
						sprite.texture = (*texturesPtr)[SP_NORTHWEST][spriteIndex++];
						startTime = SDL_GetTicks64();
						canSwitchSprite = false;
						break;
				}
			}
		}
};

#endif

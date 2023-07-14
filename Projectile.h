#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <memory>

#include "Entity.h"
#include "Player.h"
#include "Enemy.h"

class Projectile : public Entity
{
	private:
		Uint64 startMoveTime, endMoveTime;
		Uint64 currTime, lastTime;
		bool greaterX, greaterY;
		int xDelay, yDelay;
		int targetUpdateDelay;
		int targetXPos, targetYPos;
		int deltaX, deltaY;
		int originX, originY;
		int targetOriginX, targetOriginY;

	public:
		int damage;
		std::weak_ptr<Player> playerPtr;
		std::weak_ptr<Enemy> enemyPtr;

		Projectile(SDL_Renderer* const& argRenderer, int xPos, int yPos, int width, int height, std::shared_ptr<Player> argPlayer, std::vector<std::vector<SDL_Texture*>>* argTexturesPtr, int argDamage)
			: Entity(argRenderer, xPos, yPos, width, height, argTexturesPtr), damage(argDamage)
		{
			startMoveTime = endMoveTime = 0;
			currTime = lastTime = 0;
			greaterX = greaterY = false;
			xDelay = yDelay = 0;
			targetUpdateDelay = 0;
			targetXPos = targetYPos = 0;
			deltaX = deltaY = 0;
			originX = originY = 0;
			targetOriginX = targetOriginY = 0;
			playerPtr = argPlayer;
			damage = 25;
		}

		Projectile(SDL_Renderer* const& argRenderer, int xPos, int yPos, int width, int height, std::shared_ptr<Enemy> argEnemy, std::vector<std::vector<SDL_Texture*>>* argTexturesPtr, int argDamage)
			: Entity(argRenderer, xPos, yPos, width, height, argTexturesPtr), damage(argDamage)
		{
			startMoveTime = endMoveTime = 0;
			currTime = lastTime = 0;
			greaterX = greaterY = false;
			xDelay = yDelay = 0;
			targetUpdateDelay = 0;
			targetXPos = targetYPos = 0;
			deltaX = deltaY = 0;
			originX = originY = 0;
			targetOriginX = targetOriginY = 0;
			enemyPtr = argEnemy;
			damage = 25;
		}

		void Update(Uint64 deltaTime)
		{
			updateMovementBools(deltaTime);
			updatePositionSmooth();
		}

		void moveToSmoothOrigin()		// moves the projectile towards the middle of the target object instead of the top left corner
		{
			currTime = SDL_GetTicks64();

			if ((currTime - lastTime) > targetUpdateDelay) {

				if (auto pPtr = playerPtr.lock()) {
					targetOriginX = pPtr->sprite.rect.x + (pPtr->sprite.rect.w / 2);
					targetOriginY = pPtr->sprite.rect.y + (pPtr->sprite.rect.h / 2);
				}
				else if (auto ePtr = enemyPtr.lock()) {
					targetOriginX = ePtr->sprite.rect.x + (ePtr->sprite.rect.w / 2);
					targetOriginY = ePtr->sprite.rect.y + (ePtr->sprite.rect.h / 2);
				}
				else {				// if the target has been destroyed before the projectile, destroy the projectile as well
					shouldBeDestroyed = true;
				}

				originX = sprite.rect.x + (sprite.rect.w / 2);
				originY = sprite.rect.y + (sprite.rect.h / 2);

				deltaX = abs(originX - targetOriginX);
				deltaY = abs(originY - targetOriginY);

				greaterX = originX > targetOriginX;
				greaterY = originY > targetOriginY;

				xDelay = int(round(2000 * 1.0 / deltaX));
				yDelay = int(round(2000 * 1.0 / deltaY));

				movingDown = movingUp = movingLeft = movingRight = false;

				lastTime = currTime;
			}

			// doing this for each possible combination of where the sprite is currently located in relation to where it needs to go
			if (!greaterX && !greaterY) {
				if (originX <= targetOriginX && originY <= targetOriginY) {	// this will produce smooth movement because the sprite will keep moving until both coordinates have reached or exceeded their destinations
					startMoveTime = SDL_GetTicks64();

					xDelay = sqrt(int(round(2000 * 1.0 / deltaX))) * 0.8;	// having xDelay and yDelay proportional to distance remaining produces a curved trajectory for the projectile
					yDelay = sqrt(int(round(2000 * 1.0 / deltaY))) * 0.8;

					setMoveSpeedX(1);
					setMoveSpeedY(1);
					setMoveSpeedDelayX(xDelay);
					setMoveSpeedDelayY(yDelay);

					movingRight = true;
					movingDown = true;
				}
			}

			if (greaterX && !greaterY) {
				if (originX >= targetOriginX && originY <= targetOriginY) {
					startMoveTime = SDL_GetTicks64();

					xDelay = sqrt(int(round(2000 * 1.0 / deltaX))) * 0.8;
					yDelay = sqrt(int(round(2000 * 1.0 / deltaY))) * 0.8;

					setMoveSpeedX(1);
					setMoveSpeedY(1);
					setMoveSpeedDelayX(xDelay);
					setMoveSpeedDelayY(yDelay);

					movingLeft = true;
					movingDown = true;
				}
			}

			if (!greaterX && greaterY) {
				if (originX <= targetOriginX && originY >= targetOriginY) {
					startMoveTime = SDL_GetTicks64();

					xDelay = sqrt(int(round(2000 * 1.0 / deltaX))) * 0.8;
					yDelay = sqrt(int(round(2000 * 1.0 / deltaY))) * 0.8;

					setMoveSpeedX(1);
					setMoveSpeedY(1);
					setMoveSpeedDelayX(xDelay);
					setMoveSpeedDelayY(yDelay);

					movingRight = true;
					movingUp = true;
				}
			}

			if (greaterX && greaterY) {
				if (originX >= targetOriginX && originY >= targetOriginY) {
					startMoveTime = SDL_GetTicks64();

					xDelay = sqrt(int(round(2000 * 1.0 / deltaX))) * 0.8;
					yDelay = sqrt(int(round(2000 * 1.0 / deltaY))) * 0.8;

					setMoveSpeedX(1);
					setMoveSpeedY(1);
					setMoveSpeedDelayX(xDelay);
					setMoveSpeedDelayY(yDelay);

					movingLeft = true;
					movingUp = true;
				}
			}
		}

		void setTargetUpdateDelay(Uint64 num)
		{
			targetUpdateDelay = num;
			lastTime = num;
		}

		void updatePositionSmooth()		// note that this function is called every pass through the main game loop
		{
			moveToSmoothOrigin();
		}
};



#endif
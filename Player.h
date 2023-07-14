#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "Spell_Enum.h"
#include "HealthBar.h"

class Player : public Entity
{
	private:
		Uint64 currTimeFireball, lastTimeFireball;
		Uint64 currTimeIceShard, lastTimeIceShard;
		Uint64 currTimeThunder, lastTimeThunder;

	public:
		int totalHealth, health;
		HealthBar healthbar;

		bool attemptCastFireball;
		bool attemptCastIceShard;
		bool attemptCastThunder;

		bool canCastFireball;
		bool canCastIceShard;
		bool canCastThunder;

		Player(SDL_Renderer* const& argRenderer, int xPos, int yPos, int width, int height, std::vector<std::vector<SDL_Texture*>>* argTexturesPtr)
			: Entity(argRenderer, xPos, yPos, width, height, argTexturesPtr)
		{
			currTimeFireball = 0;
			lastTimeFireball = 0;

			currTimeIceShard = 0;
			lastTimeIceShard = 0;

			currTimeThunder = 0;
			lastTimeThunder = 0;

			totalHealth = health = 500;

			attemptCastFireball = false;
			attemptCastIceShard = false;
			attemptCastThunder = false;

			canCastFireball = true;
			canCastIceShard = true;
			canCastThunder = true;
		}

		void startFireballCD()
		{
			canCastFireball = false;
			lastTimeFireball = SDL_GetTicks64();
		}

		void startIceShardCD()
		{
			canCastIceShard = false;
			lastTimeIceShard = SDL_GetTicks64();
		}

		void startThunderCD()
		{
			canCastThunder = false;
			lastTimeThunder = SDL_GetTicks64();
		}

		void updateTimers()
		{
			currTimeFireball = currTimeIceShard = currTimeThunder = SDL_GetTicks64();
			if ((currTimeFireball - lastTimeFireball) >= 2000) {
				canCastFireball = true;
			}
			if ((currTimeIceShard - lastTimeIceShard) >= 3000) {
				canCastIceShard = true;
			}
			if ((currTimeThunder - lastTimeThunder) >= 4500) {
				canCastThunder = true;
			}
		}

		void Update(Uint64 deltaTime)
		{
			updateTimers();
			updateMovementBools(deltaTime);
			updateHealthBar();
		}

		void setupHealthBar(const std::string& healthBarPath, int x, int y, int width, int height)
		{
			healthbar = HealthBar(renderer, healthBarPath, x, y, width, height);
		}

		void updateHealthBar(void)
		{
			int healthBarX = sprite.rect.x;
			int healthBarY = sprite.rect.y - 15;
			healthbar.updateHealth(health, totalHealth, healthBarX, healthBarY);
		}

};

#endif
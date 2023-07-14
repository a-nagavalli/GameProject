#ifndef ENEMY_H
#define ENEMY_H

#include <vector>
#include "Entity.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Spell_Enum.h"
#include "HealthBar.h"

class Enemy : public Entity
{
	private:
		std::vector<std::vector<int>> mvmtData; 
		bool firstLoop;
		bool greaterX, greaterY;
		int xDelay, yDelay;
		int phaseCount;
		Uint64 currTimeChooseSpell, lastTimeChooseSpell;
		Uint64 currTimeFireball, lastTimeFireball;
		Uint64 currTimeIceShard, lastTimeIceShard;
		Uint64 currTimeThunder, lastTimeThunder;

	public:
		int totalHealth, health;
		HealthBar healthbar;
		Spell chosenSpell;
		bool canChooseSpell;
		bool canCastFireball;
		bool canCastIceShard;
		bool canCastThunder;

		Enemy(SDL_Renderer* const& argRenderer, int xPos, int yPos, int width, int height, const std::string& argFilePath, std::vector<std::vector<SDL_Texture*>>* argTexturesPtr)
			: Entity(argRenderer, xPos, yPos, width, height, argTexturesPtr)
		{
			readMovementData(mvmtData, argFilePath); 

			firstLoop = true;
			greaterX = greaterY = false;
			xDelay = yDelay = 0;
			phaseCount = 0;

			currTimeChooseSpell = lastTimeChooseSpell = 0;
			currTimeFireball = lastTimeFireball = 0;
			currTimeIceShard = lastTimeIceShard = 0;
			currTimeThunder = lastTimeThunder = 0;

			health = totalHealth = 300;

			chosenSpell = Spell::FIREBALL;

			canChooseSpell = true;
			canCastFireball = true;
			canCastIceShard = true;
			canCastThunder = true;
		}

		void readMovementData(std::vector<std::vector<int>>& vec, const std::string& path)
		{
			std::string num;
			std::ifstream fileStream;
			std::vector<int> innerVec;

			fileStream.open(path.c_str());
			if (fileStream.is_open()) {
				while (fileStream >> num) {

					if (innerVec.size() == 3) {
						vec.push_back(innerVec);
						innerVec.clear();
					}

					innerVec.push_back(std::stoi(num));
				}
				vec.push_back(innerVec);	// get the last set of values
			}

			for (int i = 0; i < vec.size(); ++i) {		// should always have 3 values for each phase: xPos, yPos, and time
				if (vec[i].size() != 3) {
					std::cout << "ERROR: didn't read movement data correctly" << std::endl;
				}
			}
		}

		void updatePosition(void)		// note that this function is called every pass through the main game loop
		{		
			if (phaseCount == mvmtData.size()) {
				phaseCount = 0;
			}

			bool done = moveTo(mvmtData[phaseCount][0], mvmtData[phaseCount][1], mvmtData[phaseCount][2]);	
			if (done) {
				++phaseCount;
			}
		}

		void updatePositionSmooth(void)		// note that this function is called every pass through the main game loop
		{
			if (phaseCount == mvmtData.size()) {
				phaseCount = 0;
			}

			bool done = moveToSmooth(mvmtData[phaseCount][0], mvmtData[phaseCount][1], mvmtData[phaseCount][2]);
			if (done) {
				++phaseCount;
			}
		}

		bool moveTo(int xPos, int yPos, int time)		// returns a bool indicating whether or not this phase is done / the movement is completed
		{
			if (firstLoop) {
				greaterX = sprite.rect.x > xPos;	// starting x position of sprite is greater than or less than final x position
				greaterY = sprite.rect.y > yPos;

				int deltaX = abs(sprite.rect.x - xPos);		// distance needed to travel
				int deltaY = abs(sprite.rect.y - yPos);

				xDelay = int(round(time * 1.0 / deltaX));	// assuming pixel stride is 1; then, delay is desired time divided by how far it needs to go
				yDelay = int(round(time * 1.0 / deltaY));

				firstLoop = false;	
			}

			if (!greaterX) {
				if (sprite.rect.x <= xPos) {
					setMoveSpeedX(1);
					setMoveSpeedDelayX(xDelay);

					movingRight = true;
				}
				if (sprite.rect.x >= xPos) {
					setMoveSpeedX(1);	// reset
					setMoveSpeedDelayX(15);

					movingRight = false;
				}
			}

			if (!greaterY) {
				if (sprite.rect.y <= yPos) {
					setMoveSpeedY(1);
					setMoveSpeedDelayY(yDelay);

					movingDown = true;
				}
				if (sprite.rect.y >= yPos) {
					setMoveSpeedY(1);		// reset 
					setMoveSpeedDelayY(15);

					movingDown = false;
				}
			}

			if (greaterX) {
				if (sprite.rect.x >= xPos) {
					setMoveSpeedX(1);
					setMoveSpeedDelayX(xDelay);

					movingLeft = true;
				}
				if (sprite.rect.x <= xPos) {
					setMoveSpeedX(1);	// reset
					setMoveSpeedDelayX(15);

					movingLeft = false;
				}
			}

			if (greaterY) {
				if (sprite.rect.y >= yPos) {
					setMoveSpeedY(1);
					setMoveSpeedDelayY(yDelay);

					movingUp = true;
				}
				if (sprite.rect.y <= yPos) {
					setMoveSpeedY(1);		// reset 
					setMoveSpeedDelayY(15);

					movingUp = false;
				}
			}

			if (!movingDown && !movingUp && !movingRight && !movingLeft) {		// all movements are done; movement is completed
				firstLoop = true;	// reset for next phase of movement
				return true;
			}

			return false;	// movement hasn't yet completed
		}

		bool moveToSmooth(int xPos, int yPos, int time)		// this will only get the sprite close to the target position (not exact) in the approximate length of time
		{
			if (firstLoop) {
				greaterX = sprite.rect.x > xPos;	// starting x position of sprite is greater than or less than final x position
				greaterY = sprite.rect.y > yPos;

				int deltaX = abs(sprite.rect.x - xPos);		// distance needed to travel
				int deltaY = abs(sprite.rect.y - yPos);

				xDelay = int(round(time * 1.0 / deltaX));	// assuming pixel stride is 1; then, delay is desired time divided by how far it needs to go
				yDelay = int(round(time * 1.0 / deltaY));

				firstLoop = false;	
			}

			// doing this for each possible combination of where the sprite is currently located in relation to where it needs to go
			if (!greaterX && !greaterY) {
				if (sprite.rect.x <= xPos && sprite.rect.y <= yPos) {	// this will produce smooth movement because the sprite will keep moving until both coordinates have reached or exceeded their destinations
					setMoveSpeedX(1);
					setMoveSpeedY(1);
					setMoveSpeedDelayX(xDelay);
					setMoveSpeedDelayY(yDelay);

					movingRight = true;
					movingDown = true;
				}

				int right = sprite.rect.x + sprite.rect.w;
				int bottom = sprite.rect.y + sprite.rect.h;
				if ((sprite.rect.x >= xPos && sprite.rect.y >= yPos) || (right >= 800) || (bottom >= 640)) {	// if the sprite has reached the position, or if it has reached the edge of the screen
					setMoveSpeedX(1);	// reset
					setMoveSpeedY(1);
					setMoveSpeedDelayX(15);
					setMoveSpeedDelayY(15);

					movingRight = false;
					movingDown = false;
					firstLoop = true;	// reset for next phase of movement

					return true;
				}
			}

			if (greaterX && !greaterY) {
				if (sprite.rect.x >= xPos && sprite.rect.y <= yPos) {
					setMoveSpeedX(1);
					setMoveSpeedY(1);
					setMoveSpeedDelayX(xDelay);
					setMoveSpeedDelayY(yDelay);

					movingLeft = true;
					movingDown = true;
				}

				int left = sprite.rect.x;
				int bottom = sprite.rect.y + sprite.rect.h;
				if ((sprite.rect.x <= xPos && sprite.rect.y >= yPos) || (left <= 0) || (bottom >= 640)) {
					setMoveSpeedX(1);	// reset
					setMoveSpeedY(1);
					setMoveSpeedDelayX(15);
					setMoveSpeedDelayY(15);

					movingLeft = false;
					movingDown = false;
					firstLoop = true;

					return true;
				}
			}

			if (!greaterX && greaterY) {
				if (sprite.rect.x <= xPos && sprite.rect.y >= yPos) {
					setMoveSpeedX(1);
					setMoveSpeedY(1);
					setMoveSpeedDelayX(xDelay);
					setMoveSpeedDelayY(yDelay);

					movingRight = true;
					movingUp = true;
				}

				int right = sprite.rect.x + sprite.rect.w;
				int top = sprite.rect.y;
				if ((sprite.rect.x >= xPos && sprite.rect.y <= yPos) || (right >= 800) || (top <= 0)) {
					setMoveSpeedX(1);	// reset
					setMoveSpeedY(1);
					setMoveSpeedDelayX(15);
					setMoveSpeedDelayY(15);

					movingRight = false;
					movingUp = false;
					firstLoop = true;

					return true;
				}
			}

			if (greaterX && greaterY) {
				if (sprite.rect.x >= xPos && sprite.rect.y >= yPos) {
					setMoveSpeedX(1);
					setMoveSpeedY(1);
					setMoveSpeedDelayX(xDelay);
					setMoveSpeedDelayY(yDelay);

					movingLeft = true;
					movingUp = true;
				}

				int left = sprite.rect.x;
				int top = sprite.rect.y;
				if ((sprite.rect.x <= xPos && sprite.rect.y <= yPos) || (left <= 0) || (top <= 0)) {
					setMoveSpeedX(1);	// reset
					setMoveSpeedY(1);
					setMoveSpeedDelayX(15);
					setMoveSpeedDelayY(15);

					movingLeft = false;
					movingUp = false;
					firstLoop = true;

					return true;
				}
			}
			
			return false;	// movement hasn't yet completed
		}

		void startSpellChooseCD()
		{
			canChooseSpell = false;
			lastTimeChooseSpell = SDL_GetTicks64();
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
			// every 3 seconds, choose random spell to cast, and attempt to cast it
			currTimeChooseSpell = currTimeFireball = currTimeIceShard = currTimeThunder = SDL_GetTicks64();
			if ((currTimeChooseSpell - lastTimeChooseSpell) >= 3000) {
				canChooseSpell = true;
			}
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

		void chooseNewSpell(void)
		{
			int i = rand() % 3;
			switch (i) {
				case 0:
					chosenSpell = Spell::FIREBALL;
					break;
				case 1:
					chosenSpell = Spell::ICE_SHARD;
					break;
				case 2:
					chosenSpell = Spell::THUNDER;
					break;
				default:
					chosenSpell = Spell::FIREBALL;
					break;
			}
		}

		void Update(Uint64 deltaTime)
		{
			updateMovementBools(deltaTime);
			updatePositionSmooth();
			updateTimers();
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



#endif ENEMY_H
#ifndef MANAGER_H
#define MANAGER_H

#include <SDL.h>
#include <vector>
#include <memory>

#include "Enemy.h"
#include "Player.h"
#include "Projectile.h"
#include "SpellbarBox.h"

class Manager
{
	public:

		SDL_Renderer* renderer;

		std::vector<std::vector<SDL_Texture*>> playerSprites;
		std::vector<std::vector<SDL_Texture*>> enemySprites;
		std::vector<std::vector<SDL_Texture*>> fireballSprites;
		std::vector<std::vector<SDL_Texture*>> iceShardSprites;
		std::vector<std::vector<SDL_Texture*>> thunderSprites;

		std::weak_ptr<Enemy> targetedEnemyPtr;

		std::vector<std::shared_ptr<Player>> players;
		std::vector<std::shared_ptr<Enemy>> enemies;
		std::vector<std::shared_ptr<Projectile>> projectiles;

		std::vector<SpellbarBox> spellbarBoxes;

		bool* runningPtr;

		void Update(Uint64 deltaTime)
		{
			// Update enemies
			for (int i = 0; i < enemies.size(); ++i) {
				if (enemies[i] != nullptr) {

					// check if still "alive"
					if (enemies[i]->shouldBeDestroyed) {
						enemies[i] = nullptr;
						continue;
					}

					if (enemies[i]->health <= 0) {
						enemies[i] = nullptr;
						continue;
					}

					// first, see if enemy is able to choose and cast a new spell
					if (enemies[i]->canChooseSpell) {
						enemies[i]->chooseNewSpell();

						// then, check if enemy has chosen Fireball and is able to cast it (target would be Player)
						if (enemies[i]->chosenSpell == Spell::FIREBALL && enemies[i]->canCastFireball) {
							int middleX = enemies[i]->sprite.rect.x + (enemies[i]->sprite.rect.w / 2);
							int middleY = enemies[i]->sprite.rect.y + (enemies[i]->sprite.rect.h / 2);

							if (players[0] != nullptr) {
								generateFireball(middleX, middleY, players[0]);
							}

							enemies[i]->startFireballCD();
						}
						// ...or, if enemy didn't choose Fireball, check if enemy has chosen Ice Shard and is able to cast it (target would be Player)
						else if (enemies[i]->chosenSpell == Spell::ICE_SHARD && enemies[i]->canCastIceShard) {
							int middleX = enemies[i]->sprite.rect.x + (enemies[i]->sprite.rect.w / 2);
							int middleY = enemies[i]->sprite.rect.y + (enemies[i]->sprite.rect.h / 2);

							if (players[0] != nullptr) {
								generateIceShard(middleX, middleY, players[0]);
							}

							enemies[i]->startIceShardCD();
						}

						else if (enemies[i]->chosenSpell == Spell::THUNDER && enemies[i]->canCastThunder) {
							int middleX = enemies[i]->sprite.rect.x + (enemies[i]->sprite.rect.w / 2);
							int middleY = enemies[i]->sprite.rect.y + (enemies[i]->sprite.rect.h / 2);

							if (players[0] != nullptr) {
								generateThunder(middleX, middleY, players[0]);
							}

							enemies[i]->startThunderCD();
						}

						enemies[i]->startSpellChooseCD();
					}

					// update movements
					enemies[i]->Update(deltaTime);
				}
			}

			// Update players
			for (int i = 0; i < players.size(); ++i) {
				if (players[i] != nullptr) {

					// check if still "alive"
					if (players[i]->shouldBeDestroyed) {
						players[i] = nullptr;
						continue;
					}

					if (players[i]->health <= 0) {
						players[i] = nullptr;

						SDL_Surface* surface = SDL_LoadBMP("UI Sprites/lose.bmp");
						if (surface == NULL) {
							std::cout << "ERROR: lose.bmp: couldn't load surface" << std::endl;
							std::cout << SDL_GetError() << std::endl;
						}
						SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
						if (texture == NULL) {
							std::cout << "ERROR: lose.bmp: couldn't create texture" << std::endl;
							std::cout << SDL_GetError() << std::endl;
						}
						SDL_Rect rect;
						rect.x = 0;
						rect.y = 0;
						rect.w = 800;
						rect.h = 640;

						Sprite sprite;
						sprite.texture = texture;
						sprite.rect = rect;

						if (askRetry(sprite)) {
							retry();
						}
						else {
							*runningPtr = false;
							return;
						}
					}

					// check if trying to cast Fireball (target would be selected enemy)
					if (players[i]->attemptCastFireball) {
						if (players[i]->canCastFireball) {
							int middleX = players[i]->sprite.rect.x + (players[i]->sprite.rect.w / 2);
							int middleY = players[i]->sprite.rect.y + (players[i]->sprite.rect.h / 2);

							if (auto tePtr = targetedEnemyPtr.lock()) {
								generateFireball(middleX, middleY, tePtr);
								spellbarBoxes[0].startCooldown();
							}

							players[i]->startFireballCD();

						}
						else {
							players[i]->attemptCastFireball = false;
						}
					}
					// check if trying to cast IceShard (target would be selected enemy)
					if (players[i]->attemptCastIceShard) {
						if (players[i]->canCastIceShard) {
							int middleX = players[i]->sprite.rect.x + (players[i]->sprite.rect.w / 2);
							int middleY = players[i]->sprite.rect.y + (players[i]->sprite.rect.h / 2);

							if (auto tePtr = targetedEnemyPtr.lock()) {
								generateIceShard(middleX, middleY, tePtr);
								spellbarBoxes[1].startCooldown();
							}

							players[i]->startIceShardCD();

						}
						else {
							players[i]->attemptCastIceShard = false;
						}
					}
					// check if trying to cast Thunder (target would be selected enemy)
					if (players[i]->attemptCastThunder) {
						if (players[i]->canCastThunder) {
							int middleX = players[i]->sprite.rect.x + (players[i]->sprite.rect.w / 2);
							int middleY = players[i]->sprite.rect.y + (players[i]->sprite.rect.h / 2);

							if (auto tePtr = targetedEnemyPtr.lock()) {
								generateThunder(middleX, middleY, tePtr);
								spellbarBoxes[2].startCooldown();
							}

							players[i]->startThunderCD();

						}
						else {
							players[i]->attemptCastThunder = false;
						}
					}

					// update movements
					players[i]->Update(deltaTime);

				}
			}

			// Update projectiles
			for (int i = 0; i < projectiles.size(); ++i) {
				if (projectiles[i] != nullptr) {

					// check if still "alive"
					if (projectiles[i]->shouldBeDestroyed) {
						projectiles[i] = nullptr;
						continue;
					}

					// update movements
					projectiles[i]->Update(deltaTime);
				}
			}

			updateProjectileCollisions();

			// Display Win screen
			if (checkIfEnemiesDead()) {

				SDL_Surface* surface = SDL_LoadBMP("UI Sprites/win.bmp");
				if (surface == NULL) {
					std::cout << "ERROR: win.bmp: couldn't load surface" << std::endl;
					std::cout << SDL_GetError() << std::endl;
				}
				SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
				if (texture == NULL) {
					std::cout << "ERROR: win.bmp: couldn't create texture" << std::endl;
					std::cout << SDL_GetError() << std::endl;
				}
				SDL_Rect rect;
				rect.x = 0;
				rect.y = 0;
				rect.w = 800;
				rect.h = 640;

				Sprite sprite;
				sprite.texture = texture;
				sprite.rect = rect;

				if (askRetry(sprite)) {
					retry();
				}
				else {
					*runningPtr = false;
					return;
				}
			}
		}

		bool checkIfEnemiesDead(void)
		{
			bool allEnemiesDead = true;

			for (int i = 0; i < enemies.size(); ++i) {
				if (enemies[i] != nullptr) {
					allEnemiesDead = false;
					break;
				}
			}

			return allEnemiesDead;
		}

		// two overloads for generating a projectile: one if the target is Player, one if the target is Enemy
		void generateFireball(int xPos, int yPos, std::shared_ptr<Player> target)
		{
			for (int i = 0; i < projectiles.size(); ++i) {
				if (projectiles[i] == nullptr) {
					projectiles[i] = std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &fireballSprites, 25);
					return;
				}
			}
			projectiles.push_back(std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &fireballSprites, 25));
		}

		void generateFireball(int xPos, int yPos, std::shared_ptr<Enemy> target)
		{
			for (int i = 0; i < projectiles.size(); ++i) {
				if (projectiles[i] == nullptr) {
					projectiles[i] = std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &fireballSprites, 25);
					return;
				}
			}
			projectiles.push_back(std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &fireballSprites, 25));
		}

		void generateIceShard(int xPos, int yPos, std::shared_ptr<Player> target)
		{
			for (int i = 0; i < projectiles.size(); ++i) {
				if (projectiles[i] == nullptr) {
					projectiles[i] = std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &iceShardSprites, 35);
					return;
				}
			}
			projectiles.push_back(std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &iceShardSprites, 35));
		}

		void generateIceShard(int xPos, int yPos, std::shared_ptr<Enemy> target)
		{
			for (int i = 0; i < projectiles.size(); ++i) {
				if (projectiles[i] == nullptr) {
					projectiles[i] = std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &iceShardSprites, 35);
					return;
				}
			}
			projectiles.push_back(std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &iceShardSprites, 35));
		}

		void generateThunder(int xPos, int yPos, std::shared_ptr<Player> target)
		{
			for (int i = 0; i < projectiles.size(); ++i) {
				if (projectiles[i] == nullptr) {
					projectiles[i] = std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &thunderSprites, 50);
					return;
				}
			}
			projectiles.push_back(std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &thunderSprites, 50));
		}

		void generateThunder(int xPos, int yPos, std::shared_ptr<Enemy> target)
		{
			for (int i = 0; i < projectiles.size(); ++i) {
				if (projectiles[i] == nullptr) {
					projectiles[i] = std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &thunderSprites, 50);
					return;
				}
			}
			projectiles.push_back(std::make_shared<Projectile>(renderer, xPos, yPos, 30, 30, target, &thunderSprites, 50));
		}

		void updateProjectileCollisions(void)
		{
			for (int i = 0; i < projectiles.size(); ++i) {
				checkCollisions(projectiles[i]);
			}
		}

		void checkCollisions(std::shared_ptr<Projectile> projectile)
		{
			if (projectile != nullptr) {
				if (auto pPtr = projectile->playerPtr.lock()) {
					// these coordinates represent the target sprite's bounding box
					int right = pPtr->sprite.rect.x + pPtr->sprite.rect.w;
					int left = pPtr->sprite.rect.x;
					int top = pPtr->sprite.rect.y;
					int bottom = pPtr->sprite.rect.y + pPtr->sprite.rect.h;

					if (projectile->sprite.rect.x >= left && (projectile->sprite.rect.x + projectile->sprite.rect.w) <= right && projectile->sprite.rect.y >= top && (projectile->sprite.rect.y + projectile->sprite.rect.h) <= bottom) {
						pPtr->health -= projectile->damage;		// subtract damage
						projectile->shouldBeDestroyed = true;
					}
				}
				else if (auto ePtr = projectile->enemyPtr.lock()) {
					int right = ePtr->sprite.rect.x + ePtr->sprite.rect.w;
					int left = ePtr->sprite.rect.x;
					int top = ePtr->sprite.rect.y;
					int bottom = ePtr->sprite.rect.y + ePtr->sprite.rect.h;

					if (projectile->sprite.rect.x >= left && (projectile->sprite.rect.x + projectile->sprite.rect.w) <= right && projectile->sprite.rect.y >= top && (projectile->sprite.rect.y + projectile->sprite.rect.h) <= bottom) {
						ePtr->health -= projectile->damage;
						projectile->shouldBeDestroyed = true;
					}
				}
			}
		}

		bool askRetry(const Sprite& overlay)
		{
			bool gaveResponse = false;
			bool response = false;
			SDL_Event event;

			while (!gaveResponse) {

				SDL_RenderClear(renderer);		// display the overlayed screen
				SDL_RenderCopy(renderer, overlay.texture, NULL, &(overlay.rect));
				SDL_RenderPresent(renderer);

				while (SDL_PollEvent(&event)) {
					switch (event.type) {
						// close window, etc.
					case SDL_QUIT:
						*runningPtr = false;
						gaveResponse = true;
						response = false;
						break;

					case SDL_KEYDOWN:
						switch (event.key.keysym.sym) {
							// use escape key to quit
						case SDLK_ESCAPE:
							*runningPtr = false;
							gaveResponse = true;
							response = false;
							break;

							// query if player wants to retry
						case SDLK_r:
							gaveResponse = true;
							response = true;
							break;
						case SDLK_q:
							gaveResponse = true;
							response = false;
							break;
						}
						break;
					}
				}
			}

			return response;
		}

		void retry(void)	// reset everything to how it was at the beginning
		{
			targetedEnemyPtr.reset();

			for (int i = 0; i < spellbarBoxes.size(); ++i) {
				spellbarBoxes[i].onCooldown = false;
			}

			for (int i = 0; i < players.size(); ++i) {
				players[i].reset();
			}
			players.clear();

			for (int i = 0; i < enemies.size(); ++i) {
				enemies[i].reset();
			}
			enemies.clear();

			for (int i = 0; i < projectiles.size(); ++i) {
				projectiles[i].reset();
			}
			projectiles.clear();

			players.push_back(std::make_shared<Player>(renderer, 100, 50, 50, 75, &playerSprites));
			players[0]->setMoveSpeedDelayX(9);
			players[0]->setMoveSpeedDelayY(9);
			players[0]->setMoveSpeedX(1);
			players[0]->setMoveSpeedY(1);
			players[0]->setSpriteSwitchDelay(150);

			int healthBarX = players[0]->sprite.rect.x;
			int healthBarY = players[0]->sprite.rect.y - 15;
			players[0]->setupHealthBar("healthBarSprite.bmp", healthBarX, healthBarY, 51, 7);

			enemies.push_back(std::make_shared<Enemy>(renderer, 500, 450, 50, 75, "goblin2.mvt", &enemySprites));
			enemies[0]->setMoveSpeedDelayX(15);
			enemies[0]->setMoveSpeedDelayY(15);
			enemies[0]->setMoveSpeedX(1);
			enemies[0]->setMoveSpeedY(1);
			enemies[0]->setSpriteSwitchDelay(150);

			healthBarX = enemies[0]->sprite.rect.x;
			healthBarY = enemies[0]->sprite.rect.y - 15;
			enemies[0]->setupHealthBar("healthBarSprite.bmp", healthBarX, healthBarY, 51, 7);

			enemies.push_back(std::make_shared<Enemy>(renderer, 300, 250, 50, 75, "goblin1.mvt", &enemySprites));
			enemies[1]->setMoveSpeedDelayX(15);
			enemies[1]->setMoveSpeedDelayY(15);
			enemies[1]->setMoveSpeedX(1);
			enemies[1]->setMoveSpeedY(1);
			enemies[1]->setSpriteSwitchDelay(150);

			healthBarX = enemies[1]->sprite.rect.x;
			healthBarY = enemies[1]->sprite.rect.y - 15;
			enemies[1]->setupHealthBar("healthBarSprite.bmp", healthBarX, healthBarY, 51, 7);
		}
};


#endif
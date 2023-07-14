#include <SDL.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>

#include "Entity.h"
#include "Enemy.h"
#include "Player.h"
#include "Projectile.h"
#include "Sprite.h"
#include "SpellbarBox.h"
#include "Manager.h"
#include "Spell_Enum.h"

bool running = true;
const Uint64 MAX_DT = 17;	// every update should be in intervals of dt (max should be 17 ms)

// Function prototypes
void handleInput(void);
void loadTextures(std::vector<std::vector<SDL_Texture*>>& spriteVec, const std::string& path, SDL_Renderer* const& renderer);
std::shared_ptr<Enemy> processClick(int x, int y);
void Draw(void);
void loadTileLayout(std::vector<Sprite>& tilesVec, const std::vector<SDL_Texture*> tileSpritesVec, const std::string& path);
void loadTileTextures(std::vector<SDL_Texture*>& tileSpritesVec, const std::string& path, SDL_Renderer* const& renderer);
void loadCooldownNumbers(std::vector<SDL_Texture*>& cooldownNumbersVec, const std::string& path, SDL_Renderer* const& renderer);

/*
	TO-DO:	

	(maybe)
		use SDL_PointInRect for mouse
		optimize w/o shared_ptrs
		enemy approaching player
		make generateFireball in Enemy class use shared_ptr instead of index
		update all texture vectors to be formed from SpriteContainer
		make each entity have its own bounding box member
		probably will need to implement IDs for every object
		optimization: find some way to handleInput without needing to create a new pointer object every tick
			(im making a new pointer object every tick for the Projectile anyway, so maybe i shoudn't bother optimizing this (?))
		allow user to set the magic numbers in Projectile moveToSmooth()
		melee attack
		call Enemy.updatePosition() in Enemy.Update()?
		rename Enemy class
		rename "delay" stuff and "speed" stuff to timing/stride?
		should movement timers be done in the dt loop? could do them there and then pass them to the object
				or could just call goblin's updatePosition() function in the dt loop 
*/

// Objects with global scope
// renderer
SDL_Renderer* renderer;

// sprite containers
std::vector<std::vector<SDL_Texture*>> playerSprites;
std::vector<std::vector<SDL_Texture*>> enemySprites;
std::vector<std::vector<SDL_Texture*>> fireballSprites;
std::vector<std::vector<SDL_Texture*>> iceShardSprites;
std::vector<std::vector<SDL_Texture*>> thunderSprites;
std::vector<SDL_Texture*> cooldownNumberSprites;
std::vector<SDL_Texture*> tileSprites;
std::vector<Sprite> tiles;

// target selection
SDL_Texture* targetSelectorTexture;
SDL_Rect targetSelectorRect;
std::shared_ptr<Enemy> targetedEnemyPtr;

// object to manage object movement and relationships between objects
Manager manager;

int main(int argc, char* argv[])
{
	srand(time(NULL));

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	/*SDL_Window* window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);*/
	SDL_Window* window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 640, 0);
	renderer = SDL_CreateRenderer(window, -1, 0);

	// load any necessary textures in (for any object)
	loadTextures(playerSprites, "playerSprites.txt", renderer);
	loadTextures(enemySprites, "enemySprites.txt", renderer);
	loadTextures(fireballSprites, "fireballSprites.txt", renderer);
	loadTextures(iceShardSprites, "iceShardSprites.txt", renderer);
	loadTextures(thunderSprites, "thunderSprites.txt", renderer);

	// EntityManager settings
	// loading sprites
	manager.renderer = renderer;
	manager.fireballSprites = fireballSprites;
	manager.iceShardSprites = iceShardSprites;
	manager.thunderSprites = thunderSprites;
	manager.playerSprites = playerSprites;
	manager.enemySprites = enemySprites;

	// load entities and set them up
	manager.players.push_back(std::make_shared<Player>(renderer, 100, 50, 50, 75, &playerSprites));
	manager.players[0]->setMoveSpeedDelayX(9);
	manager.players[0]->setMoveSpeedDelayY(9);
	manager.players[0]->setMoveSpeedX(1);
	manager.players[0]->setMoveSpeedY(1);
	manager.players[0]->setSpriteSwitchDelay(150);
	
	int healthBarX = manager.players[0]->sprite.rect.x;
	int healthBarY = manager.players[0]->sprite.rect.y - 15;
	manager.players[0]->setupHealthBar("UI Sprites/healthBarSprite.bmp", healthBarX, healthBarY, 51, 7);

	manager.enemies.push_back(std::make_shared<Enemy>(renderer, 500, 450, 50, 75, "goblin2.mvt", &enemySprites));
	manager.enemies[0]->setMoveSpeedDelayX(15);
	manager.enemies[0]->setMoveSpeedDelayY(15);
	manager.enemies[0]->setMoveSpeedX(1);
	manager.enemies[0]->setMoveSpeedY(1);
	manager.enemies[0]->setSpriteSwitchDelay(150);

	healthBarX = manager.enemies[0]->sprite.rect.x;
	healthBarY = manager.enemies[0]->sprite.rect.y - 15;
	manager.enemies[0]->setupHealthBar("UI Sprites/healthBarSprite.bmp", healthBarX, healthBarY, 51, 7);

	manager.enemies.push_back(std::make_shared<Enemy>(renderer, 300, 250, 50, 75, "goblin1.mvt", &enemySprites));
	manager.enemies[1]->setMoveSpeedDelayX(15);
	manager.enemies[1]->setMoveSpeedDelayY(15);
	manager.enemies[1]->setMoveSpeedX(1);
	manager.enemies[1]->setMoveSpeedY(1);
	manager.enemies[1]->setSpriteSwitchDelay(150);

	healthBarX = manager.enemies[1]->sprite.rect.x;
	healthBarY = manager.enemies[1]->sprite.rect.y - 15;
	manager.enemies[1]->setupHealthBar("UI Sprites/healthBarSprite.bmp", healthBarX, healthBarY, 51, 7);

	// set up target selector icon
	SDL_Surface* targetSelectorSurface = SDL_LoadBMP("UI Sprites/targetSelector.bmp");
	if (targetSelectorSurface == NULL) {
		std::cout << "ERROR: TargetSelector: couldn't load surface" << std::endl;
		std::cout << SDL_GetError() << std::endl;
	}
	targetSelectorTexture = SDL_CreateTextureFromSurface(renderer, targetSelectorSurface);
	if (targetSelectorTexture == NULL) {
		std::cout << "ERROR: TargetSelector: couldn't create texture" << std::endl;
		std::cout << SDL_GetError() << std::endl;
	}
	targetSelectorRect;
	targetSelectorRect.x = targetSelectorRect.y = 0;
	targetSelectorRect.w = 60;
	targetSelectorRect.h = 20;

	// laod cooldown number sprites (used in hotbar)
	loadCooldownNumbers(cooldownNumberSprites, "cooldownNumberSprites.txt", renderer);

	// UI elements
	manager.spellbarBoxes.emplace_back(renderer, "UI Sprites/boxSprite.bmp", "UI Sprites/fireballIconSprite.bmp", 225, 525, 60, 60, 2000, &cooldownNumberSprites);
	manager.spellbarBoxes.emplace_back(renderer, "UI Sprites/boxSprite.bmp", "UI Sprites/iceShardIconSprite.bmp", 290, 525, 60, 60, 3000, &cooldownNumberSprites);
	manager.spellbarBoxes.emplace_back(renderer, "UI Sprites/boxSprite.bmp", "UI Sprites/thunderIconSprite.bmp", 355, 525, 60, 60, 4500, &cooldownNumberSprites);

	// load tiles (textures and layout)
	loadTileTextures(tileSprites, "tileSprites.txt", renderer);
	loadTileLayout(tiles, tileSprites, "tileLayout1.txt");

	// allow Manager to set the global running bool
	manager.runningPtr = &running;

	Uint64 lastTime = SDL_GetTicks64();
	while (running) {	// game loop

		Uint64 currTime = SDL_GetTicks64();		//milliseconds passed since SDL library initialized
		Uint64 frameTime = currTime - lastTime;		// frameTime now holds the amount of time it took to get to the next frame
		lastTime = currTime;

		handleInput();
		SDL_RenderClear(renderer);

		while (frameTime > 0) {		// while we haven't expended all of the time interval, update the position in intervals of dt
			Uint64 deltaTime = std::min(frameTime, MAX_DT);		// deltaTime shouldn't get any higher than MAX_DT
			manager.Update(deltaTime);
			frameTime -= deltaTime;
		}

		Draw();

		SDL_RenderPresent(renderer);		
	}

	SDL_Quit();
	return 0;
}

void Draw(void)
{
	// draw ground tiles
	for (int i = 0; i < tiles.size(); ++i) {
		SDL_RenderCopy(renderer, tiles[i].texture, NULL, &(tiles[i].rect));
	}

	// draw player(s) and their healthbars
	for (int i = 0; i < manager.players.size(); ++i) {
		if (manager.players[i] != nullptr) {
			manager.players[i]->Draw();
			manager.players[i]->healthbar.Draw();
		}
	}
	
	// draw target selector icon
	if (auto tePtr = manager.targetedEnemyPtr.lock()) {
		targetSelectorRect.x = tePtr->sprite.rect.x;
		targetSelectorRect.y = tePtr->sprite.rect.y + (tePtr->sprite.rect.h - 15);
		SDL_RenderCopy(renderer, targetSelectorTexture, NULL, &targetSelectorRect);
	}

	// draw enemies and their healthbars
	for (int i = 0; i < manager.enemies.size(); ++i) {
		if (manager.enemies[i] != nullptr) {
			manager.enemies[i]->Draw();
			manager.enemies[i]->healthbar.Draw();
		}
	}

	// draw projectiles
	for (int i = 0; i < manager.projectiles.size(); ++i) {
		if (manager.projectiles[i] != nullptr) {
			manager.projectiles[i]->Draw();
		}
	}

	// draw spellbar boxes (hotbar icons)
	for (int i = 0; i < manager.spellbarBoxes.size(); ++i) {
		manager.spellbarBoxes[i].Draw();
	}
}

void handleInput(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			// close window, etc.
			case SDL_QUIT:
				running = false;
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					// use escape key to quit
					case SDLK_ESCAPE:
						running = false;
						break;

					// player movement
					case SDLK_d:
					case SDLK_RIGHT:
						if (manager.players[0] != nullptr) {
							manager.players[0]->movingRight = true;
						}
						break;

					case SDLK_a:
					case SDLK_LEFT:
						if (manager.players[0] != nullptr) {
							manager.players[0]->movingLeft = true;
						}
						break;

					case SDLK_s:
					case SDLK_DOWN:
						if (manager.players[0] != nullptr) {
							manager.players[0]->movingDown = true;
						}
						break;

					case SDLK_w:
					case SDLK_UP:
						if (manager.players[0] != nullptr) {
							manager.players[0]->movingUp = true;
						}
						break;

					// player abilities
					case SDLK_q:
						if (manager.players[0] != nullptr) {
							if (auto tePtr = manager.targetedEnemyPtr.lock()) {
								manager.players[0]->attemptCastFireball = true;
							}
						}
						break;
					case SDLK_e:
						if (manager.players[0] != nullptr) {
							if (auto tePtr = manager.targetedEnemyPtr.lock()) {
								manager.players[0]->attemptCastIceShard = true;
							}
						}
						break;
					case SDLK_r:
						if (manager.players[0] != nullptr) {
							if (auto tePtr = manager.targetedEnemyPtr.lock()) {
								manager.players[0]->attemptCastThunder = true;
							}
						}
						break;
					}
				break;

			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
					// player movement
					case SDLK_d:
					case SDLK_RIGHT:
						if (manager.players[0] != nullptr) {
							manager.players[0]->movingRight = false;
						}
						break;

					case SDLK_a:
					case SDLK_LEFT:
						if (manager.players[0] != nullptr) {
							manager.players[0]->movingLeft = false;
						}
						break;

					case SDLK_s:
					case SDLK_DOWN:
						if (manager.players[0] != nullptr) {
							manager.players[0]->movingDown = false;
						}
						break;

					case SDLK_w:
					case SDLK_UP:
						if (manager.players[0] != nullptr) {
							manager.players[0]->movingUp = false;
						}
						break;
					}
				break;
			}

		switch (event.button.type) {
			// target selection with mouse
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button) {
					case SDL_BUTTON_LEFT:
						if (manager.players[0] != nullptr) {
							manager.targetedEnemyPtr.reset();
							manager.targetedEnemyPtr = processClick(event.button.x, event.button.y);
						}
						break;
				}
				break;
		}
	}
}

std::shared_ptr<Enemy> processClick(int x, int y)
{
	int right, left, top, bottom;
	right = left = top = bottom = 0;

	// checking if click coordinates are within bounding box coordinates of any of the enemies
	for (int i = 0; i < manager.enemies.size(); ++i) {
		if (manager.enemies[i] != nullptr) {
			right = manager.enemies[i]->sprite.rect.x + manager.enemies[i]->sprite.rect.w;
			left = manager.enemies[i]->sprite.rect.x;
			top = manager.enemies[i]->sprite.rect.y;
			bottom = manager.enemies[i]->sprite.rect.y + manager.enemies[i]->sprite.rect.h;

			if (x >= left && x <= right && y >= top && y <= bottom) {
				std::shared_ptr<Enemy> enemyPtr = manager.enemies[i];
				return enemyPtr;
			}
		}
	}

	// if no enemy clicked, nothing will be selected
	return std::shared_ptr<Enemy>();
}

void loadTextures(std::vector<std::vector<SDL_Texture*>>& spriteVec, const std::string& path, SDL_Renderer* const& renderer)		// populate textures vector
{
	std::string str;
	std::ifstream fileStream;
	std::string strNoNum;
	std::string spriteDirectory;

	fileStream.open(path.c_str());	// open the text file specifying the sprite filenames

	int spriteNum = 0;
	int indexNEW = 0;
	std::vector<SDL_Texture*> vec;
	for (int i = 0; i < 8; ++i) {
		spriteVec.push_back(vec);
	}

	if (fileStream.is_open()) {
		std::getline(fileStream, str);
		spriteDirectory = str;

		while (fileStream.good()) {
			std::getline(fileStream, str);
			std::stringstream strStream(str);

			char c;
			while (strStream >> c) {	// extract just the part of the filename that isn't a digit
				if (!isdigit(c)) {		// if the character isn't a digit, put it in the new string
					strNoNum.push_back(c);
				}
			}
			int pos = strNoNum.find('.');
			strNoNum = strNoNum.substr(0, pos);

			if (strNoNum == "north")
				indexNEW = 0;
			if (strNoNum == "northeast")
				indexNEW = 1;
			if (strNoNum == "east")
				indexNEW = 2;
			if (strNoNum == "southeast")
				indexNEW = 3;
			if (strNoNum == "south")
				indexNEW = 4;
			if (strNoNum == "southwest")
				indexNEW = 5;
			if (strNoNum == "west")
				indexNEW = 6;
			if (strNoNum == "northwest")
				indexNEW = 7;

			SDL_Surface* surface = SDL_LoadBMP((spriteDirectory + '/' + str).c_str());
			if (surface == NULL) {
				std::cout << "ERROR: Entity.h: couldn't load surface" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
			if (texture == NULL) {
				std::cout << "ERROR: Entity.h: couldn't create texture" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			spriteVec[indexNEW].push_back(texture);

			strNoNum.clear();
		}
	}

	if (spriteVec.size() != 8) {
		std::cout << "ERROR: textures not loaded in properly" << std::endl;
	}
}

void loadTileLayout(std::vector<Sprite>& tilesVec, const std::vector<SDL_Texture*> tileSpritesVec, const std::string& path)
{
	std::string str;
	std::ifstream fileStream;

	fileStream.open(path.c_str());

	if (fileStream.is_open()) {

		Sprite sprite;
		sprite.rect.x = 0;
		sprite.rect.y = 0;
		sprite.rect.w = 32;
		sprite.rect.h = 32;

		while (fileStream.good()) {
			std::getline(fileStream, str);
			std::stringstream strStream(str);

			char c;
			while (strStream >> c) {
				switch (c) {
				case '0':
					sprite.texture = tileSpritesVec[0];
					break;
				case '1':
					sprite.texture = tileSpritesVec[1];
					break;
				case '2':
					sprite.texture = tileSpritesVec[2];
					break;
				case '3':
					sprite.texture = tileSpritesVec[3];
					break;
				case '4':
					sprite.texture = tileSpritesVec[4];
					break;
				case '5':
					sprite.texture = tileSpritesVec[5];
					break;
				case '6':
					sprite.texture = tileSpritesVec[6];
					break;
				case '7':
					sprite.texture = tileSpritesVec[7];
					break;
				case '8':
					sprite.texture = tileSpritesVec[8];
					break;
				case '9':
					sprite.texture = tileSpritesVec[9];
					break;
				default:
					sprite.texture = tileSpritesVec[0];
					break;
				}
				tilesVec.push_back(sprite);

				sprite.rect.x += 32;
			}
			sprite.rect.x = 0;
			sprite.rect.y += 32;
		}
	}
}

void loadTileTextures(std::vector<SDL_Texture*>& tileSpritesVec, const std::string& path, SDL_Renderer* const& renderer)
{
	std::string str;
	std::ifstream fileStream;
	std::string spriteDirectory;

	SDL_Surface* surface;
	SDL_Texture* texture;

	fileStream.open(path.c_str());

	if (fileStream.is_open()) {
		std::getline(fileStream, str);
		spriteDirectory = str;

		while (fileStream.good()) {
			std::getline(fileStream, str);

			surface = SDL_LoadBMP((spriteDirectory + '/' + str).c_str());
			if (surface == NULL) {
				std::cout << "ERROR: loadTileTextures: couldn't load surface" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			texture = SDL_CreateTextureFromSurface(renderer, surface);
			if (texture == NULL) {
				std::cout << "ERROR: loadTileTextures: couldn't create texture" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}

			tileSpritesVec.push_back(texture);
		}
	}
}

void loadCooldownNumbers(std::vector<SDL_Texture*>& cooldownNumbersVec, const std::string& path, SDL_Renderer* const& renderer)
{
	std::string str;
	std::ifstream fileStream;
	std::string spriteDirectory;

	SDL_Surface* surface;
	SDL_Texture* texture;

	fileStream.open(path.c_str());

	if (fileStream.is_open()) {
		std::getline(fileStream, str);
		spriteDirectory = str;

		while (fileStream.good()) {
			std::getline(fileStream, str);

			surface = SDL_LoadBMP((spriteDirectory + '/' + str).c_str());
			if (surface == NULL) {
				std::cout << "ERROR: loadCooldownNumbers: couldn't load surface" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
			texture = SDL_CreateTextureFromSurface(renderer, surface);
			if (texture == NULL) {
				std::cout << "ERROR: loadCooldownNumbers: couldn't create texture" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}

			cooldownNumbersVec.push_back(texture);
		}
	}
}
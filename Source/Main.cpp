#include "SDL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include "Sprite.h"
#include <algorithm>
#include <vector>

// DONE: Enemies drop experience
// DONE: Text rendering
// DONE: Visible player health
// DONE: Clean up code
// DONE: Enemy health on collision with bullet
// DONE: Change number colors
// DONE: Respawning enemies

// DONE?: Fix Warnings
	// NOTES: I reloaded the project and the below errors went away. Nevermind, they came back?
		// 0) Primary Error: Severity	Code	Description	Project	File	Line	Suppression State
			// Warning	C26451	Arithmetic overflow : Using operator '*' on a 4 byte value and
			// then casting the result to a 8 byte value.Cast the value to the wider type before 
			// calling operator '*' to avoid overflow(io.2).Project_2_Name_TBD	C : \Projects\Project_2_Name_TBD\Contrib\stb_image.h	1014
		// 1) Unscoped enum warnings
		// 2) Cleaning up other warnings (Changed floats to doubles)
		// 3) Uninitialized struct warnings
// DONE: Make drawString less specific.
// DONE: Switch to using convertCameraSpaceNoWH for health bar
	// See ConvertCameraSpaceScreenWH function
// DONE: Creating a temporary interface for different enemies and weapons?
	// Start with keybinds (1-9 spawn weapons) etc.
// DONE: Outline the current HP bar with a black rectangle
// DONE: Change game font -> will also change the text scaling. Tried downloading/installing a tff. 
//			ascii bitmap font

// TODO: Consider add numbers to the health bar
// TODO: Improve weapon categorization and selections
// TODO: Display current weapon on screen
// TODO: Draw animations for a player model and an enemy model
// TODO: Add boss / Elite monsters that are scaled up or change in color and have health bars
// TODO: Fix enemy spawning?
// TODO: Exp collision / tracking NOTE: Give the exp a gravitational pull and have the player
//			have the stronger gravitational pull so it eventually collides with the player.
// TODO: Exp bar for character
// TODO: Enemy drawings / facing directions / animations
// TODO: Enemies spawning in more of waves / groups
// TODO: Adding player, enemy, weapon variety (Possibly through arrays or vectors)
// TODO: Develop a better map
// TODO: Items / experience that can be picked up by the player
// TODO: Menu
// TODO: Weapon selection
// TODO: Boss enemies (with a health bar)
// TODO: World scaler
// TODO: Impliment status effects
// TODO: IMGUI
// TODO: Statistic overlay
// TODO: *Refactor*

bool running = true;
bool up = false;
bool down = false;
bool left = false;
bool right = false;
double fireTime = 0;
const int DAMAGE_NUMBER_SIZE_E = 1;
const int DAMAGE_NUMBER_SIZE_P = 1;
const double DAMAGE_NUMBER_LIFETIME = .75;
const double EXPERIENCE_ORB_LIFETIME = 10;
int ENEMYSPAWNAMOUNT = 50;

int main(int argc, char** argv) {

	SDL_Window* window = nullptr;
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Undead Legends",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		RESOLUTION_X, RESOLUTION_Y, SDL_WINDOW_SHOWN);

	// -1 = default gpu
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// Maps Types
	Image mapA = loadImage(renderer, "Assets/Map_1.png");

	// Player Types
	Image characterDemon = loadImage(renderer, "Assets/Character_Demon_1.png");
	Image characterMaiden = loadImage(renderer, "Assets/Character_Maiden_1.png");
	Image characterGhoul = loadImage(renderer, "Assets/Character_Ghoul_5.png");
	Image characterVampireA = loadImage(renderer, "Assets/Character_Vampire_4.png");
	Image characterVampireB = loadImage(renderer, "Assets/Character_Vampire_5_ShortHair.png");
	Image characterFrankensteinCreation = loadImage(renderer, "Assets/Character_FrankensteinCreation_1.png");
	Image characterSkeleton = loadImage(renderer, "Assets/Character_Skeleton_1.png");
	
	// Enemy Types
	Image enemyBat = loadImage(renderer, "Assets/Enemy_VampireBat_1.png");
	Image enemyGargoyle = loadImage(renderer, "Assets/Enemy_Gargoyle_1.png");
	Image enemyType = {};
	enemyType = enemyBat;

	// Weapon Types
	Image weaponShadowOrb = loadImage(renderer, "Assets/Weapon_ShadowOrb_1.png");
	Image weaponSpike = loadImage(renderer, "Assets/Weapon_Spike_1.png");
	int weaponDamage = 25;
	Image currentWeaponSelected = weaponShadowOrb;
	double ATTACKSPEED = .50;
	double PROJECTILESPEED = 500;

	Image experienceOrbImage = loadImage(renderer, "Assets/Experience_Orb_1.png");
	// Image font = loadFont(renderer, "Assets/Font_1.png");
	// Image font = loadFont(renderer, "Assets/Font_2.png");
	Image font = loadFont(renderer, "Assets/Font_3.png");

	GameData gameData;
	gameData.renderer = renderer;

	// Map Tiles
	gameData.tileTypeArray[TILE_GRASS] = loadImage(renderer, "Assets/grassTile.png");
	gameData.tileTypeArray[TILE_DIRT] = loadImage(renderer, "Assets/dirtTile.png");
	gameData.tileTypeArray[TILE_ROCK] = loadImage(renderer, "Assets/rockTile.png");

	gameData.player = createCharacter(characterDemon, 100);
	gameData.player.position.x = RESOLUTION_X / 2;
	gameData.player.position.y = RESOLUTION_Y / 2;

	// Capping frame rate
	const int FPS = 60;
	// The max time between each frame
	const int frameDelay = 1000 / FPS;
	Uint32 frameStart;
	int frameTime;

	double lastFrameTime = getTime();

 	while (running) {
		frameStart = SDL_GetTicks();
		SDL_Event event = {};
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
				case SDLK_w:
					up = false;
					break;
				case SDLK_a:
					left = false;
					break;
				case SDLK_d:
					right = false;
					break;
				case SDLK_s:
					down = false;
					break;
				}
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_w:
					up = true;
					break;
				case SDLK_a:
					left = true;
					break;
				case SDLK_d:
					right = true;
					break;
				case SDLK_s:
					down = true;
					break;

				// Characters
				case SDLK_1:
					gameData.player = createCharacter(characterDemon, 100);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_2:
					gameData.player = createCharacter(characterGhoul, 100);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_3:
					gameData.player = createCharacter(characterMaiden, 100);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_4:
					gameData.player = createCharacter(characterVampireA, 100);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_5:
					gameData.player = createCharacter(characterVampireB, 100);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_6:
					gameData.player = createCharacter(characterFrankensteinCreation, 100);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_7:
					gameData.player = createCharacter(characterSkeleton, 100);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
					

				// Enemies
				case SDLK_z:
					destroyEnemies(gameData);
					enemyType = enemyBat;
					break;
				case SDLK_x:
					destroyEnemies(gameData);
					enemyType = enemyGargoyle;
					break;

				// Weapons
				case SDLK_r:
					currentWeaponSelected = weaponShadowOrb;
					weaponDamage = 50;
					ATTACKSPEED = .5;
					PROJECTILESPEED = 500;
					break;
				case SDLK_t:
					currentWeaponSelected = weaponSpike;
					weaponDamage = 25;
					ATTACKSPEED = .25;
					PROJECTILESPEED = 750;
					break;

				// Destroy
				case SDLK_BACKSPACE:
					destroyEnemies(gameData);
					break;

				}
				break;
			}
		}
	
		double currentTime = getTime();

		double deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		fireTime -= deltaTime;
		
		double speed = 2;

		if (left) {
			gameData.player.position.x -= speed;
		}
		if (right) {
			gameData.player.position.x += speed;
		}
		if (down) {
			gameData.player.position.y += speed;
		}
		if (up) {
			gameData.player.position.y -= speed;
		}

		// Spawn enemies
		for (int i = 0; i < (ENEMYSPAWNAMOUNT - gameData.enemies.size()); i++) {
			double range = randomFloat(RESOLUTION_X / 2, RESOLUTION_X);
			Vector enemyPosition = facingDirection(randomFloat(0, 360));

			enemyPosition.x *= range;
			enemyPosition.y *= range;

			enemyPosition.x += RESOLUTION_X / 2;
			enemyPosition.y += RESOLUTION_Y / 2;

			createEnemy(enemyType, enemyPosition, &gameData, 100, 2);
		}

		if (gameData.player.hp > 0) {
			for (int i = 0; i < gameData.enemies.size(); i++) {			
				updateEnemyPosition(&gameData.player, &gameData.enemies[i], deltaTime);
				for (int j = 0; j < gameData.enemies.size(); j++) {
					if (j == i) {
						continue;
					}
					else {
						double distanceBetween = distance(gameData.enemies[i].position, gameData.enemies[j].position);
						double radiusSum = gameData.enemies[i].radius + gameData.enemies[j].radius;
						if (distanceBetween < radiusSum) {
							Vector offset = gameData.enemies[j].position - gameData.enemies[i].position;
							offset *= 1 / distanceBetween;							
							offset *= radiusSum;
							gameData.enemies[j].position = offset + gameData.enemies[i].position;
						}
					}
				}
			}
		}

		// Update weapon position
		for (int i = 0; i < gameData.weapon.size(); i++) {
			updateEntityPosition(&gameData.weapon[i], deltaTime);
		}

		// Weapon spike firing at nearest enemies
		if (gameData.player.hp > 0) {
			if (fireTime <= 0) {
				int nearestEnemy = closestEnemy(gameData.player, &gameData);
				if (nearestEnemy >= 0) {
					Weapon currentWeaponInUse = createWeapon(currentWeaponSelected, weaponDamage);
					currentWeaponInUse.position = gameData.player.position;;
					Vector offset = {};
					// Calculates the vector from the player to the enemy (enemy <--- player)
					offset = gameData.enemies[nearestEnemy].position - gameData.player.position;
					currentWeaponInUse.velocity = normalize(offset);
					currentWeaponInUse.velocity *= PROJECTILESPEED;
					fireTime = ATTACKSPEED;
					currentWeaponInUse.angle = angleFromDirection(currentWeaponInUse.velocity);
					gameData.weapon.push_back(currentWeaponInUse);
				}
			}
		}

		/*
		// Weapon spike firing at nearest enemies
		if (gameData.player.hp > 0) {
			if (fireTime <= 0) {
				int nearestEnemy = closestEnemy(gameData.player, &gameData);
				if (nearestEnemy >= 0) {
					Weapon weaponSpike = createWeapon(weaponSpikeImage, 25);
					weaponSpike.position = gameData.player.position;;
					Vector offset = {};
					// Calculates the vector from the player to the enemy (enemy <--- player)
					offset = gameData.enemies[nearestEnemy].position - gameData.player.position;
					weaponSpike.velocity = normalize(offset);
					weaponSpike.velocity *= PROJECTILESPEED;
					fireTime = ATTACKSPEED;
					weaponSpike.angle = angleFromDirection(weaponSpike.velocity);
					gameData.weaponSpike.push_back(weaponSpike);
				}
			}
		}
		*/

		// Weapon collision with enemy
		for (int i = 0; i < gameData.weapon.size(); i++) {
			for (int j = 0; j < gameData.enemies.size(); j++) {
				double distanceBetween = distance(gameData.weapon[i].position, gameData.enemies[j].position);
				double radiusSum = gameData.weapon[i].radius + gameData.enemies[j].radius;
				if (distanceBetween < radiusSum) {
					if (gameData.enemies[j].hp > 0) {
						Vector numberVelocity = {};

						DamageNumber damageNumber = createDamageNumber(ENTITY_ENEMY, gameData.weapon[i].damage, gameData.enemies[j].position, 
							{ randomFloat(300, -300), randomFloat(-600, -300) }, DAMAGE_NUMBER_SIZE_E, DAMAGE_NUMBER_LIFETIME);
						gameData.damageNumbers.push_back(damageNumber);
						gameData.enemies[j].hp -= gameData.weapon[i].damage;
						gameData.weapon[i].lifeTime = 0;
						if (gameData.enemies[j].hp <= 0) {
							gameData.enemies[j].destroyed = true;
							ExperienceOrb experienceOrb = createExperienceOrb(gameData, experienceOrbImage, 
								gameData.enemies[j].position.x, gameData.enemies[j].position.y, EXPERIENCE_ORB_LIFETIME);
							gameData.experienceOrbs.push_back(experienceOrb);
						}

						// Knock back enemies
						gameData.enemies[j].velocity = gameData.weapon[i].velocity;
						double length = sqrt(gameData.enemies[j].velocity.x * gameData.enemies[j].velocity.x + gameData.enemies[j].velocity.y * gameData.enemies[j].velocity.y);
						gameData.enemies[j].velocity.x /= length;
						gameData.enemies[j].velocity.y /= length;
						gameData.enemies[j].velocity = gameData.enemies[j].velocity * 600;
						break;
					}
				}
			}
		}

		bool playerTakingDamage = false;
		
		for (int i = 0; i < gameData.enemies.size(); i++) {
			if (gameData.enemies[i].timeUntilDamage > 0) {
				gameData.enemies[i].timeUntilDamage -= deltaTime;
			}
		}

		// Player collision with enemy
		if (gameData.player.hp > 0) {
			for (int i = 0; i < gameData.enemies.size(); i++) {
				if (gameData.enemies[i].timeUntilDamage <= 0) {
					double distanceBetween = distancePlayer(gameData.player.position, gameData.enemies[i].position);
					double radiusSum = gameData.player.radius + gameData.enemies[i].radius;
					if (distanceBetween < radiusSum) {
						DamageNumber damageNumber = createDamageNumber(ENTITY_PLAYER, gameData.enemies[i].damage, gameData.player.position, { randomFloat(300, -300), randomFloat(-600, -300) }, DAMAGE_NUMBER_SIZE_P, DAMAGE_NUMBER_LIFETIME);
						gameData.damageNumbers.push_back(damageNumber);
						gameData.player.hp -= gameData.enemies[i].damage;
						playerTakingDamage = true;
						gameData.enemies[i].timeUntilDamage = .1;
					}
				}
			}
		}

		gameData.camera.position = gameData.player.position;

		// SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		// Clear what we are drawing to
		// Anything done before render clear gets erased
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, mapA.texture, NULL, NULL);

		double playerPositionX = gameData.player.position.x;
		double playerPositionY = gameData.player.position.y;

		for (int w = 0; w < (RESOLUTION_X / TILE_SIZE) + 2; w++) {
			for (int h = 0; h < (RESOLUTION_Y / TILE_SIZE) + 2; h++) {
				Tile tile = {};
				double offsetX = gameData.camera.position.x - (RESOLUTION_X / 2);
				double offsetY = gameData.camera.position.y - (RESOLUTION_Y / 2);
				tile.position.x = (w * (double) TILE_SIZE) + (floor(offsetX / TILE_SIZE) * TILE_SIZE);
				tile.position.y = (h * (double) TILE_SIZE) + (floor(offsetY / TILE_SIZE) * TILE_SIZE);
				tile.tileType = (TileType)abs((int)(tile.position.x / TILE_SIZE) % 1);
				drawTile(gameData, tile);
			}
		}

		// Draw player healthbar
		drawHealthBar(gameData, renderer);

		// Draw entities
		if (gameData.player.hp > 0) {
			drawEntity(gameData, gameData.player);
#if 0
			if (playerTakingDamage) {
				if (playerTakingDamage) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
					drawCircle(gameData, gameData.player.position, gameData.player.radius, -20);
					// drawHealthBar(gameData, gameData.player.position, 0);
				}
			}
#endif
		}


		for (int i = 0; i < gameData.enemies.size(); i++) {
			// Check to see if the boolean value is true when the enemy was created. If it was, draw it.
			double damagePercent = (double)gameData.enemies[i].hp / (double)gameData.enemies[i].maxHP;
			SDL_SetTextureColorMod(gameData.enemies[i].sprite.image.texture, (Uint8)255, (Uint8)(100 + (155 * damagePercent)), (Uint8)(100 + (155 * damagePercent)));
			drawEntity(gameData, gameData.enemies[i]);
			// SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			// drawCircle(renderer, enemy[i].sprite.position, enemy[i].radius);
		}

		for (int i = 0; i < gameData.weapon.size(); i++) {
			drawEntity(gameData, gameData.weapon[i]);
			gameData.weapon[i].lifeTime -= deltaTime;
			// SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			// drawCircle(renderer, weaponSpikeArray[i].sprite.position, weaponSpikeArray[i].radius);
		}

		for (int i = 0; i < gameData.damageNumbers.size(); i++) {
			drawDamageNumber(gameData, gameData.damageNumbers[i], &font, deltaTime);
			gameData.damageNumbers[i].lifeTime -= deltaTime;
		}

		for (int i = 0; i < gameData.experienceOrbs.size(); i++) {
			drawEntity(gameData, gameData.experienceOrbs[i]);
			gameData.experienceOrbs[i].lifeTime -= deltaTime;
		}

		// After renderPresent, the frame is over
		SDL_RenderPresent(renderer);

		// [] syntax for lambda
		// erase_if is going over every element and asking you if 
		// you want it to be deleted
		std::erase_if(gameData.enemies, [](const Enemy& enemy) {
			return enemy.destroyed;
			}
		);

		std::erase_if(gameData.weapon, [](const Weapon& weapon) {
			return weapon.lifeTime <= 0;
			}
		);

		std::erase_if(gameData.damageNumbers, [](const DamageNumber& damageNumber) {
			return damageNumber.lifeTime <= 0;
			}
		);

		std::erase_if(gameData.experienceOrbs, [](const ExperienceOrb& experienceOrb) {
			return experienceOrb.lifeTime <= 0;
			}
		);

		// Calculate the frame time (Home much time it's 
		// taken to get through the loop and update game
		// objects...etc.
		frameTime = SDL_GetTicks() - frameStart;

		// This will delay our frames
		if (frameDelay > frameTime)
		{
			SDL_Delay(frameDelay - frameTime);
		}

	}

	return 0;
}
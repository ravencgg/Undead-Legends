#include "SDL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include "Sprite.h"
#include <algorithm>
#include <vector>
#define STB_PERLIN_IMPLEMENTATION
#include "Perlin.h"

// DONE: Initialize the audio library. Function calls, point to a file, etc.

// With Chris
// TODO: Think about resource management. Look it up. You're going to want the same thing with sound. 
//			The sound in memory should only be loaded once. The same goes for images.
// TODO: Camera stuff (one to one pixels) and switching coordinate system

// CHRIS WIP
// DONE: Resource Management Audio. Unordered Map
// TODO: Unorded map for images
// TODO: Fix enemy spawn counter
// TODO: Develop a better map
// TODO: Death animations for enemies
// TODO: Look into TTF fonts - Slug font renderer
// TODO: Adding different levels
// TODO: Separate entity, staff, spell. Non of this belongs in sprite.h and .cpp
//			Gamedata doesn't belong in sprite either. Game.h and Game.cpp which 
//			has the game system and gamedata. Think about what system is doing what.
//			Staff system is responsible for spawning spells.
// TODO: Make sure every variable is initialized before using it (Default arguments or in a constructor)
// TODO: Change characters to high res? / 2 in the draw function
// TODO: Double check sprite and image w and h varaibles no redundant
// TODO: POWER UP: Increase number of projectiles fired
// TODO: Use perlin noise to draw the background layer. Draw everything else on top of it. 
//			you can then sample what the perlin noise generated and decide what to draw 
//			on top of it
// TODO: Make only one drawString function (w and h should be a parameter in a struct)
// TODO: Merge drawEnemyAnimated and drawCharacterAnimated - Only have 1 bool
// TODO: Offset enemy animations with a random number
// TODO: Review order of operations (ternary, etc)
// TODO: Exp bar for character / Exp tracking
// TODO: Consider add numbers to the health bar
// TODO: Display current weapon on screen
// TODO: Add boss / Elite monsters that are scaled up or change in color and have health bars
// TODO: Consecrated ground differeny status effects it applys? (Poison, frost, fire) - SDL_COLORMOD
// TODO: Fix enemy spawning?
// TODO: Experiement with fonts (Bitmap ASCII font generators)
// TODO: Enemies spawning in more of waves / groups
// TODO: Adding player, enemy, weapon variety (Possibly through arrays or vectors)
// TODO: Items / experience that can be picked up by the player
// TODO: Menu
// TODO: Weapon selection
// TODO: Boss enemies (with a health bar)
// TODO: World scaler
// TODO: Implement status effects
// TODO: IMGUI
// TODO: Statistic overlay
// TODO: *Refactor*

bool animated = true;
bool running = true;
bool up = false;
bool down = false;
bool left = false;
bool right = false;
bool facingRight = false;
double fireTime = 0;
double fireTimeAOE = 0;
int ENEMYSPAWNAMOUNT = 100;
bool consecratedGround = false;

double projectileDamageDelay = 1;

int totalEnemiesKilled = 0;

// No longer allocated on the stack
GameData gameData;

int main(int argc, char** argv) {
	REF(argc);
	REF(argv);

	SDL_Window* window = nullptr;
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Undead Legends",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		Constants::RESOLUTION_X, Constants::RESOLUTION_Y, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	// -1 = default gpu
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	gameData.renderer = renderer;

	// Maps Types
	Image mapA = loadImage(renderer, "Assets/Map_1.png");

	// Player Types
	Image characterDemon = loadImage(renderer, "Assets/Character_Demon_8.png");
	Image characterDemonAnimated = loadImage(renderer, "Assets/Character_Demon_2_Sprite_Sheet_2.png");
	Image characterMaiden = loadImage(renderer, "Assets/Character_Maiden_1.png");
	Image characterGhoul = loadImage(renderer, "Assets/Character_Ghoul_5.png");
	Image characterVampireA = loadImage(renderer, "Assets/Character_Vampire_4.png");
	Image characterVampireB = loadImage(renderer, "Assets/Character_Vampire_5_ShortHair.png");
	Image characterFrankensteinCreation = loadImage(renderer, "Assets/Character_FrankensteinCreation_1.png");
	Image characterSkeleton = loadImage(renderer, "Assets/Character_Skeleton_1.png");
	// "Assets/Character_Ice_Golem_1.png"
	Image characterDemonTest = loadImage(renderer, "Assets/Character_Demon_7.png");
	Image characterIceGolem = loadImage(renderer, "Assets/Character_Ice_Golem_3.png");
	
	// Enemy Types
	// Image enemyBat = loadImage(renderer, "Assets/Enemy_VampireBat_1.png");
	Image enemyBatAnimated = loadImage(renderer, "Assets/Enemy_VampireBat_1_SpriteSheet.png");
	// Image enemyGargoyle = loadImage(renderer, "Assets/Enemy_Gargoyle_1.png");
	// Image enemyGargoyle = loadImage(renderer, "Assets/Enemy_Gargoyle_1_Smaller.png");
	Image enemyGargoyleAnimated = loadImage(renderer, "Assets/Enemy_Gargoyle_1_Smaller_Sprite_Sheet.png"); 
	Image enemyFlower = loadImage(renderer, "Assets/Ememy_Flower_1.png");
	Image enemyType = {};
	enemyType = enemyBatAnimated;

	// Weapon Types
	// gameData.weaponType[WEAPON_SHADOW_ORB] = loadImage(renderer, "Assets/Weapon_ShadowOrb_1.png");
	// gameData.weaponType[WEAPON_SPIKE] = loadImage(renderer, "Assets/Weapon_Spike_1.png");
	Image weaponSpike = loadImage(renderer, "Assets/Weapon_Spike_2.png");
	Image weaponShadowOrb = loadImage(renderer, "Assets/Weapon_ShadowOrb_1.png");
	Image weaponConsecratedGround = loadImage(renderer, "Assets/Weapon_Consecrated_Ground_1.png");
	Image weaponFireball = loadImage(renderer, "Assets/Weapon_Fireball_1.png");
	// Assets/Weapon_Fireball_AOE_1.png
	Image weaponFireballAOE = loadImage(renderer, "Assets/Weapon_Fireball_AOE_6.png");
	Image currentWeaponSelected = weaponSpike;

	Image experienceOrbImage = loadImage(renderer, "Assets/Experience_Orb_1.png");
	// Image font = loadFont(renderer, "Assets/Font_1.png");
	// Image font = loadFont(renderer, "Assets/Font_2.png");
	Image font = loadFont(renderer, "Assets/Font_3.png");

	// Map Tiles
	gameData.tileTypeArray[TILE_GRASS] = loadImage(renderer, "Assets/grassTile.png");
	gameData.tileTypeArray[TILE_DIRT] = loadImage(renderer, "Assets/dirtTile.png");
	gameData.tileTypeArray[TILE_ROCK] = loadImage(renderer, "Assets/rockTile.png");

	createCharacter(gameData, characterIceGolem, 100, false, 300, 1);
	
	gameData.player.position.x = Constants::RESOLUTION_X / 2;
	gameData.player.position.y = Constants::RESOLUTION_Y / 2;


	// Capping frame rate
	const int FPS = 60;
	// The max time between each frame
	const int frameDelay = 1000 / FPS;
	Uint32 frameStart;
	int frameTime;

	double lastFrameTime = getTime();

	// Initialize soloud
	gameData.soloud.init();

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
					gameData.player.animated = false;
					break;
				case SDLK_d:
					right = false;
					gameData.player.animated = false;
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
					facingRight = false;
					gameData.player.animated = true;
					break;
				case SDLK_d:
					right = true;
					facingRight = true;
					gameData.player.animated = true;
					break;
				case SDLK_s:
					down = true;
					break;
				case SDLK_f:
					break;

					// Characters
				case SDLK_1:
					createCharacter(gameData, characterDemonAnimated, 100, true, 300, 3);
					break;
				case SDLK_2:
					createCharacter(gameData, characterGhoul, 100, false, 300, 1);
					break;
				case SDLK_3:
					createCharacter(gameData, characterMaiden, 100, false, 300, 1);
					break;
				case SDLK_4:
					createCharacter(gameData, characterVampireA, 100, false, 300, 1);
					break;
				case SDLK_5:
					createCharacter(gameData, characterVampireB, 100, false, 300, 1);
					break;
				case SDLK_6:
					createCharacter(gameData, characterFrankensteinCreation, 100, false, 300, 1);
					break;
				case SDLK_7:
					createCharacter(gameData, characterSkeleton, 100, false, 300, 1);
					break;
				case SDLK_8:
					createCharacter(gameData, characterDemonTest, 100, false, 300, 1);
					break;
				case SDLK_9:
					createCharacter(gameData, characterIceGolem, 100, false, 300, 1);
					break;

					// Enemies
				case SDLK_z:
					destroyEnemies(gameData);
					enemyType = enemyBatAnimated;
					animated = true;
					break;
				case SDLK_x:
					destroyEnemies(gameData);
					enemyType = enemyGargoyleAnimated;
					animated = true;
					break;
				case SDLK_c:
					destroyEnemies(gameData);
					enemyType = enemyFlower;
					animated = false;
					break;

				// Weapons
				case SDLK_r:
					gameData.player.newStaff(new SpikeStaff(gameData, &gameData.player));
					break;
				case SDLK_t:
					gameData.player.newStaff(new ShadowOrbStaff(gameData, &gameData.player));
					break;
				case SDLK_y:
					gameData.player.newStaff(new ConsecratedGroundStaff(gameData, &gameData.player));
					break;
				case SDLK_u:
					gameData.player.newStaff(new FireballStaff(gameData, &gameData.player));
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
		// Shouldn't be below this frame time realistically
		if (deltaTime >= 0.10) {
			deltaTime = 0.10;
		}
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
			double range = randomFloat(Constants::RESOLUTION_X / 2, Constants::RESOLUTION_X);
			Vector enemyPosition = facingDirection(randomFloat(0, 360));

			enemyPosition.x *= range;
			enemyPosition.y *= range;

			enemyPosition.x += Constants::RESOLUTION_X / 2;
			enemyPosition.y += Constants::RESOLUTION_Y / 2;

			if (animated) {
				createEnemy(enemyType, enemyPosition, &gameData, 100, 2, true, 300, 2);
			}
			else {
				createEnemy(enemyType, enemyPosition, &gameData, 100, 2, false, 300, 0);
			}
		}

		// Update Enemy Position
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

		// Update Experience Orb position
		for (int i = 0; i < gameData.experienceOrbs.size(); i++) {
			updateExperienceOrbPosition(gameData, &gameData.experienceOrbs[i], Constants::EXPERIENCE_ORB_SPEED, deltaTime);
		}

		bool playerTakingDamage = false;

		for (int i = 0; i < gameData.enemies.size(); i++) {
			if (gameData.enemies[i].timeUntilDamageTaken > 0) {
				gameData.enemies[i].timeUntilDamageTaken -= deltaTime;
			}
			if (gameData.enemies[i].timeUntilDamageDealt > 0) {
				gameData.enemies[i].timeUntilDamageDealt -= deltaTime;
			}
		}

		for (Spell* spell : gameData.spells) {
			spell->updatePosition(deltaTime);
		}
		for (int i = 0; i < gameData.spells.size(); i++) {
			for (int j = 0; j < gameData.enemies.size(); j++) {
				if (gameData.spells[i]->canDamage(&gameData.enemies[j])) {
					gameData.spells[i]->collision(&gameData.enemies[j]);
				}
			}
		}

		// Create Projectile and fire at nearest enemy
		if (gameData.player.hp > 0) {
			if (gameData.enemies.size() >= 0) {
				if (gameData.player.staff) {
					Spell* spell = gameData.player.staff->cast(deltaTime);
					if (spell) {
						gameData.spells.push_back(spell);
					}
				}
			}
		}

		// Player collision with enemy
		if (gameData.player.hp > 0) {
			for (int i = 0; i < gameData.enemies.size(); i++) {
				if (gameData.enemies[i].timeUntilDamageDealt <= 0) {
					double distanceBetween = distancePlayer(gameData.player.position, gameData.enemies[i].position);
					double radiusSum = gameData.player.radius + gameData.enemies[i].radius;
					if (distanceBetween < radiusSum) {
						DamageNumber damageNumber = createDamageNumber(ENTITY_PLAYER, gameData.enemies[i].damage, gameData.player.position,
							{ randomFloat(300, -300), randomFloat(-600, -300) }, Constants::DAMAGE_NUMBER_SIZE_P, Constants::DAMAGE_NUMBER_LIFETIME);
						gameData.damageNumbers.push_back(damageNumber);
						gameData.player.hp -= gameData.enemies[i].damage;
						playerTakingDamage = true;
						gameData.enemies[i].timeUntilDamageDealt = .1;
					}
				}
			}
		}

		// Player collision with experience orb
		if (gameData.player.hp > 0) {
			for (int i = 0; i < gameData.experienceOrbs.size(); i++) {
				double distanceBetween = distancePlayer(gameData.player.position, gameData.experienceOrbs[i].position);
				double radiusSum = (gameData.player.radius / 2) + gameData.experienceOrbs[i].radius;
				if (distanceBetween < radiusSum) {
					gameData.experienceOrbs[i].lifeTime = 0;
					gameData.player.experience += Constants::EXPERIENCE_ORB_EXPERIENCE;
					if (gameData.player.experience == 1000) {
						gameData.player.level += 1;
						gameData.player.experience = 0;
					}
				}
			}
		}

		gameData.camera.position = gameData.player.position;

		// Clear what we are drawing to
		// Anything done before render clear gets erased
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, mapA.texture, NULL, NULL);

		for (int w = 0; w < (Constants::RESOLUTION_X / Constants::TILE_SIZE) + 2; w++) {
			for (int h = 0; h < (Constants::RESOLUTION_Y / Constants::TILE_SIZE) + 2; h++) {
				Tile tile = {};
				double offsetX = gameData.camera.position.x - (Constants::RESOLUTION_X / 2);
				double offsetY = gameData.camera.position.y - (Constants::RESOLUTION_Y / 2);
				tile.position.x = (w * (double)Constants::TILE_SIZE) + (floor(offsetX / Constants::TILE_SIZE) * Constants::TILE_SIZE);
				tile.position.y = (h * (double)Constants::TILE_SIZE) + (floor(offsetY / Constants::TILE_SIZE) * Constants::TILE_SIZE);
				float perlin = stb_perlin_noise3((float)tile.position.x / 256, (float)tile.position.y / 256, 0, 0, 0, 0);
				drawTile(gameData, tile, perlin);
			}
		}

		drawHealthBar(gameData, renderer);

		Color textColor = {};
		textColor.r = 255;
		textColor.g = 255;
		textColor.b = 255;
		// Kill tracker
		drawString(textColor, gameData.renderer, &font, 1, std::string("Kills: "), 10, 10);
		drawString(textColor, gameData.renderer, &font, 1, std::to_string(totalEnemiesKilled), 100, 10);

		// DRAW ENTITIES

		// Draw AOE
		for (int i = 0; i < gameData.aoe.size(); i++) {
			drawEntity(gameData, gameData.aoe[i]);
			gameData.aoe[i].lifeTime -= deltaTime;
			// SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			// drawCircle(renderer, weaponSpikeArray[i].sprite.position, weaponSpikeArray[i].radius);
		}

		// Draw Spells
		for (Spell* spell : gameData.spells) {
			if (spell->getLifeTime() > 0) {
				spell->draw(gameData);
			}
		}

		// Draw Player
		if (gameData.player.hp > 0) {
			if (gameData.player.animated) {
				drawEntityAnimated(gameData, gameData.player, facingRight);
			}
			if (!gameData.player.animated) {
				drawCharacterIdle(gameData, gameData.player, facingRight);
			}
			// WIP - Radius for picking up experience orbs
			// SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
			// drawCircle(gameData, gameData.player.position, gameData.player.radius * EXPERIENCE_RADIUS, 0);
		}

		// Draw Enemies
		for (int i = 0; i < gameData.enemies.size(); i++) {
			// Check to see if the boolean value is true when the enemy was created. If it was, draw it.
			double damagePercent = (double)gameData.enemies[i].hp / (double)gameData.enemies[i].maxHP;
			SDL_SetTextureColorMod(gameData.enemies[i].sprite.image.texture, (Uint8)255, (Uint8)(100 + (155 * damagePercent)), (Uint8)(100 + (155 * damagePercent)));
			if (gameData.enemies[i].animated) {
				drawEntityAnimated(gameData, gameData.enemies[i], gameData.enemies[i].position.x < gameData.player.position.x);
			}
			if (!gameData.enemies[i].animated) {
				drawEntity(gameData, gameData.enemies[i]);
			}
			// SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			// drawCircle(renderer, enemy[i].sprite.position, enemy[i].radius);
		}

		drawExperienceBar(gameData, renderer);

		// Experience tracker
		std::string experienceTracker = std::to_string(gameData.player.experience);
		experienceTracker += std::string("/");
		experienceTracker += std::to_string(gameData.player.levelUp);
		int numberOfPixelsW = (int)experienceTracker.size() * 14;
		drawString(textColor, renderer, &font, 1, experienceTracker, (Constants::RESOLUTION_X / 2) - (numberOfPixelsW / 2), 865);

		// Level tracker
		std::string levelTracker = std::string("Level: ");
		levelTracker += std::to_string(gameData.player.level);
		int pixelWidthLeveltracker = (int)levelTracker.size() * 14;
		drawString(textColor, renderer, &font, 1, levelTracker, (Constants::RESOLUTION_X / 2) - (pixelWidthLeveltracker / 2), 835);

		// Draw Damage Numbers
		for (int i = 0; i < gameData.damageNumbers.size(); i++) {
			drawDamageNumber(gameData, gameData.damageNumbers[i], &font, deltaTime);
			gameData.damageNumbers[i].lifeTime -= deltaTime;
		}

		// Draw Experience Orbs
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

		std::erase_if(gameData.spells, [](Spell* spell) {
			return spell->getLifeTime() <= 0;
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

	// Clean up audio engine
	gameData.soloud.deinit();

	return 0;
}

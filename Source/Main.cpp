#include "SDL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <algorithm>
#include <vector>
#define STB_PERLIN_IMPLEMENTATION
#include "Perlin.h"

#include "Constants.h"
#include "Tools.h"
#include "Staff.h"
#include "Entity.h"
#include "Game.h"

// DONE: Death animations finished
// DONE: Damage numbers now delay death animations to make it feel more responsive.
// DONE: Hierarchy for files
// DONE: More sounds
// DONE: Enchanted Sword
// DONE: Memory leak (objects created with new need to be deleted) - Change back to value types.
//			gamedata is the highest up, so it can see more.

// TODO: Add a indicator over the head of enemies that the enchanted sword is targeting, change the 
//			speed of the sword when it is turning
// TODO: Game states (levels & menus), and a character selection window
// TODO: Increase the pixel ratios of all characters, weapons, and enemies
// TODO: Experience level up window
// TODO: Improve the damage number implementation (Offset each number by some or add a small knockback
//			per damage number applied)
// TODO: Code clean up and revision of names

// Chris WIP: Shadows for sprites (For free)
// Chris WIP: Camera stuff (one to one pixels) and switching coordinate system

// TODO: Known bug: When enemies are in radius of consecrated ground when I activate it, they don't take damage.
// TODO: Death animations
// TODO: Better memory allocation for damage numbers (Unordered maps?) A weapon doing 50% 
//			bonus damage as an addition number for example
// TODO: Change name 'entityType' to something more suitable.
// TODO: Clean up DamageNumber struct, createDamageNumber(), and draw damageNumber()
// TODO: Unorded map for images
// TODO: Develop a better map
// TODO: Look into TTF fonts - Slug font renderer
// TODO: Adding different levels
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
// TODO: Consecrated ground different status effects it applys? (Poison, frost, fire) - SDL_COLORMOD
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

static int totalEnemiesKilled = 0;

bool animated = true;
bool running = true;
bool up = false;
bool down = false;
bool left = false;
bool right = false;
bool facingRight = false;
double fireTime = 0;
int ENEMYSPAWNAMOUNT = 50;

// No longer allocated on the stack
GameData gameData = {};

int main(int argc, char** argv) {
	REF(argc);
	REF(argv);

	SDL_Window* window = nullptr;
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Undead Legends",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		Constants::RESOLUTION_X, Constants::RESOLUTION_Y, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	// -1 = default gpu
    R_CreateRenderer(window);

	// Maps Types
	Image mapA = loadImage("Assets/Map_1.png", 1);

	// Player Types
	Image characterSizeTest = loadImage("Assets/Character_Size_Test_1.png", 1);
	Image characterDemon = loadImage("Assets/Character_Demon_8.png", 1);
	Image characterDemonAnimated = loadImage("Assets/Character_Demon_2_Sprite_Sheet_2.png", 1);
	Image characterMaiden = loadImage("Assets/Character_Maiden_1.png", 1);
	Image characterGhoul = loadImage("Assets/Character_Ghoul_5.png", 1);
	Image characterVampireA = loadImage("Assets/Character_Vampire_4.png", 1);
	Image characterVampireB = loadImage("Assets/Character_Vampire_5_ShortHair.png", 1);
	Image characterFrankensteinCreation = loadImage("Assets/Character_FrankensteinCreation_1.png", 1);
	Image characterSkeleton = loadImage("Assets/Character_Skeleton_1.png", 1);
	// "Assets/Character_Ice_Golem_1.png"
	Image characterDemonTest = loadImage("Assets/Character_Demon_7.png", 1);

	Image characterIceGolem = loadImage("Assets/Character_Ice_Golem_3.png", 1);
	
	// Enemy Types
	// Image enemyBat = loadImage("Assets/Enemy_VampireBat_1.png");
	Image enemyBatAnimated = loadImage("Assets/Enemy_VampireBat_1_SpriteSheet.png", 2);
	// Image enemyGargoyle = loadImage("Assets/Enemy_Gargoyle_1.png");
	// Image enemyGargoyle = loadImage("Assets/Enemy_Gargoyle_1_Smaller.png");
	Image enemyGargoyleAnimated = loadImage("Assets/Enemy_Gargoyle_1_Smaller_Sprite_Sheet.png", 2); 
	Image enemyFlower = loadImage("Assets/Ememy_Flower_1.png", 1);
	Image enemyType = {};
	enemyType = enemyBatAnimated;

	Image batDeathSpriteSheet = loadImage("Assets/Enemy_VampireBat_Death_Sprite-Sheet.png", 5);

	Image experienceOrbImage = loadImage("Assets/Experience_Orb_1.png", 1);
	
	int fontSize = 1;
	// Image font = loadFont("Assets/Font_1.png");
	// Image font = loadFont("Assets/Font_2.png");
	Image font = loadFont("Assets/Font_3.png");

	// Map Tiles
	gameData.tileTypeArray[TILE_GRASS] = loadImage("Assets/grassTile.png", 1);
	gameData.tileTypeArray[TILE_DIRT] = loadImage("Assets/dirtTile.png", 1);
	gameData.tileTypeArray[TILE_ROCK] = loadImage("Assets/rockTile.png", 1);

	createCharacter(gameData, characterDemon, 100, false, 300);
	
	gameData.player->position.x = Constants::RESOLUTION_X / 2;
	gameData.player->position.y = Constants::RESOLUTION_Y / 2;

	// Capping frame rate
	const int FPS = 144;
	// The max time between each frame
	const int frameDelay = 1000 / FPS;
	Uint32 frameStart;
	//int frameTime;

	double lastFrameTime = getTime();

	// Initialize soloud
	gameData.soloud.init();

	while (running) {
		frameStart = SDL_GetTicks();
		SDL_Event event = {};
		// Only need pollevent in one place of your code
		// Tied directly to pollevent. Input is 1 to 1.
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
					gameData.player->animated = false;
					break;
				case SDLK_d:
					right = false;
					gameData.player->animated = false;
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
					gameData.player->animated = true;
					break;
				case SDLK_d:
					right = true;
					facingRight = true;
					gameData.player->animated = true;
					break;
				case SDLK_s:
					down = true;
					break;
				case SDLK_f:
					break;

				// Characters
				case SDLK_1:
					createCharacter(gameData, characterSizeTest, 100, false, 300);
					break;
				case SDLK_2:
					createCharacter(gameData, characterIceGolem, 100, false, 300);
					break;
				case SDLK_3:
					createCharacter(gameData, characterDemon, 100, false, 300);
					break;
				case SDLK_4:
					createCharacter(gameData, characterVampireA, 100, false, 300);
					break;
				case SDLK_5:
					createCharacter(gameData, characterVampireB, 100, false, 300);
					break;
				case SDLK_6:
					createCharacter(gameData, characterFrankensteinCreation, 100, false, 300);
					break;
				case SDLK_7:
					createCharacter(gameData, characterSkeleton, 100, false, 300);
					break;
				case SDLK_8:
					createCharacter(gameData, characterMaiden, 100, false, 300);
					break;
				case SDLK_9:
					createCharacter(gameData, characterGhoul, 100, false, 300);
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
					gameData.player->newStaff(new SpikeStaff(gameData, gameData.player));
					break;
				case SDLK_t:
					gameData.player->newStaff(new ShadowOrbStaff(gameData, gameData.player));
					break;
				case SDLK_y:
					gameData.player->newStaff(new ConsecratedGroundStaff(gameData, gameData.player));
					break;
				case SDLK_u:
					gameData.player->newStaff(new FireballStaff(gameData, gameData.player));
					break;
				case SDLK_i:
					gameData.player->newStaff(new MagicSwordStaff(gameData, gameData.player));
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

		double speed = 200.0 * deltaTime;

		if (left) {
			gameData.player->position.x -= speed;
		}
		if (right) {
			gameData.player->position.x += speed;
		}
		if (down) {
			gameData.player->position.y += speed;
		}
		if (up) {
			gameData.player->position.y -= speed;
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
				createEnemy(enemyType, enemyPosition, &gameData, 100, 2, true, 300);
			}
			else {
				createEnemy(enemyType, enemyPosition, &gameData, 100, 2, false, 300);
			}
			/*
			* // Degugging
			Vector enemyPosition = gameData.player->position;
			enemyPosition.y += 300;
			createEnemy(enemyType, enemyPosition, &gameData, 100, 2, true, 300, 2);
			*/
		}

		// Update Enemy Position
		if (gameData.player->hp > 0) {
			for (int i = 0; i < gameData.enemies.size(); i++) {
				updateEnemyPosition(gameData.player, &gameData.enemies[i], deltaTime);
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
			double distanceBetween = distance(gameData.player->position, gameData.experienceOrbs[i].position);
			double radiusSum = gameData.player->pickUpRadius + gameData.experienceOrbs[i].radius;
			if (distanceBetween < radiusSum) {
				updateExperienceOrbPosition(gameData, &gameData.experienceOrbs[i], Constants::EXPERIENCE_ORB_SPEED, deltaTime);
			}
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
		if (gameData.player->hp > 0) {
			if (gameData.enemies.size() >= 0) {
				if (gameData.player->staff) {
					Spell* spell = gameData.player->staff->cast(deltaTime);
					if (spell) {
						gameData.spells.push_back(spell);
					}
				}
			}
		}

		// Player collision with enemy
		if (gameData.player->hp > 0) {
			for (int i = 0; i < gameData.enemies.size(); i++) {
				if (gameData.enemies[i].timeUntilDamageDealt <= 0) {
					double distanceBetween = distancePlayer(gameData.player->position, gameData.enemies[i].position);
					double radiusSum = gameData.player->radius + gameData.enemies[i].radius;
					if (distanceBetween < radiusSum) {
						DamageNumber damageNumber = createDamageNumber(ENTITY_PLAYER, DN_FALLING, 0.0, gameData.enemies[i].damage, gameData.player->position,
							{ randomFloat(300, -300), randomFloat(-600, -300) }, Constants::DAMAGE_NUMBER_SIZE_P, Constants::DAMAGE_NUMBER_LIFETIME);
						gameData.damageNumbers.push_back(damageNumber);
						gameData.player->hp -= gameData.enemies[i].damage;
						playerTakingDamage = true;
						gameData.enemies[i].timeUntilDamageDealt = .1;
					}
				}
			}
		}

		// Player collision with experience orb
		if (gameData.player->hp > 0) {
			for (int i = 0; i < gameData.experienceOrbs.size(); i++) {
				double distanceBetween = distancePlayer(gameData.player->position, gameData.experienceOrbs[i].position);
				double radiusSum = (gameData.player->radius / 2) + gameData.experienceOrbs[i].radius;
				if (distanceBetween < radiusSum) {
					gameData.experienceOrbs[i].lifeTime = 0;
					gameData.player->experience += Constants::EXPERIENCE_ORB_EXPERIENCE;
					if (gameData.player->experience == 1000) {
						gameData.player->level += 1;
						gameData.player->experience = 0;
					}
				}
			}
		}

		gameData.camera.position = {};// gameData.player->position;
		gameData.camera.size = { Constants::RESOLUTION_X, Constants::RESOLUTION_Y };

        R_View view = {
            gameData.camera.position.x,
            gameData.camera.position.y,
            gameData.camera.size.x,
            gameData.camera.size.y
        };
		R_BeginFrame(view);
		R_BeginWorldDrawing();
		R_RenderClear();
		R_RenderCopy(mapA.texture, NULL, NULL);

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

		drawHealthBar(gameData);

		Color textColor = {};
		textColor.r = 255;
		textColor.g = 255;
		textColor.b = 255;

		// ***Draw Entities***

		// Draw Spells
		for (Spell* spell : gameData.spells) {
			if (spell->getLifeTime() > 0) {
				spell->draw(gameData);
			}
		}

		// Draw Player
		if (gameData.player->hp > 0) {
			if (gameData.player->animated) {
				drawEntityAnimated(gameData, gameData.player, facingRight);
			}
			if (!gameData.player->animated) {
				drawCharacterIdle(gameData, gameData.player, facingRight);
			}
		}

		// Draw Enemies
		for (int i = 0; i < gameData.enemies.size(); i++) {
			if (!gameData.enemies[i].destroyed) {
				// Check to see if the boolean value is true when the enemy was created. If it was, draw it.
				double damagePercent = (double)gameData.enemies[i].hp / (double)gameData.enemies[i].maxHP;
				R_SetTextureColorMod(gameData.enemies[i].sprite.image.texture, (Uint8)255, (Uint8)(100 + (155 * damagePercent)), (Uint8)(100 + (155 * damagePercent)));
				if (gameData.enemies[i].animated) {
					drawEntityAnimated(gameData, &gameData.enemies[i], gameData.enemies[i].position.x < gameData.player->position.x);
				}
				if (!gameData.enemies[i].animated) {
					drawEntity(gameData, &gameData.enemies[i]);
				}
				// SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				// drawCircle(renderer, enemy[i].sprite.position, enemy[i].radius);
			}
			else {
				createDeathAnimation(batDeathSpriteSheet, gameData.enemies[i].position, gameData, gameData.enemies[i].timesHit);
			}
		}

		// Death animations
		for (int i = 0; i < gameData.deathAnimations.size(); i++) {
			drawDeathAnimation(gameData, &gameData.deathAnimations[i], gameData.deathAnimations[i].position.x < gameData.player->position.x);
			gameData.deathAnimations[i].timeUntilNextFrame -= deltaTime;
		}

		drawExperienceBar(gameData);

		// Draw Damage Numbers
		for (int i = 0; i < gameData.damageNumbers.size(); i++) {
			if (gameData.damageNumbers[i].lifeTime > 0 && gameData.damageNumbers[i].numberDelay <= 0) {
				drawDamageNumber(gameData, gameData.damageNumbers[i], &font, deltaTime);
				if (gameData.damageNumbers[i].entityType == ENTITY_ENEMY && gameData.damageNumbers[i].soundPlayed == false) {
					playEnemyHitSound(gameData);
					gameData.damageNumbers[i].soundPlayed = true;
				}
				gameData.damageNumbers[i].lifeTime -= deltaTime;
			}
			gameData.damageNumbers[i].numberDelay -= deltaTime;
		}

		// Draw Experience Orbs
		for (int i = 0; i < gameData.experienceOrbs.size(); i++) {
			drawEntity(gameData, &gameData.experienceOrbs[i]);
			gameData.experienceOrbs[i].lifeTime -= deltaTime;
		}

		// ***Player Interface***
		// Experience tracker

		R_BeginUIDrawing();
		std::string experienceTracker = std::to_string(gameData.player->experience);
		experienceTracker += std::string("/");
		experienceTracker += std::to_string(gameData.player->levelUp);
		int numberOfPixelsW = (int)experienceTracker.size() * 14;
		drawString(textColor, &font, fontSize, experienceTracker, (Constants::RESOLUTION_X / 2) - (numberOfPixelsW / 2), 865);

		// Level tracker
		std::string levelTracker = std::string("Level: ");
		levelTracker += std::to_string(gameData.player->level);
		int pixelWidthLeveltracker = (int)levelTracker.size() * 14;
		drawString(textColor, &font, fontSize, levelTracker, (Constants::RESOLUTION_X / 2) - (pixelWidthLeveltracker / 2), 835);

		// Kill tracker
		drawString(textColor, &font, fontSize, std::string("Kills: "), 10, 10);
		drawString(textColor, &font, fontSize, std::to_string(totalEnemiesKilled), 100, 10);

		// After renderPresent, the frame is over
		R_RenderPresent();

		// [] syntax for lambda
		// erase_if is going over every element and asking you if 
		// you want it to be deleted

		std::erase_if(gameData.enemies, [](const Enemy& enemy) {
			if (enemy.destroyed) {
				totalEnemiesKilled += 1;
			}
			return enemy.destroyed;
			}
		);

		std::erase_if(gameData.deathAnimations, [](const DeathAnimation& deathAnimation) {
			return deathAnimation.currentFrame >= deathAnimation.sprite.image.num_frames;
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

#if 0
		// Calculate the frame time (Home much time it's 
		// taken to get through the loop and update game
		// objects...etc.
		frameTime = SDL_GetTicks() - frameStart;

		// This will delay our frames
		if (frameDelay > frameTime)
		{
			SDL_Delay(frameDelay - frameTime);
		}
#endif

	}

	// Clean up audio engine
	gameData.soloud.deinit();

	return 0;
}

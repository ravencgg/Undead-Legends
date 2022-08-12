#include "SDL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include "Sprite.h"
#include <algorithm>
#include <vector>
#define STB_PERLIN_IMPLEMENTATION
#include "Perlin.h"

// DONE: Enemies drop experience
// DONE: Text rendering
// DONE: Visible player health
// DONE: Clean up code
// DONE: Enemy health on collision with bullet
// DONE: Change number colors
// DONE: Respawning enemies
// DONE: Fix Warnings
// DONE: Switch to using convertCameraSpaceNoWH for health bar
	// See ConvertCameraSpaceScreenWH function
// DONE: Creating a temporary interface/key bindings for different enemies and weapons?
	// Start with keybinds (1-9 spawn weapons) etc.
// DONE: Outline the current HP bar with a black rectangle
// DONE: Change game font -> will also change the text scaling. Tried downloading/installing a tff. 
//			ascii bitmap font
// DONE: Impliment consecrated ground
// DONE: Exp collision	
// DONE: Develop more sprites / animations
// DONE: Draw animations for a player model and an enemy model
// DONE: Enemy drawings / facing directions / animations

// TODO: Look into TTF fonts - Slug font renderer
// TODO: Drawing a string with numbers on screen implimentation
// TODO: Develop a better map
// TODO: Add sound and music? Where to find these? Commission people? Free sites?
// TODO: Improve weapon categorization and selections

// TODO: Virtual function for weapons (Bullet class, missle class)
// TODO: Use perlin noise to draw the background layer. Draw everything else on top of it. 
//			you can then sample what the perlin noise generated and decide what to draw 
//			on top of it
// TODO: Set position in create character function
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
// TODO: Impliment status effects
// TODO: IMGUI
// TODO: Statistic overlay
// TODO: *Refactor*

bool running = true;
bool up = false;
bool down = false;
bool left = false;
bool right = false;
bool facingRight = false;
double fireTime = 0;
const int DAMAGE_NUMBER_SIZE_E = 1;
const int DAMAGE_NUMBER_SIZE_P = 1;
const double DAMAGE_NUMBER_LIFETIME = .75;
const double EXPERIENCE_ORB_LIFETIME = 10;
const double EXPERIENCE_RADIUS = 6;
const double EXPERIENCE_ORB_SPEED = 250;
int experienceOrbExperience = 50;
int ENEMYSPAWNAMOUNT = 50;
bool consecratedGround = false;
int totalEnemiesKilled = 0;
bool fireball = false;

// Spike
int weaponDamageSpike = 50;
double attackSpeedSpike = 1;
double projectileSpeedSpike = 300;

// Shadow Orb
int weaponDamageShadowOrb = 100;
double attackSpeedShadowOrb = 2;
double projectileSpeedShadowOrb = 150;

// Consecrated Ground
int weaponDamageConsecratedGround = 100;
double damageDelayConsecratedGround = 1;

// Fireball
int weaponDamageFireball = 100;
double attackSpeedFireball = 5;
double projectileSpeedFireball = 100;
int weaponDamageFireballAOE = 20;
int weaponDurationFireballAOE = 5;

int main(int argc, char** argv) {

	SDL_Window* window = nullptr;
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Undead Legends",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		RESOLUTION_X, RESOLUTION_Y, SDL_WINDOW_SHOWN);

	// -1 = default gpu
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	GameData gameData;
	gameData.renderer = renderer;

	// Maps Types
	Image mapA = loadImage(renderer, "Assets/Map_1.png");

	// Player Types
	Image characterDemon = loadImage(renderer, "Assets/Character_Demon_2.png");
	Image characterDemonAnimated = loadImage(renderer, "Assets/Character_Demon_2_Sprite_Sheet_2.png");
	Image characterMaiden = loadImage(renderer, "Assets/Character_Maiden_1.png");
	Image characterGhoul = loadImage(renderer, "Assets/Character_Ghoul_5.png");
	Image characterVampireA = loadImage(renderer, "Assets/Character_Vampire_4.png");
	Image characterVampireB = loadImage(renderer, "Assets/Character_Vampire_5_ShortHair.png");
	Image characterFrankensteinCreation = loadImage(renderer, "Assets/Character_FrankensteinCreation_1.png");
	Image characterSkeleton = loadImage(renderer, "Assets/Character_Skeleton_1.png");
	
	// Enemy Types
	// Image enemyBat = loadImage(renderer, "Assets/Enemy_VampireBat_1.png");
	Image enemyBatAnimated = loadImage(renderer, "Assets/Enemy_VampireBat_1_SpriteSheet.png");
	// Image enemyGargoyle = loadImage(renderer, "Assets/Enemy_Gargoyle_1.png");
	// Image enemyGargoyle = loadImage(renderer, "Assets/Enemy_Gargoyle_1_Smaller.png");
	Image enemyGargoyleAnimated = loadImage(renderer, "Assets/Enemy_Gargoyle_1_Smaller_Sprite_Sheet.png");
	Image enemyType = {};
	enemyType = enemyBatAnimated;

	// Weapon Types
	// gameData.weaponType[WEAPON_SHADOW_ORB] = loadImage(renderer, "Assets/Weapon_ShadowOrb_1.png");
	// gameData.weaponType[WEAPON_SPIKE] = loadImage(renderer, "Assets/Weapon_Spike_1.png");
	Image weaponSpike = loadImage(renderer, "Assets/Weapon_Spike_2.png");
	Image weaponShadowOrb = loadImage(renderer, "Assets/Weapon_ShadowOrb_1.png");
	Image weaponConsecratedGround = loadImage(renderer, "Assets/Weapon_Consecrated_Ground_1.png");
	Image weaponFireball = loadImage(renderer, "Assets/Weapon_Fireball_1.png");
	Image weaponFireballAOE = loadImage(renderer, "Assets/Weapon_Fireball_AOE_1.png");

	Image currentWeaponSelected = weaponSpike;
	int weaponDamage = weaponDamageSpike;
	double attackSpeed = attackSpeedSpike;
	double projectileSpeed = projectileSpeedSpike;

	Image experienceOrbImage = loadImage(renderer, "Assets/Experience_Orb_1.png");
	// Image font = loadFont(renderer, "Assets/Font_1.png");
	// Image font = loadFont(renderer, "Assets/Font_2.png");
	Image font = loadFont(renderer, "Assets/Font_3.png");

	// Map Tiles
	gameData.tileTypeArray[TILE_GRASS] = loadImage(renderer, "Assets/grassTile.png");
	gameData.tileTypeArray[TILE_DIRT] = loadImage(renderer, "Assets/dirtTile.png");
	gameData.tileTypeArray[TILE_ROCK] = loadImage(renderer, "Assets/rockTile.png");

	Image dirtGrassSpriteSheet = loadImage(renderer, "Assets/Tiles_Dirt_Grass_Sprite_Sheet.png");

	gameData.player = createCharacter(characterDemonAnimated, 100, false, 300, 3);
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

				// Characters
				case SDLK_1:
					gameData.player = createCharacter(characterDemonAnimated, 100, true, 300, 3);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_2:
					gameData.player = createCharacter(characterGhoul, 100, false, 300, 1);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_3:
					gameData.player = createCharacter(characterMaiden, 100, false, 300, 1);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_4:
					gameData.player = createCharacter(characterVampireA, 100, false, 300, 1);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_5:
					gameData.player = createCharacter(characterVampireB, 100, false, 300, 1);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_6:
					gameData.player = createCharacter(characterFrankensteinCreation, 100, false, 300, 1);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
				case SDLK_7:
					gameData.player = createCharacter(characterSkeleton, 100, false, 300, 1);
					gameData.player.position.x = RESOLUTION_X / 2;
					gameData.player.position.y = RESOLUTION_Y / 2;
					break;
					

				// Enemies
				case SDLK_z:
					destroyEnemies(gameData);
					enemyType = enemyBatAnimated;
					break;
				case SDLK_x:
					destroyEnemies(gameData);
					enemyType = enemyGargoyleAnimated;
					break;

				// Weapons
				case SDLK_r:
					fireball = false;
					consecratedGround = false;
					currentWeaponSelected = weaponSpike;
					weaponDamage = weaponDamageSpike;
					attackSpeed = attackSpeedSpike;
					projectileSpeed = projectileSpeedSpike;
					break;
				case SDLK_t:
					fireball = false;
					consecratedGround = false;
					currentWeaponSelected = weaponShadowOrb;
					weaponDamage = weaponDamageShadowOrb;
					attackSpeed = attackSpeedShadowOrb;
					projectileSpeed = projectileSpeedShadowOrb;
					break;
				case SDLK_y:
					fireball = false;
					consecratedGround = true;
					weaponDamage = weaponDamageConsecratedGround;
					break;
				case SDLK_u:
					fireball = true;
					consecratedGround = false;
					currentWeaponSelected = weaponFireball;
					weaponDamage = weaponDamageFireball;
					attackSpeed = attackSpeedFireball;
					projectileSpeed = projectileSpeedFireball;
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

			createEnemy(enemyType, enemyPosition, &gameData, 100, 2, true, 300, 2);
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

		// Update weapon position
		for (int i = 0; i < gameData.weapon.size(); i++) {
			updateEntityPosition(&gameData.weapon[i], deltaTime);
		}
		// Update Experience Orb position
		for (int i = 0; i < gameData.experienceOrbs.size(); i++) {
			updateExperienceOrbPosition(gameData, &gameData.experienceOrbs[i], EXPERIENCE_ORB_SPEED, deltaTime);
		}

		// Weapon firing at nearest enemy
		if (!consecratedGround) {
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
						currentWeaponInUse.velocity *= projectileSpeed;
						fireTime = attackSpeed;
						currentWeaponInUse.angle = angleFromDirection(currentWeaponInUse.velocity);
						gameData.weapon.push_back(currentWeaponInUse);
					}
				}
			}
		}

		if (consecratedGround) {
			if (gameData.player.hp > 0) {
				gameData.consecratedGround = createWeaponConsecratedGround(weaponConsecratedGround, weaponDamageConsecratedGround);
				gameData.consecratedGround.position = gameData.player.position;
			}
		}

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
						if (!gameData.weapon[i].fireballAOE) {
							gameData.weapon[i].lifeTime = 0;
						}

						if (gameData.weapon[i].fireball) {
							Weapon fireballAOE = createWeapon(weaponFireballAOE, weaponDamageFireballAOE);
							fireballAOE.position = gameData.enemies[i].position;
							fireTime = attackSpeed;
							fireballAOE.lifeTime = weaponDurationFireballAOE;
							fireballAOE.fireballAOE = true;
							gameData.weapon.push_back(fireballAOE);
						}

						if (gameData.enemies[j].hp <= 0) {
							gameData.enemies[j].destroyed = true;
							totalEnemiesKilled += 1;
							// Experience Orbs Vector
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
			if (gameData.enemies[i].timeUntilDamageCG > 0) {
				gameData.enemies[i].timeUntilDamageCG -= deltaTime;
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

		// Player collision with experience orb
		if (gameData.player.hp > 0) {
			for (int i = 0; i < gameData.experienceOrbs.size(); i++) {
				double distanceBetween = distancePlayer(gameData.player.position, gameData.experienceOrbs[i].position);
				double radiusSum = (gameData.player.radius / 2) + gameData.experienceOrbs[i].radius;
				if (distanceBetween < radiusSum) {
					gameData.experienceOrbs[i].lifeTime = 0;
					gameData.player.experience += experienceOrbExperience;
					if (gameData.player.experience == 1000) {
						gameData.player.level += 1;
						gameData.player.experience = 0;
					}
				}
			}
		}

		// Consecrated Ground collision with enemy
		if (consecratedGround) {
			for (int i = 0; i < gameData.enemies.size(); i++) {
				if (gameData.enemies[i].timeUntilDamageCG <= 0) {
					double distanceBetween = distance(gameData.enemies[i].position, gameData.consecratedGround.position);
					double radiusSum = gameData.enemies[i].radius + gameData.consecratedGround.radius;
					if (distanceBetween < radiusSum) {
						if (gameData.enemies[i].hp > 0) {
							Vector numberVelocity = {};
							DamageNumber damageNumber = createDamageNumber(ENTITY_ENEMY, gameData.consecratedGround.damage, gameData.enemies[i].position,
								{ randomFloat(300, -300), randomFloat(-600, -300) }, DAMAGE_NUMBER_SIZE_E, DAMAGE_NUMBER_LIFETIME);
							gameData.enemies[i].hp -= gameData.consecratedGround.damage;
							gameData.damageNumbers.push_back(damageNumber);
							// Time until hit again
							gameData.enemies[i].timeUntilDamageCG = damageDelayConsecratedGround;
							if (gameData.enemies[i].hp <= 0) {
								gameData.enemies[i].destroyed = true;
								// Experience Orbs Vector
								ExperienceOrb experienceOrb = createExperienceOrb(gameData, experienceOrbImage,
									gameData.enemies[i].position.x, gameData.enemies[i].position.y, EXPERIENCE_ORB_LIFETIME);
								gameData.experienceOrbs.push_back(experienceOrb);
								totalEnemiesKilled += 1;
							}
							// Knock back enemies
							gameData.enemies[i].velocity.x = gameData.enemies[i].velocity.x * -1;
							gameData.enemies[i].velocity.y = gameData.enemies[i].velocity.y * -1;
							double length = sqrt(gameData.enemies[i].velocity.x * gameData.enemies[i].velocity.x +
								gameData.enemies[i].velocity.y * gameData.enemies[i].velocity.y);
							gameData.enemies[i].velocity.x /= length;
							gameData.enemies[i].velocity.y /= length;
							gameData.enemies[i].velocity = gameData.enemies[i].velocity * 600;
							break;
						}
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
				float perlin = stb_perlin_noise3(tile.position.x / 256, tile.position.y / 256, 0, 0, 0, 0);
				drawTile(gameData, tile, perlin);
			}
		}

#if 0
		ProceduralTile tile = {};
		tile.position.x = 50;
		tile.position.y = 50;
		tile.type = 0;
		drawProceduralTile(gameData, dirtGrassSpriteSheet, tile, (dirtGrassSpriteSheet.w / TILE_SIZE));
#endif

		if (consecratedGround) {
			drawEntity(gameData, gameData.consecratedGround);
		}

		drawHealthBar(gameData, renderer);

		Color textColor = {};
		textColor.r = 255;
		textColor.g = 255;
		textColor.b = 255;
		// std::to_string(totalEnemiesKilled)
		// Kill tracker
		drawString(textColor, gameData, gameData.renderer, &font, 1, std::string("Kills: "), 10, 10);
		drawString(textColor, gameData, gameData.renderer, &font, 1, std::to_string(totalEnemiesKilled), 100, 10);

		// Draw entities
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
		int numberOfPixelsW = experienceTracker.size() * 14;
		drawString(textColor, gameData, renderer, &font, 1, experienceTracker, (RESOLUTION_X / 2) - (numberOfPixelsW / 2), 865);

		// Level tracker
		std::string levelTracker = std::string("Level: ");
		levelTracker += std::to_string(gameData.player.level);
		int pixelWidthLeveltracker = levelTracker.size() * 14;
		drawString(textColor, gameData, renderer, &font, 1, levelTracker, (RESOLUTION_X / 2) - (pixelWidthLeveltracker / 2), 835);

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
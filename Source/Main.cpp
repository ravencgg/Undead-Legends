#include "SDL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// C's standard io. Used for sending errors for console.
#include <stdio.h>
#include "Sprite.h"
#include <algorithm>

// DONE: Bullet collision detection with enemies
// DONE: Player collision with enemies (reuse from bullet)
// DONE: Add enemy health
// DONE: Add player health // Currently, the enemies do a small amount of damage really fast
// DONE: Enemies get pushed back when hit if the bullet doesn't kill them
// TODO: Enemies don't collide (push eachother away) // Have every enemy push every other 
// enemy away (set velocity or position)
// TODO: Make hitbox smaller
// TODO: Potentially impliment the normalize function
// TODO: Randomly spawn enemies around player / off screen?

bool running = true;
bool up = false;
bool down = false;
bool left = false;
bool right = false;
double fireTime = 0;
double ATTACKSPEED = .1;
double PROJECTILESPEED = 250;

int main(int argc, char** argv) {

	SDL_Window* window = nullptr;
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Undead Legends",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		RESOLUTION_X, RESOLUTION_Y, SDL_WINDOW_SHOWN);

	// -1 = default gpu
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	Image mapA = loadImage(renderer, "Assets/Map_1.png");
	// Character_Ghoul_5.png
	// Character_Vampire_4.png
	// Character_Vampire_5_ShortHair.png
	// Character_Maiden_1.png
	// Character_FrankensteinCreation_1.png
	// Character_Skeleton_1.png
	Image characterA = loadImage(renderer, "Assets/Character_Maiden_1.png");
	Image mapTextureGrass = loadImage(renderer, "Assets/Map_Grass_1.png");
	// Assets/Enemy_VampireBat_1.png
	// Assets/Enemy_Gargoyle_1.png
	Image enemyA = loadImage(renderer, "Assets/Enemy_VampireBat_1.png");
	Image weaponSpikeImage = loadImage(renderer, "Assets/Weapon_Spike_1.png");

	// Vectors initialize themselves in the constructor
	GameData gameData;

	gameData.player = createCharacter(characterA, 100);
	
	for (int i = 0; i < 50; i++) {
		double range = randomFloat((RESOLUTION_X / 5), RESOLUTION_X / 2);
		Vector enemyPosition = facingDirection(randomFloat(0, 360));
		
		enemyPosition.x *= range;
		enemyPosition.y *= range;
		
		enemyPosition.x += RESOLUTION_X / 2;
		enemyPosition.y += RESOLUTION_Y / 2;

		createEnemy(enemyA, enemyPosition, &gameData, 100, 2);
	}

	// Capping frame rate
	const int FPS = 60;
	// The max time between each frame
	const int frameDelay = 1000 / FPS;
	// Massive integer
	Uint32 frameStart;
	int frameTime;

	double lastFrameTime = getTime();


	gameData.player.sprite.position.x = RESOLUTION_X / 2;
	gameData.player.sprite.position.y = RESOLUTION_Y / 2;

	// Game Loop
 	while (running) {
		// How many miliseconds it's been since we first
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
			gameData.player.sprite.position.x -= speed;
		}
		if (right) {
			gameData.player.sprite.position.x += speed;
		}
		if (down) {
			gameData.player.sprite.position.y += speed;
		}
		if (up) {
			gameData.player.sprite.position.y -= speed;
		}

		// What is the first thing I should do?
		// 1: Check to see if the enemies are colliding with eachother

		if (gameData.player.healthPoints > 0) {
			for (int i = 0; i < gameData.enemies.size(); i++) {			
				updateEnemyPosition(&gameData.player, &gameData.enemies[i], deltaTime);
				for (int j = 0; j < gameData.enemies.size(); j++) {
					if (j == i) {
						continue;
					}
					else {
						// Length
						double distanceBetween = distance(gameData.enemies[i].sprite.position, gameData.enemies[j].sprite.position);
						double radiusSum = gameData.enemies[i].radius + gameData.enemies[j].radius;
						if (distanceBetween < radiusSum) {
							Vector offset = gameData.enemies[j].sprite.position - gameData.enemies[i].sprite.position;
							// This is equivalent to offset / length
							offset *= 1 / distanceBetween;							
							offset *= radiusSum;
							gameData.enemies[j].sprite.position = offset + gameData.enemies[i].sprite.position;
						}
						// float randomNumber = randomFloat(-80, 80);
					}
				}
			}
		}

		// Update weapon position
		for (int i = 0; i < gameData.weaponSpike.size(); i++) {
			updateSpritePosition(&gameData.weaponSpike[i].sprite, deltaTime);
		}

		// Weapon spike firing at nearest enemies
		// Check if the player is existing
		if (gameData.player.healthPoints > 0) {
			if (fireTime <= 0) {
				// Find the closest enemy
				int nearestEnemy = closestEnemy(gameData.player, &gameData);
				if (nearestEnemy >= 0) {
					Weapon weaponSpike = createWeapon(weaponSpikeImage, 50);
					weaponSpike.sprite.position = gameData.player.sprite.position;
					// Vector spikeDirection = facingDirection(weaponSpikeArray[i].sprite.angle);
					Vector offset = {};
					// Calculates the vector from the player to the enemy (enemy <--- player)
					offset = gameData.enemies[nearestEnemy].sprite.position - gameData.player.sprite.position;
					weaponSpike.sprite.velocity = normalize(offset);
					weaponSpike.sprite.velocity *= PROJECTILESPEED;
					fireTime = ATTACKSPEED;
					weaponSpike.sprite.angle = angleFromDirection(weaponSpike.sprite.velocity);
					gameData.weaponSpike.push_back(weaponSpike);
				}
			}
		}

		// Weapon collision with enemy
		for (int i = 0; i < gameData.weaponSpike.size(); i++) {
			for (int j = 0; j < gameData.enemies.size(); j++) {
				double distanceBetween = distance(gameData.weaponSpike[i].sprite.position, gameData.enemies[j].sprite.position);
				double radiusSum = gameData.weaponSpike[i].radius + gameData.enemies[j].radius;
				if (distanceBetween < radiusSum) {
					if (gameData.enemies[j].healthPoints > 0) {
						gameData.enemies[j].healthPoints -= gameData.weaponSpike[i].damage;
						gameData.weaponSpike[i].lifeTime = 0;
						if (gameData.enemies[j].healthPoints <= 0) {
							gameData.enemies[j].destroyed = true;						
						}

						// Knock back enemies
						gameData.enemies[j].velocity = gameData.weaponSpike[i].sprite.velocity;
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
		if (gameData.player.healthPoints > 0) {
			for (int i = 0; i < gameData.enemies.size(); i++) {
				if (gameData.enemies[i].timeUntilDamage <= 0) {
					double distanceBetween = distance(gameData.player.sprite.position, gameData.enemies[i].sprite.position);
					double radiusSum = gameData.player.radius + gameData.enemies[i].radius;
					if (distanceBetween < radiusSum) {
						gameData.player.healthPoints -= gameData.enemies[i].damage;
						playerTakingDamage = true;
						gameData.enemies[i].timeUntilDamage = .1;
					}
				}
			}
		}

		// SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		// Clear what we are drawing to
		// Anything done before render clear gets erased
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, mapA.texture, NULL, NULL);

		// This makes it so the character is no longer draw to the screen
		// but the character still exists
		if (gameData.player.healthPoints > 0) {
			drawSprite(renderer, gameData.player.sprite);
			if (playerTakingDamage) {
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				drawCircle(renderer, gameData.player.sprite.position, gameData.player.radius);
			}
		}

		// Draw enemies
		for (int i = 0; i < gameData.enemies.size(); i++) {
			// Check to see if the boolean value is true when the enemy was created. If it was, draw it.
			drawSprite(renderer, gameData.enemies[i].sprite);
			// SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			// drawCircle(renderer, enemy[i].sprite.position, enemy[i].radius);
		}

		// Draw bullets
		for (int i = 0; i < gameData.weaponSpike.size(); i++) {
			drawSprite(renderer, gameData.weaponSpike[i].sprite);
			gameData.weaponSpike[i].lifeTime -= deltaTime;
			//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			//drawCircle(renderer, weaponSpikeArray[i].sprite.position, weaponSpikeArray[i].radius);
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

		// If lifeTime <= 0, get rid of it
		std::erase_if(gameData.weaponSpike, [](const Weapon& weapon) {
			return weapon.lifeTime <= 0;
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
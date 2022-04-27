#include "SDL.h"
#include "stb_image.h"
#include <stdio.h>
#include "Sprite.h"
// For max double in nearestEnemy()
#include <float.h>

// A .obj file is the result of compilation. Inside them 
// is all the code and hard coded data.
// The linker will open Vector.obj
// Cpp files are compiled. Header files are not.

double getTime() {
	return SDL_GetTicks() / 1000.0;
}

Vector facingDirection(double theta) {
	Vector result = {};
	// M_PI / 180 is converting from degrees to radians
	// 90 = Pi / 2
	// The cos and sin of any angle is 1
	result.x = cos(theta * M_PI / 180);
	result.y = sin(theta * M_PI / 180);
	// Returns JUST a direction of that angle
	return result;
}

double angleFromDirection(Vector a) {
	double result = (180 / M_PI) * atan2(a.y, a.x);
	return result;
}

// Vector math
Vector normalize(Vector a) {
	// Find the length (the magnitude)
	// (See notes)
	double magnitude = sqrt(a.x * a.x + a.y * a.y);
	Vector normalized = { a.x / magnitude, a.y / magnitude };
	return normalized;
}

Vector operator+(Vector a, Vector b) {
	Vector result = {};
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

Vector operator-(Vector a, Vector b) {
	Vector result = {};
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

Vector& operator*=(Vector& a, double b) {
	a.x *= b;
	a.y *= b;
	return a;
}

Vector operator*(Vector a, double b) {
	Vector result = {};
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

// it is a 'const' because the copy (source) doesn't change
void myMemcpy(void* destination, void const* source, size_t size) {
	// We want to go 1 byte at a time using char*
	char* d = (char*)destination;
	char* s = (char*)source;

	for (int i = 0; i < size; i++)
	{
		// de-reference
		// all of memory is one long array
		// dereference means you are talking about the thing the pointer is pointing at, not the address itself
		// d is an address. the square bracket is saying we want to access the i variable after d.
		// Examples:
		// d[i] = s[i] is the same as *d = *s;
		// *(d + j) = *(s + i);
		// *d++ = *s++;
		/*
		*d = *s;
		d++;
		s++;
		*/

		d[i] = s[i];
	}
}

Sprite createSprite(Image image) {
	Sprite result = {};
	SDL_QueryTexture(image.texture, NULL, NULL, &result.width, &result.height);
	result.image = image;
	return result;
}

Color readPixel(Image image, int x, int y) {
	// This is a cast from 1 unsigned character to 4 unsigned characters
	Color* pixels = (Color*)image.pixelData;

	// The square brackets are derefencing the pointer
	// 3 * 6 + 6
	Color c = pixels[y * image.w + x];

	return c;
}

double distance(Vector a, Vector b) {
	Vector to_a = {};
	to_a.x = a.x - b.x;
	to_a.y = a.y - b.y;

	double result = sqrt(to_a.x * to_a.x + to_a.y * to_a.y);
	return result;
}

// Review
Image loadImage(SDL_Renderer* renderer, const char* fileName) {
	Image result;

	int x, y, n;
	unsigned char* data = stbi_load(fileName, &x, &y, &n, 4);

	SDL_Texture* texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
		x, y);

	int pitch;
	void* pixels;

	// Locking the texture gives us a position in memory
	// While the texture is locked, you can write to the texture
	int lockTexture = SDL_LockTexture(texture, NULL, &pixels,
		&pitch);

	myMemcpy(pixels, data, (x * y * 4));

	// prevents leaking memory (memory is being leaked
	// we no longer want to free the memory
	// stbi_image_free(data);

	// When the texture is unlocked, you can no longer write to it
	SDL_UnlockTexture(texture);

	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	result.pixelData = data;
	result.texture = texture;
	result.w = x;
	result.h = y;

	return result;
}

double returnSpriteSize(Image image) {
	Vector center = {};
	center.x = (double)image.w / 2;
	center.y = (double)image.h / 2;

	Vector pixel = {};

	double maxDistance = 0;

	for (int i = 0; i < image.w; i++) {
		pixel.x = i;
		for (int j = 0; j < image.h; j++) {
			pixel.y = j;
			Color alphaTest = (readPixel(image, i, j));

			if (alphaTest.a > 0) {
				double distance1 = distance(center, pixel);
				if (distance1 > maxDistance) {
					maxDistance = distance1;
				}
			}
		}
	}

	return maxDistance;
}

Character createCharacter(Image image, int healthPoints) {
	Character character = {};
	character.sprite = createSprite(image);
	character.radius = returnSpriteSize(image);
	character.healthPoints = healthPoints;
	return character;
}

float randomFloat(float min, float max) {
	float base = (float)rand() / (float)RAND_MAX;
	float range = max - min;
	float newRange = range * base + min;
	return newRange;
}

void updateSpritePosition(Sprite* sprite, double delta) {
	// This is where the player exists in the world at this frame
	// -> is short hand for dereferencing a member out of a pointer
	sprite->position.x += sprite->velocity.x * delta;
	sprite->position.y += sprite->velocity.y * delta;
}

float dotProduct(Vector a, Vector b) {
	return a.x * b.x + a.y * b.y;
}

void updateEnemyPosition(Character* player, Enemy* enemy, double delta) {
	// Direction you want to acclerate in
	Vector enemyToPlayer = player->sprite.position - enemy->sprite.position;
	
	// Normalize to 1
	double length = sqrt(enemyToPlayer.x * enemyToPlayer.x + enemyToPlayer.y * enemyToPlayer.y);
	enemyToPlayer.x /= length;
	enemyToPlayer.y /= length;
	Vector enemyAcceleration = (enemyToPlayer * ENEMY_ACCELERATION);

	// Kinematic Equations
	// v = v0 + a0t
	Vector updatedVelocity = enemy->velocity + (enemyAcceleration * delta);
	double currentLength = sqrt(updatedVelocity.x * updatedVelocity.x + updatedVelocity.y * updatedVelocity.y);
	if (currentLength > ENEMY_SPEED) {
		if (dotProduct(updatedVelocity, enemyAcceleration) > 0) {
			updatedVelocity.x /= currentLength;
			updatedVelocity.y /= currentLength;

			updatedVelocity *= ENEMY_SPEED;
		}
	}
	// x = x0 + v0t
	Vector updatedPosition = enemy->sprite.position + (updatedVelocity * delta);
	
	enemy->velocity = updatedVelocity;
	enemy->sprite.position = updatedPosition;
}

void drawSprite(SDL_Renderer* renderer, Sprite sprite) {
	SDL_Rect rect;
	// rect.x and y is an int. It is truncating the double when it is cast to an int -> (int)
	rect.w = sprite.width;
	rect.h = sprite.height;
	rect.x = (int)sprite.position.x - rect.w / 2;
	rect.y = (int)sprite.position.y - rect.h / 2;

	SDL_RenderCopyEx(renderer, sprite.image.texture, NULL, &rect, sprite.angle, NULL, SDL_FLIP_NONE);
}

// Adds one enemy into the world
void createEnemy(Image image, Vector position, GameData* gameData, int healthPoints, int damage) {
	Enemy enemy = {};
	enemy.radius = returnSpriteSize(image);
	enemy.sprite = createSprite(image);
	enemy.sprite.position = position;
	enemy.healthPoints = healthPoints;
	enemy.damage = damage;
	enemy.timeUntilDamage = 0;
	gameData->enemies.push_back(enemy);
}

Weapon createWeapon(Image image, int damage) {
	Weapon weapon = {};
	weapon.sprite = createSprite(image);
	weapon.radius = returnSpriteSize(image);
	weapon.lifeTime = 10;
	weapon.damage = damage;
	return weapon;
}

int closestEnemy(Character player, GameData* gameData) {
	double closestDistance = DBL_MAX;
	int index = -1;
	for (int i = 0; i < gameData->enemies.size(); i++) {
		double distanceBetween = distance(player.sprite.position, gameData->enemies[i].sprite.position);
		if (distanceBetween < closestDistance) {
			closestDistance = distanceBetween;
			index = i;
		}
	}
	return index;
}

void drawCircle(SDL_Renderer* renderer, Vector position, float radius) {
	// We are going through the C runtime library so we need to be talking in terms of radians
	// SDL talks in terms of degrees
	const int NUMPOINTS = 24;
	const float DELTA = (M_PI * 2) / NUMPOINTS;


	// 24 is 1 less of the number of lines we need to draw so add one
	for (int i = 0; i < NUMPOINTS + 1; i++) {
		// Where we are starting to draw
		float x1 = position.x + cos(DELTA * i) * radius;
		float y1 = position.y + sin(DELTA * i) * radius;
		// Where we are drawing to
		float x2 = position.x + cos(DELTA * (i + 1)) * radius;
		float y2 = position.y + sin(DELTA * (i + 1)) * radius;
		// Draw the line
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}

}
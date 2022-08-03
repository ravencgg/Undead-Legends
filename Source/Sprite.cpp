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

void myMemcpy(void* destination, void const* source, size_t size) {
	char* d = (char*)destination;
	char* s = (char*)source;

	for (int i = 0; i < size; i++)
	{
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
	// Calculate the offset
	to_a.x = a.x - b.x;
	to_a.y = a.y - b.y;
	// Calculate the length
	double result = sqrt(to_a.x * to_a.x + to_a.y * to_a.y);
	return result;
}

double distancePlayer(Vector a, Vector b) {
	Vector to_a = {};
	// Calculate the offset
	to_a.x = a.x - b.x;
	to_a.y = (a.y + 20) - b.y;
	// Calculate the length
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


Image loadText(SDL_Renderer* renderer, const char* fileName) {
	Image result;
	int x, y, n;

	unsigned char* fileData = stbi_load(fileName, &x, &y, &n, 4);

	unsigned char* readPixels = fileData;

	int totalPixels = x * y;

	for (int i = 0; i < totalPixels; i++) {
		char r = readPixels[0];
		char g = readPixels[1];
		char b = readPixels[2];
		char a = readPixels[3];

		if (r == 0)
			readPixels[3] = 0;

		readPixels += 4;
	}

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, x, y);

	void* pixels;
	int pitch;
	int area = (x * y * 4);

	int lockTexture = SDL_LockTexture(texture, NULL, &pixels, &pitch);

	myMemcpy(pixels, fileData, area);

	SDL_UnlockTexture(texture);

	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	result.pixelData = fileData;
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
	character.radius = returnSpriteSize(image) / 2;
	character.hp = healthPoints;
	return character;
}

/*
void createTile(GameData& gameData, int x, int y) {
	Tile currentTile = {};
	currentTile.tileType = rand() % 3;
	currentTile.position.x = x * TILE_SIZE;
	currentTile.position.y = y * TILE_SIZE;
	currentTile.height = TILE_SIZE;
	currentTile.width = TILE_SIZE;
	gameData.generatedTiles.push_back(currentTile);
}
*/

float randomFloat(float min, float max) {
	float base = (float)rand() / (float)RAND_MAX;
	float range = max - min;
	float newRange = range * base + min;
	return newRange;
}

// Test function
float randomFloatScreen(float min, float max) {
	// Gives us a random percentage
	float base = (float)rand() / (float)RAND_MAX;
	float range = max - min;
	float newRange = range * base + min;
	return newRange;
}

void updateEntityPosition(Entity* entity, double delta) {
	// This is where the player exists in the world at this frame
	// -> is short hand for dereferencing a member out of a pointer
	entity->position.x += entity->velocity.x * delta;
	entity->position.y += entity->velocity.y * delta;
}

float dotProduct(Vector a, Vector b) {
	return a.x * b.x + a.y * b.y;
}

void updateEnemyPosition(Character* player, Enemy* enemy, double delta) {
	// Direction you want to acclerate in
	Vector enemyToPlayer = player->position - enemy->position;
	
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
	Vector updatedPosition = enemy->position + (updatedVelocity * delta);
	
	enemy->velocity = updatedVelocity;
	enemy->position = updatedPosition;
}

SDL_Rect convertCameraSpace(Camera& camera, SDL_Rect worldSpace) {
	SDL_Rect cameraSpace;
	cameraSpace.w = worldSpace.w;
	cameraSpace.h = worldSpace.h;

	Vector toCenter = { RESOLUTION_X / 2.0f, RESOLUTION_Y / 2.0f };

	Vector worldSpaceV = { worldSpace.x, worldSpace.y };

	// Entity <------------ Camera 
	// worldSpaceV is the vector of the entity
	Vector offset = worldSpaceV - camera.position;

	offset = offset + toCenter;
	cameraSpace.x = (int)offset.x - cameraSpace.w / 2;
	cameraSpace.y = (int)offset.y - cameraSpace.h / 2;
	
	return cameraSpace;
}

void drawEntity(GameData& gameData, Entity &entity) {
	SDL_Rect rect;
	// rect.x and y is an int. It is truncating the double when it is cast to an int -> (int)
	rect.w = entity.sprite.width;
	rect.h = entity.sprite.height;
	rect.x = entity.position.x;
	rect.y = entity.position.y;

	rect = convertCameraSpace(gameData.camera, rect);

	SDL_RenderCopyEx(gameData.renderer, entity.sprite.image.texture, NULL, &rect, entity.angle, NULL, SDL_FLIP_NONE);
}

int getRandomTile() {
	int randomNumber = rand() % TILE_COUNT;
	return randomNumber;
}

void drawTile(GameData& gameData, Tile tile) {
	SDL_Rect rect;
	rect.w = TILE_SIZE;
	rect.h = TILE_SIZE;
	rect.x = tile.position.x;
	rect.y = tile.position.y;

	rect = convertCameraSpace(gameData.camera, rect);

	SDL_RenderCopyEx(gameData.renderer, gameData.tileTypeArray[tile.tileType].texture, NULL, &rect, 0, NULL, SDL_FLIP_NONE);
}

// Adds one enemy into the world
void createEnemy(Image image, Vector position, GameData* gameData, int healthPoints, int damage) {
	Enemy enemy = {};
	enemy.radius = returnSpriteSize(image) / 2;
	enemy.sprite = createSprite(image);
	enemy.position = position;
	enemy.hp = healthPoints;
	enemy.damage = damage;
	enemy.timeUntilDamage = 0;
	gameData->enemies.push_back(enemy);
}

#if 0
Tile createTile(Image image, Vector position) {
	Tile tile = {};
	tile.position = position;
	tile.height = TILE_SIZE;
	tile.width = TILE_SIZE;
	tile.image = image;
	return tile;
}
#endif

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
		double distanceBetween = distance(player.position, gameData->enemies[i].position);
		if (distanceBetween < closestDistance) {
			closestDistance = distanceBetween;
			index = i;
		}
	}
	return index;
}

void drawCircle(GameData& gameData, Vector position, float radius, int circleOffsetY) {
	// We are going through the C runtime library so we need to be talking in terms of radians
	// SDL talks in terms of degrees
	const int NUMPOINTS = 24;
	const float DELTA = (M_PI * 2) / NUMPOINTS;

	// 24 is 1 less of the number of lines we need to draw so add one
	for (int i = 0; i < NUMPOINTS + 1; i++) {
		// Where we are starting to draw
		float x1 = position.x + cos(DELTA * i) * radius;
		float y1 = (position.y - circleOffsetY) + sin(DELTA * i) * radius;
		// Where we are drawing to
		float x2 = position.x + cos(DELTA * (i + 1)) * radius;
		float y2 = (position.y - circleOffsetY) + sin(DELTA * (i + 1)) * radius;
		
		SDL_Rect rectangle1 = {};
		rectangle1.x = x1;
		rectangle1.y = y1;
		rectangle1 = convertCameraSpace(gameData.camera, rectangle1);

		SDL_Rect rectangle2 = {};
		rectangle2.x = x2;
		rectangle2.y = y2;
		rectangle2 = convertCameraSpace(gameData.camera, rectangle2);

		SDL_RenderDrawLine(gameData.renderer, rectangle1.x, rectangle1.y, rectangle2.x, rectangle2.y);
	
	}
}

void drawString(GameData& gameData, SDL_Renderer* renderer, Image* textImage, int size, std::string string, int x, int y) {
	SDL_Rect sourceRect;
	sourceRect.w = 7;
	sourceRect.h = 9;
	int stringSize = string.size();
	int spacing = 0;
	for (int i = 0; i < stringSize; i++) {
		char glyph = string[i];
		int index = glyph - ' ';
		int row = index / 18;
		int col = index % 18;
		
		sourceRect.x = (col * 7);
		sourceRect.y = (row * 9);

		SDL_Rect destinationRect;

		destinationRect.x = x + spacing;
		spacing += sourceRect.w;

		destinationRect.y = y - ((9 * 4) / 2);
		
		destinationRect.w = 7 * size;
		destinationRect.h = 9 * size;

		destinationRect = convertCameraSpace(gameData.camera, destinationRect);
		
		SDL_RenderCopy(renderer, textImage->texture, &sourceRect, &destinationRect);
		spacing += destinationRect.w;
	}
}

DamageNumber createDamageNumber(int damageNumber, Vector position, Vector velocity, int textSize, double lifeTime) {
	DamageNumber result;

	result.damageString = std::to_string(damageNumber);
	result.position = position;
	result.velocity = velocity;
	result.textSize = textSize;
	result.lifeTime = lifeTime;

	return result;
}

void drawDamageNumber(GameData& gameData, DamageNumber& damageNumber, Image* textImage, double deltaTime) {
	Vector finalVelocity = {};

	finalVelocity.x = damageNumber.velocity.x;
	// Gravity only affects the Y axis
	finalVelocity.y = damageNumber.velocity.y + (GRAVITY * deltaTime);

	Vector displacement = {};
	displacement.x = ((finalVelocity.x + damageNumber.velocity.x) / 2) * deltaTime;
	displacement.y = ((finalVelocity.y + damageNumber.velocity.y) / 2) * deltaTime;

	damageNumber.velocity = finalVelocity;
	damageNumber.position.x += displacement.x;
	damageNumber.position.y += displacement.y;

	drawString(gameData, gameData.renderer, textImage, damageNumber.textSize, damageNumber.damageString, damageNumber.position.x, damageNumber.position.y);
}

ExperienceOrb createExperienceOrb(GameData& gameData, Image image, int positionX, int positionY, double lifeTime) {
	ExperienceOrb result = {};

	result.sprite = createSprite(image);
	result.radius = returnSpriteSize(image);
	result.position.x = positionX;
	result.position.y = positionY;
	result.lifeTime = lifeTime;

	return result;
}
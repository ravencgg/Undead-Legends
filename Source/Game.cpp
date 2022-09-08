#include "Constants.h"
#include "Game.h"
#include "stb_image.h"

void loadEntityImages() {
			
}

void myMemcpy(void* destination, void const* source, size_t size) {
	char* d = (char*)destination;
	char* s = (char*)source;

	for (int i = 0; i < size; i++)
	{
		d[i] = s[i];
	}
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
	SDL_LockTexture(texture, NULL, &pixels, &pitch);

	myMemcpy(pixels, data, ((size_t)x * (size_t)y * 4));

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

Image loadFont(SDL_Renderer* renderer, const char* fileName) {
	Image result;
	int x, y, n;

	unsigned char* fileData = stbi_load(fileName, &x, &y, &n, 4);

	unsigned char* readPixels = fileData;

	int totalPixels = x * y;

	for (int i = 0; i < totalPixels; i++) {
		char r = readPixels[0];
		// char g = readPixels[1];
		// char b = readPixels[2];
		// char a = readPixels[3];

		if (r == 0) {
			readPixels[3] = 0;
		}
		readPixels += 4;
	}

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, x, y);

	void* pixels;
	int pitch;
	int area = (x * y * 4);

	SDL_LockTexture(texture, NULL, &pixels, &pitch);

	myMemcpy(pixels, fileData, area);

	SDL_UnlockTexture(texture);

	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	result.pixelData = fileData;
	result.texture = texture;
	result.w = x;
	result.h = y;

	return result;
}

float randomFloat(float min, float max) {
	float base = (float)rand() / (float)RAND_MAX;
	float range = max - min;
	float newRange = range * base + min;
	return newRange;
}

double dotProduct(Vector a, Vector b) {
	return a.x * b.x + a.y * b.y;
}

SDL_Rect convertCameraSpace(Camera& camera, SDL_Rect worldSpace) {
	SDL_Rect cameraSpace;
	cameraSpace.w = worldSpace.w;
	cameraSpace.h = worldSpace.h;

	Vector toCenter = { Constants::RESOLUTION_X / 2.0f, Constants::RESOLUTION_Y / 2.0f };

	Vector worldSpaceV = { (double)worldSpace.x, (double)worldSpace.y };

	// Entity <------------ Camera 
	// worldSpaceV is the vector of the entity
	Vector offset = worldSpaceV - camera.position;

	offset = offset + toCenter;
	cameraSpace.x = (int)offset.x - cameraSpace.w / 2;
	cameraSpace.y = (int)offset.y - cameraSpace.h / 2;

	return cameraSpace;
}

// W / H conversion not relevant to WS
SDL_Rect convertCameraSpaceScreenWH(Camera& camera, SDL_Rect worldSpace) {
	SDL_Rect cameraSpace = {};
	// Converts the healthbar to the width and height of the screen
	cameraSpace.w = worldSpace.w * (Constants::RESOLUTION_X / 1600);
	cameraSpace.h = worldSpace.h * (Constants::RESOLUTION_Y / 900);

	Vector toCenter = { Constants::RESOLUTION_X / 2.0f, Constants::RESOLUTION_Y / 2.0f };

	Vector worldSpaceV = { (double)worldSpace.x, (double)worldSpace.y };

	Vector offset = worldSpaceV - camera.position;

	offset = offset + toCenter;

	cameraSpace.x = (int)offset.x - cameraSpace.w / 2;
	cameraSpace.y = (int)offset.y - cameraSpace.h / 2;

	return cameraSpace;
}


void drawTile(GameData& gameData, Tile tile, float perlin) {
	SDL_Rect rect;
	rect.w = Constants::TILE_SIZE;
	rect.h = Constants::TILE_SIZE;
	rect.x = (int)tile.position.x;
	rect.y = (int)tile.position.y;

	if (perlin > -0.1) {
		tile.tileType = TILE_GRASS;
	}
	else if (perlin > -.35) {
		tile.tileType = TILE_DIRT;
	}
	else {
		tile.tileType = TILE_ROCK;
	}
	rect = convertCameraSpace(gameData.camera, rect);

	SDL_RenderCopyEx(gameData.renderer, gameData.tileTypeArray[tile.tileType].texture, NULL, &rect, 0, NULL, SDL_FLIP_NONE);
}

void drawCircle(GameData& gameData, Vector position, double radius, int circleOffsetY) {
	// SDL talks in terms of degrees. We need radians.
	const int NUMPOINTS = 24;
	const double DELTA = (M_PI * 2) / NUMPOINTS;

	// 24 is 1 less of the number of lines we need to draw so add one
	for (int i = 0; i < NUMPOINTS + 1; i++) {
		// Where we are starting to draw
		double x1 = position.x + cos(DELTA * i) * radius;
		double y1 = (position.y - circleOffsetY) + sin(DELTA * i) * radius;
		// Where we are drawing to
		double x2 = position.x + cos(DELTA * ((double)i + 1)) * radius;
		double y2 = (position.y - circleOffsetY) + sin(DELTA * ((double)i + 1)) * radius;

		SDL_Rect rectangle1 = {};
		rectangle1.x = (int)x1;
		rectangle1.y = (int)y1;
		rectangle1 = convertCameraSpace(gameData.camera, rectangle1);

		SDL_Rect rectangle2 = {};
		rectangle2.x = (int)x2;
		rectangle2.y = (int)y2;
		rectangle2 = convertCameraSpace(gameData.camera, rectangle2);

		SDL_RenderDrawLine(gameData.renderer, rectangle1.x, rectangle1.y, rectangle2.x, rectangle2.y);

	}
}

void drawString(Color color, SDL_Renderer* renderer, Image* textImage, int size, std::string string, int x, int y) {
	SDL_Rect sourceRect = {};
	// Font 1
	// int fontW = 7;
	// int fontH = 9;
	// Font 2 
	int fontW = 14;
	int fontH = 18;
	sourceRect.w = fontW;
	sourceRect.h = fontH;
	int stringSize = (int)string.size();
	int spacing = 0;

	for (int i = 0; i < stringSize; i++) {
		char glyph = string[i];
		int index = glyph - ' ';
		int row = index / fontH;
		int col = index % fontH;

		sourceRect.x = (col * fontW);
		sourceRect.y = (row * fontH);

		SDL_Rect destinationRect;

		destinationRect.x = x + spacing;

		destinationRect.y = y;

		destinationRect.w = fontW * size;
		destinationRect.h = fontH * size;

		SDL_SetTextureColorMod(textImage->texture, color.r, color.g, color.b);

		SDL_RenderCopy(renderer, textImage->texture, &sourceRect, &destinationRect);
		spacing += destinationRect.w;
	}
}

void drawStringWorldSpace(Color color, GameData& gameData, SDL_Renderer* renderer, Image* textImage, int size, std::string string, int x, int y) {
	SDL_Rect destRect = {};
	destRect.x = x;
	destRect.y = y;
	destRect = convertCameraSpace(gameData.camera, destRect);
	drawString(color, renderer, textImage, size, string, destRect.x, destRect.y);
}

DamageNumber createDamageNumber(EntityType type, DamageNumberType damageNumberT, double numberDelay, int damageNumber, Vector position, Vector velocity, int textSize, double lifeTime) {
	DamageNumber result;

	result.entityType = type;
	result.damageString = std::to_string(damageNumber);
	result.damageDealt = damageNumber;
	result.position = position;
	result.velocity = velocity;
	result.textSize = textSize;
	result.lifeTime = lifeTime;
	result.damageNumberT = damageNumberT;
	result.numberDelay = numberDelay;

	return result;
}

std::string loadDamageNumberHitSound(GameData& gameData) {
	std::string enemyHitKey = "enemyHitSound";
	// Did not find the key
	if (gameData.soundFileUMap.find(enemyHitKey) == gameData.soundFileUMap.end()) {
		SoLoud::Wav& wav = gameData.soundFileUMap[enemyHitKey];
		// Assets/Audio/mixkit-hard-typewriter-hit-1364.wav
		// Assets/Audio/mixkit-typewriter-hit-1362.wav
		wav.load("Assets/Audio/mixkit-air-in-a-hit-2161.wav");
	}
	return enemyHitKey;
}

// Used inside draw damageNumber
void playEnemyHitSound(GameData& gameData) {
	std::string enemyHitKey = loadDamageNumberHitSound(gameData);
	auto result = gameData.soundFileUMap.find(enemyHitKey);
	if (result != gameData.soundFileUMap.end()) {
		int handle = gameData.soloud.play(result->second, 0.25f, 0, 1);
		float playSpeed = 1.0f;
		gameData.soloud.setRelativePlaySpeed(handle, playSpeed);
		gameData.soloud.setPause(handle, 0);
	}
}

void drawDamageNumber(GameData& gameData, DamageNumber& damageNumber, Image* textImage, double deltaTime) {
	Vector finalVelocity = {};
	Color color = {};

	if (damageNumber.damageNumberT == DN_FALLING) {
		finalVelocity.x = damageNumber.velocity.x;
		// Gravity only affects the Y axis
		finalVelocity.y = damageNumber.velocity.y + (Constants::GRAVITY * deltaTime);

		Vector displacement = {};
		displacement.x = ((finalVelocity.x + damageNumber.velocity.x) / 2) * deltaTime;
		displacement.y = ((finalVelocity.y + damageNumber.velocity.y) / 2) * deltaTime;
		REF(deltaTime);

		damageNumber.velocity = finalVelocity;
		damageNumber.position.x += displacement.x;
		damageNumber.position.y += displacement.y;
	}
	else if (damageNumber.damageNumberT == DN_ASCENDING) {
		REF(deltaTime);

		finalVelocity.x = damageNumber.velocity.x;
		finalVelocity.y = damageNumber.velocity.y;

		Vector displacement = {};
		displacement.x = 0;
		displacement.y = -1.0;

		damageNumber.velocity = finalVelocity;
		damageNumber.position.x += displacement.x;
		damageNumber.position.y += displacement.y;
	}
	else {
		printf("ERROR: drawDamageNumber function (No valid damage number type selected)\n");
	}

	if (damageNumber.entityType == ENTITY_PLAYER) {
		color.r = 255;
		color.g = 0;
		color.b = 0;
	}
	if (damageNumber.entityType == ENTITY_ENEMY) {
		color.r = 255;
		color.g = 255;
		color.b = 255;
	}

	drawStringWorldSpace(color, gameData, gameData.renderer, textImage, damageNumber.textSize, damageNumber.damageString,
		(int)damageNumber.position.x, (int)damageNumber.position.y);
}

void drawFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha) {
	SDL_SetRenderDrawColor(renderer, (Uint8)red, (Uint8)green, (Uint8)blue, (Uint8)alpha);
	SDL_RenderFillRect(renderer, rect);
}

void drawNonFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha) {
	SDL_SetRenderDrawColor(renderer, (Uint8)red, (Uint8)green, (Uint8)blue, (Uint8)alpha);
	SDL_RenderDrawRect(renderer, rect);
}
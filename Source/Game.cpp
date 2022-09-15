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

Image loadImage(const char* fileName, int frames) {
	Image result;

	int x, y, n;
	unsigned char* data = stbi_load(fileName, &x, &y, &n, 4);

	R_Texture* texture = R_CreateTexture(R_PixelFormat_RGBA, x, y);

	int pitch;
	void* pixels;

	// Locking the texture gives us a position in memory
	// While the texture is locked, you can write to the texture
	R_LockTexture(texture, NULL, &pixels, &pitch);

	myMemcpy(pixels, data, ((size_t)x * (size_t)y * 4));

	// prevents leaking memory (memory is being leaked
	// we no longer want to free the memory
	// stbi_image_free(data);

	// When the texture is unlocked, you can no longer write to it
	R_UnlockTexture(texture);

	R_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	result.num_frames = frames;
	result.pixelData = data;
	result.texture = texture;
	result.w = x;
	result.h = y;

	return result;
}

Image loadFont(const char* fileName) {
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

	R_Texture* texture = R_CreateTexture(R_PixelFormat_RGBA, x, y);

	void* pixels;
	int pitch;
	int area = (x * y * 4);

	R_LockTexture(texture, NULL, &pixels, &pitch);

	myMemcpy(pixels, fileData, area);

	R_UnlockTexture(texture);

	R_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

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

double ClosestRotation(double current, double target)
{
	double result = 0;
	double angle = target - current;
	if (angle > 0)
	{
		if (angle < 180)
			result = angle;
		else
			result = angle - (2 * 180);
	}
	else if (angle < 0)
	{
		if (angle > - 180)
			result = angle;
		else
			result = angle + (2 * 180);
	}
	return result;
}

double sign(double value) {
	if (value >= 0)
		return 1;
	else
		return -1;
}

void drawTile(GameData& gameData, Tile tile, float perlin) {
	R_Rect rect;
	rect.w = (double)Constants::TILE_SIZE;
	rect.h = (double)Constants::TILE_SIZE;
	rect.x = tile.position.x;
	rect.y = tile.position.y;

	if (perlin > -0.1) {
		tile.tileType = TILE_GRASS;
	}
	else if (perlin > -.35) {
		tile.tileType = TILE_DIRT;
	}
	else {
		tile.tileType = TILE_ROCK;
	}

	R_RenderCopyEx(gameData.tileTypeArray[tile.tileType].texture, NULL, &rect, 0, NULL, SDL_FLIP_NONE);
}

void drawCircle(GameData& gameData, Vector position, double radius, int circleOffsetY) {
	REF(gameData);
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

		R_Rect rectangle1 = {};
		rectangle1.x = (int)x1;
		rectangle1.y = (int)y1;

		R_Rect rectangle2 = {};
		rectangle2.x = (int)x2;
		rectangle2.y = (int)y2;

		R_RenderDrawLine(rectangle1.x, rectangle1.y, rectangle2.x, rectangle2.y);

	}
}

void drawString(Color color, Image* textImage, int size, std::string string, double x, double y) {
	R_Rect sourceRect = {};
	// Font 1
	// int fontW = 7;
	// int fontH = 9;
	// Font 2 
	int fontW = 14;
	int fontH = 18;
	sourceRect.w = fontW;
	sourceRect.h = fontH;
	int stringSize = (int)string.size();
	double spacing = 0;

	for (int i = 0; i < stringSize; i++) {
		char glyph = string[i];
		int index = glyph - ' ';
		int row = index / fontH;
		int col = index % fontH;

		sourceRect.x = (col * fontW);
		sourceRect.y = (row * fontH);

		R_Rect destinationRect;

		destinationRect.x = x + spacing;

		destinationRect.y = y;

		destinationRect.w = fontW * size;
		destinationRect.h = fontH * size;

		R_SetTextureColorMod(textImage->texture, color.r, color.g, color.b);

		R_RenderCopy(textImage->texture, &sourceRect, &destinationRect);
		spacing += floor(destinationRect.w);
	}
}

void drawStringWorldSpace(Color color, GameData& gameData, Image* textImage, int size, std::string string, int x, int y) {
    REF(gameData);
	R_Rect destRect = {};
	destRect.x = x;
	destRect.y = y;
	drawString(color, textImage, size, string, destRect.x, destRect.y);
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
		// Assets/Audio/mixkit-punch-with-short-whistle-2049.wav
		// Assets/Audio/mixkit-air-in-a-hit-2161.wav
		// Assets/Audio/mixkit-light-impact-on-the-ground-2070.wav
		wav.load("Assets/Audio/mixkit-air-in-a-hit-2161.wav");
	}
	return enemyHitKey;
}

// Used inside draw damageNumber
void playEnemyHitSound(GameData& gameData) {
	std::string enemyHitKey = loadDamageNumberHitSound(gameData);
	auto result = gameData.soundFileUMap.find(enemyHitKey);
	if (result != gameData.soundFileUMap.end()) {
		int handle = gameData.soloud.play(result->second, 0.15f, 0, 1);
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
		// v = v0 + at
		finalVelocity.y = damageNumber.velocity.y + (Constants::GRAVITY * deltaTime);

		Vector displacement = {};
		// change in X (displacement) = (v + v0 / 2)t
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

	drawStringWorldSpace(color, gameData, textImage, damageNumber.textSize, damageNumber.damageString,
		(int)damageNumber.position.x, (int)damageNumber.position.y);
}

void drawFilledRectangle(R_Rect* rect, int red, int green, int blue, int alpha) {
	R_SetRenderDrawColor((Uint8)red, (Uint8)green, (Uint8)blue, (Uint8)alpha);
	R_RenderFillRect(rect);
}

void drawNonFilledRectangle(R_Rect* rect, int red, int green, int blue, int alpha) {
	R_SetRenderDrawColor((Uint8)red, (Uint8)green, (Uint8)blue, (Uint8)alpha);
	R_RenderDrawRect(rect);
}
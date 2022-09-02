#include "SDL.h"
#include "stb_image.h"
#include <stdio.h>
#include "Sprite.h"
#include <float.h>

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

double magnitude(Vector a) {
	return sqrt(a.x * a.x + a.y * a.y);
}

Vector normalize(Vector a) {
	double m = magnitude(a);
	Vector normalized = { a.x / m, a.y / m };
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

void createCharacter(GameData& gameData, Image image, int healthPoints, bool animated, int speed, int frames) {
	Character& character = gameData.player;
	character.position.x = Constants::RESOLUTION_X / 2;
	character.position.y = Constants::RESOLUTION_Y / 2;
	character.sprite = createSprite(image);
	character.radius = returnSpriteSize(image) / 2;
	character.hp = healthPoints;
	character.maxHP = healthPoints;
	character.animated = animated;
	character.speed = speed;
	character.frames = frames;
	character.experience = 0;
	character.level = 0;
	character.levelUp = 1000;
	character.newStaff(new ConsecratedGroundStaff(gameData, &gameData.player));;
}

float randomFloat(float min, float max) {
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

void updateExperienceOrbPosition(GameData& gameData, ExperienceOrb* experienceOrb, double speed, double delta) {
	Vector offset = {};
	offset = gameData.player.position - experienceOrb->position;
	experienceOrb->velocity = normalize(offset);
	experienceOrb->velocity *= speed;

	experienceOrb->position.x += experienceOrb->velocity.x * delta;
	experienceOrb->position.y += experienceOrb->velocity.y * delta;
}

double dotProduct(Vector a, Vector b) {
	return a.x * b.x + a.y * b.y;
}

void updateEnemyPosition(Character* player, Enemy* enemy, double delta) {
	// Direction you want to accelerate
	Vector enemyToPlayer = player->position - enemy->position;
	
	// Normalize to 1
	double length = sqrt(enemyToPlayer.x * enemyToPlayer.x + enemyToPlayer.y * enemyToPlayer.y);
	enemyToPlayer.x /= length;
	enemyToPlayer.y /= length;
	Vector enemyAcceleration = (enemyToPlayer * Constants::ENEMY_ACCELERATION);

	// Kinematic Equations
	// v = v0 + a0t
	Vector updatedVelocity = enemy->velocity + (enemyAcceleration * delta);
	double currentLength = sqrt(updatedVelocity.x * updatedVelocity.x + updatedVelocity.y * updatedVelocity.y);
	if (currentLength > Constants::ENEMY_SPEED) {
		if (dotProduct(updatedVelocity, enemyAcceleration) > 0) {
			updatedVelocity.x /= currentLength;
			updatedVelocity.y /= currentLength;

			updatedVelocity *= Constants::ENEMY_SPEED;
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

void drawEntity(GameData& gameData, Entity &entity) {
	SDL_Rect rect;
	// rect.x and y is an int. It is truncating the double when it is cast to an int -> (int)

	if (entity.sprite.width == 80 && entity.sprite.height == 80) {
		rect.w = entity.sprite.width;
		rect.h = entity.sprite.height;
	}
	else if (entity.sprite.width == 160 && entity.sprite.height == 160) {
		rect.w = entity.sprite.width / 2;
		rect.h = entity.sprite.height / 2;
	}
	else if (entity.sprite.width == 190 && entity.sprite.height == 190) {
		rect.w = entity.sprite.width / 2;
		rect.h = entity.sprite.height / 2;
	}
	else {
		rect.w = entity.sprite.width;
		rect.h = entity.sprite.height;
	}

	rect.x = (int)entity.position.x;
	rect.y = (int)entity.position.y;

	rect = convertCameraSpace(gameData.camera, rect);

	SDL_RenderCopyEx(gameData.renderer, entity.sprite.image.texture, NULL, &rect, entity.angle, NULL, SDL_FLIP_NONE);
}

void drawEntityAnimated(GameData& gameData, Entity& entity, bool right) {
	SDL_Rect srcRect = {};
	SDL_Rect destRect = {};

	srcRect.w = entity.sprite.width / entity.frames;
	srcRect.h = entity.sprite.height;
	srcRect.x = srcRect.w * (int)((SDL_GetTicks() / entity.speed) % entity.frames);

	if (srcRect.w == 80 && srcRect.h == 80) {
		destRect.w = srcRect.w;
		destRect.h = srcRect.h;
	}
	else if (srcRect.w == 160 && srcRect.h == 160) {
		destRect.w = srcRect.w / 2;
		destRect.h = srcRect.h / 2;
	}
	else if (srcRect.w == 190 && srcRect.h == 190) {
		destRect.w = srcRect.w / 2;
		destRect.h = srcRect.h / 2;
	}
	else {
		destRect.w = srcRect.w;
		destRect.h = srcRect.h;
	}

	destRect.x = (int)entity.position.x;
	destRect.y = (int)entity.position.y;

	destRect = convertCameraSpace(gameData.camera, destRect);
	if (right) {
		SDL_RenderCopyEx(gameData.renderer, entity.sprite.image.texture, &srcRect, &destRect, entity.angle, NULL, SDL_FLIP_HORIZONTAL);
	}
	else {
		SDL_RenderCopyEx(gameData.renderer, entity.sprite.image.texture, &srcRect, &destRect, entity.angle, NULL, SDL_FLIP_NONE);
	}
}

void drawCharacterIdle(GameData& gameData, Entity& entity, bool right) {
	SDL_Rect srcRect = {};
	SDL_Rect destRect = {};

	srcRect.w = entity.sprite.width / entity.frames;
	srcRect.h = entity.sprite.height;

	if (srcRect.w == 80 && srcRect.h == 80) {
		destRect.w = srcRect.w;
		destRect.h = srcRect.h;
	}
	else if (srcRect.w == 160 && srcRect.h == 160) {
		destRect.w = srcRect.w / 2;
		destRect.h = srcRect.h / 2 ;
	}
	else if (srcRect.w == 190 && srcRect.h == 190) {
		destRect.w = srcRect.w / 2;
		destRect.h = srcRect.h / 2;
	}
	else {
		destRect.w = srcRect.w;
		destRect.h = srcRect.h;
	}

	destRect.x = (int)entity.position.x;
	destRect.y = (int)entity.position.y;

	destRect = convertCameraSpace(gameData.camera, destRect);
	if (right) {
		SDL_RenderCopyEx(gameData.renderer, entity.sprite.image.texture, &srcRect, &destRect, entity.angle, NULL, SDL_FLIP_HORIZONTAL);
	}
	else {
		SDL_RenderCopyEx(gameData.renderer, entity.sprite.image.texture, &srcRect, &destRect, entity.angle, NULL, SDL_FLIP_NONE);
	}
}

void drawConsecratedGround(GameData& gameData, Entity& entity) {
	SDL_Rect rect;

	rect.w = entity.sprite.width;
	rect.h = entity.sprite.height;
	rect.x = (int)gameData.player.position.x;
	rect.y = (int)gameData.player.position.y;

	SDL_RenderCopyEx(gameData.renderer, entity.sprite.image.texture, NULL, &rect, entity.angle, NULL, SDL_FLIP_NONE);
}

int getRandomTile() {
	int randomNumber = rand() % TILE_COUNT;
	return randomNumber;
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

void drawProceduralTile(GameData& gameData, Image image, ProceduralTile tile, int totalTiles) {
	SDL_Rect rect = {};
	SDL_Rect src = {};
	Image resultImage = {};
	src.w = Constants::TILE_SIZE;
	src.h = Constants::TILE_SIZE;
	src.x = (image.w % totalTiles);

	rect.w = Constants::TILE_SIZE;
	rect.h = Constants::TILE_SIZE;
	rect.x = (int)tile.position.x;
	rect.y = (int)tile.position.y;

	rect = convertCameraSpace(gameData.camera, rect);

	SDL_RenderCopyEx(gameData.renderer, image.texture, &src, &rect, 0, NULL, SDL_FLIP_NONE);
}

void createEnemy(Image image, Vector position, GameData* gameData, int healthPoints, int damage, bool animated, int speed, int frames) {
	Enemy enemy = {};

	enemy.radius = returnSpriteSize(image) / 2;
	enemy.sprite = createSprite(image);
	enemy.position = position;
	enemy.hp = healthPoints;
	enemy.maxHP = healthPoints;
	enemy.damage = damage;
	enemy.timeUntilDamageDealt = 0;
	enemy.animated = animated;
	enemy.frames = frames;
	enemy.speed = speed;

	gameData->enemies.push_back(enemy);
}

Projectile createProjectile(WeaponType projectileType, Image image, int damage, int piercingLayers) {
	Projectile projectile = {};
	projectile.sprite = createSprite(image);
	projectile.radius = returnSpriteSize(image);
	projectile.lifeTime = 20;
	projectile.damage = damage;
	projectile.projectileType = projectileType;
	projectile.piercingLayer = piercingLayers;
	return projectile;
}

AOE createAOE(WeaponType aoeType, Image image, int damage, int lifeTime) {
	AOE aoe = {};
	aoe.sprite = createSprite(image);
	aoe.radius = returnSpriteSize(image);
	aoe.damage = damage;
	aoe.lifeTime = lifeTime;
	aoe.aoeType = aoeType;
	return aoe;
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
	sourceRect.w = 14;
	sourceRect.h = 18;
	int stringSize = (int)string.size();
	int spacing = 0;

	for (int i = 0; i < stringSize; i++) {
		char glyph = string[i];
		int index = glyph - ' ';
		int row = index / 18;
		int col = index % 18;

		sourceRect.x = (col * 14);
		sourceRect.y = (row * 18);

		SDL_Rect destinationRect;

		destinationRect.x = x + spacing;

		destinationRect.y = y;

		destinationRect.w = 14 * size;
		destinationRect.h = 18 * size;

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

DamageNumber createDamageNumber(EntityType type, int damageNumber, Vector position, Vector velocity, int textSize, double lifeTime) {
	DamageNumber result;

	result.entityType = type;
	result.damageString = std::to_string(damageNumber);
	result.position = position;
	result.velocity = velocity;
	result.textSize = textSize;
	result.lifeTime = lifeTime;

	return result;
}

void drawDamageNumber(GameData& gameData, DamageNumber& damageNumber, Image* textImage, double deltaTime) {
	Vector finalVelocity = {};
	Color color = {};

	finalVelocity.x = damageNumber.velocity.x;
	// Gravity only affects the Y axis
	finalVelocity.y = damageNumber.velocity.y + (Constants::GRAVITY * deltaTime);

	Vector displacement = {};
	displacement.x = ((finalVelocity.x + damageNumber.velocity.x) / 2) * deltaTime;
	displacement.y = ((finalVelocity.y + damageNumber.velocity.y) / 2) * deltaTime;

	damageNumber.velocity = finalVelocity;
	damageNumber.position.x += displacement.x;
	damageNumber.position.y += displacement.y;

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

	drawStringWorldSpace(color, gameData, gameData.renderer, textImage, damageNumber.textSize, damageNumber.damageString, (int)damageNumber.position.x, (int)damageNumber.position.y);
}

ExperienceOrb createExperienceOrb(Image image, double positionX, double positionY, double lifeTime) {
	ExperienceOrb result = {};

	result.sprite = createSprite(image);
	result.radius = returnSpriteSize(image);
	result.position.x = positionX;
	result.position.y = positionY;
	result.lifeTime = lifeTime;

	return result;
}

void drawFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha) {
	SDL_SetRenderDrawColor(renderer, (Uint8)red, (Uint8)green, (Uint8)blue, (Uint8)alpha);
	SDL_RenderFillRect(renderer, rect);
}

void drawNonFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha) {
	SDL_SetRenderDrawColor(renderer, (Uint8)red, (Uint8)green, (Uint8)blue, (Uint8)alpha);
	SDL_RenderDrawRect(renderer, rect);
}

void drawHealthBar(GameData& gameData, SDL_Renderer* renderer) {
	SDL_Rect remainingHP = {};
	SDL_Rect missingHP = {};
	SDL_Rect outlineHP = {};
	int remainingHPOffset = 0;
	int missingHPOffset = 0;

	double remainingHPPercent = (double)gameData.player.hp / (double)gameData.player.maxHP;
	remainingHP.w = (int)(Constants::HEALTH_BAR_W * remainingHPPercent);
	remainingHPOffset = (Constants::HEALTH_BAR_W - remainingHP.w) / 2;
	remainingHP.h = Constants::HEALTH_BAR_H;
	remainingHP.x = (int)(gameData.player.position.x - remainingHPOffset);
	remainingHP.y = (int)(gameData.player.position.y + 47);

	double missingHPPercent = 1 - remainingHPPercent;
	missingHP.w = (int)(Constants::HEALTH_BAR_W * missingHPPercent);
	missingHPOffset = (Constants::HEALTH_BAR_W - missingHP.w) / 2;
	missingHP.h = Constants::HEALTH_BAR_H;
	missingHP.x = (int)(gameData.player.position.x + missingHPOffset);
	missingHP.y = (int)(gameData.player.position.y + 47);

	outlineHP.w = Constants::HEALTH_BAR_W;
	outlineHP.h = Constants::HEALTH_BAR_H;
	outlineHP.x = (int)(gameData.player.position.x);
	outlineHP.y = (int)(gameData.player.position.y + 47);

	SDL_Rect remainingHPRect = convertCameraSpaceScreenWH(gameData.camera, remainingHP);
	drawFilledRectangle(renderer, &remainingHPRect, 255, 140, 0, 255);

	SDL_Rect missingHPRect = convertCameraSpaceScreenWH(gameData.camera, missingHP);
	drawFilledRectangle(renderer, &missingHPRect, 255, 0, 0, 255);

	SDL_Rect outlineHPRect = convertCameraSpaceScreenWH(gameData.camera, outlineHP);;
	drawNonFilledRectangle(renderer, &outlineHPRect, 0, 0, 0, 255);
}

void drawExperienceBar(GameData& gameData, SDL_Renderer* renderer) {
	SDL_Rect currentEXP = {};
	SDL_Rect missingEXP = {};
	SDL_Rect outlineEXP = {};
	int currentEXPOffset = 0;
	int missingEXPOffset = 0;

	double currentEXPPercent = (double)gameData.player.experience / (double)gameData.player.levelUp;
	currentEXP.w = (int)(Constants::EXP_BAR_W * currentEXPPercent);
	currentEXPOffset = (Constants::EXP_BAR_W - currentEXP.w) / 2;
	currentEXP.h = Constants::EXP_BAR_H;
	currentEXP.x = (int)(gameData.player.position.x - currentEXPOffset);
	currentEXP.y = (int)(gameData.player.position.y + 425);

	double missingEXPPercent = 1 - currentEXPPercent;
	missingEXP.w = (int)(Constants::EXP_BAR_W * missingEXPPercent);
	missingEXPOffset = (Constants::EXP_BAR_W - missingEXP.w) / 2;
	missingEXP.h = Constants::EXP_BAR_H;
	missingEXP.x = (int)(gameData.player.position.x + missingEXPOffset);
	missingEXP.y = (int)(gameData.player.position.y + 425);

	outlineEXP.w = Constants::EXP_BAR_W;
	outlineEXP.h = Constants::EXP_BAR_H;
	outlineEXP.x = (int)(gameData.player.position.x);
	outlineEXP.y = (int)(gameData.player.position.y + 425);

	SDL_Rect currentEXPRect = convertCameraSpaceScreenWH(gameData.camera, currentEXP);
	drawFilledRectangle(renderer, &currentEXPRect, 255, 127, 80, 255);

#if 0
	SDL_Rect missingEXPRect = convertCameraSpaceScreenWH(gameData.camera, missingEXP);
	drawFilledRectangle(renderer, &missingEXPRect, 255, 255, 255, 255);
#endif

	SDL_Rect outlineEXPRect = convertCameraSpaceScreenWH(gameData.camera, outlineEXP);;
	drawNonFilledRectangle(renderer, &outlineEXPRect, 0, 0, 0, 255);
}

void destroyEnemies(GameData& gameData) {
	for (int i = 0; i < gameData.enemies.size(); i++) {
		gameData.enemies[i].destroyed = true;
	}
}

void Character::newStaff(Staff* newStaff) {
	delete staff;
	staff = newStaff;
}

// All Spells
void Spell::setTarget(int spellSpeed) {
	if (mGameData.enemies.size() > 0) {
		Vector offset = {};
		int nearestEnemy = closestEnemy(this->mGameData.player, &mGameData);
		offset = mGameData.enemies[nearestEnemy].position - mGameData.player.position;
		setVelocity(normalize(offset) * spellSpeed);
		mAngle = angleFromDirection(mVelocity);
	}
}

void Spell::activateKnockback(Enemy* enemyTargeted) {
	Vector knockbackVector = {};
	if (magnitude(mVelocity) == 0) {
		knockbackVector = enemyTargeted->position - mPosition;
		// No suitable knockback vector for this case
		if (magnitude(knockbackVector) == 0) {
			knockbackVector = { 100, 100 };
		}
	}
	else {
		knockbackVector = mVelocity;
	}

	knockbackVector = normalize(knockbackVector);

	knockbackVector = knockbackVector * getKnockbackDistance();

	enemyTargeted->velocity.x += knockbackVector.x;
	enemyTargeted->velocity.y += knockbackVector.y;
}

void Spell::collision(Enemy* enemyTargeted) {
	double distanceBetween = distance(mPosition, enemyTargeted->position);
	double radiusSum = mRadius + enemyTargeted->radius;
	if (distanceBetween < radiusSum) {
		if (enemyTargeted->hp > 0) {
			int damageDealt = applyDamage(enemyTargeted);

			Vector numberVelocity = {};
			DamageNumber damageNumber = createDamageNumber(ENTITY_ENEMY, damageDealt, enemyTargeted->position,
				{ randomFloat(300, -300), randomFloat(-600, -300) }, Constants::DAMAGE_NUMBER_SIZE_E, Constants::DAMAGE_NUMBER_LIFETIME);
			mGameData.damageNumbers.push_back(damageNumber);

			mEnemyIds.push_back(enemyTargeted->mId);

			if (enemyTargeted->hp <= 0) {
				enemyTargeted->destroyed = true;
				// totalEnemiesKilled += 1;
				// Experience Orbs Vector
				static Image experienceOrbImage = loadImage(mGameData.renderer, "Assets/Experience_Orb_1.png");
				ExperienceOrb experienceOrb = createExperienceOrb(experienceOrbImage,
					enemyTargeted->position.x, enemyTargeted->position.y, Constants::EXPERIENCE_ORB_LIFETIME);
				mGameData.experienceOrbs.push_back(experienceOrb);
			}

			// Knock back enemies
			activateKnockback(enemyTargeted);
		}
	}
}

void Spell::draw(GameData& gameData) {
	Image* image = getImage(gameData);

	SDL_Rect rect;

	rect.w = image->w;
	rect.h = image->h;
	rect.x = (int)mPosition.x;
	rect.y = (int)mPosition.y;

	rect = convertCameraSpace(gameData.camera, rect);

	SDL_RenderCopyEx(gameData.renderer, image->texture, NULL, &rect, mAngle, NULL, SDL_FLIP_NONE);
}

void Spell::loadCastSound() {

}

void Spell::playCastSound() {

}

// Spike Spell
void SpikeSpell::setPosition(Vector position) {
	mPosition = position;
}

Image* SpikeSpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_Spike_2.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}

void SpikeSpell::loadCastSound() {
	mKey = "spikeCastSound";
	// Did not find the key
	if (mGameData.soundFileUMap.find(mKey) == mGameData.soundFileUMap.end()) {
		SoLoud::Wav& wav = mGameData.soundFileUMap[mKey];
		wav.load("Assets/Audio/mixkit-arrow-whoosh-1491.wav");
	}
	else {
		printf("Key already in vector\n");
	}
}

void SpikeSpell::playCastSound() {
	loadCastSound();
	auto result = mGameData.soundFileUMap.find(mKey);
	if (result != mGameData.soundFileUMap.end()) {
		int handle = mGameData.soloud.play(result->second, 0.05f, 0, 1);
		float playSpeed = 1.0f / (float)getSpellAttackDelay();
		mGameData.soloud.setRelativePlaySpeed(handle, playSpeed);
		mGameData.soloud.setPause(handle, 0);
	}
}

// Shadow Orb Spell
void ShadowOrbSpell::setPosition(Vector position) {
	mPosition = position;
}

Image* ShadowOrbSpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_ShadowOrb_1.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}

// Consecrated Ground Spell
void ConsecratedGroundSpell::setPosition(Vector position) {
	mPosition = position;
}

Image* ConsecratedGroundSpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_Consecrated_Ground_1.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}

// Fireball spell
void FireballSpell::setPosition(Vector position) {
	mPosition = position;
}

Image* FireballSpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_Fireball_1.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}

int FireballSpell::applyDamage(Enemy* enemyTargeted) {
	if (getPiercingLayers() == 0) {
		Spell* fireAOESPELL = new FireAOESpell(mGameData);
		fireAOESPELL->setPosition(mPosition);
		fireAOESPELL->setLifeTime(2);
		fireAOESPELL->setDamage(10);
		fireAOESPELL->setKnockBackDistance(100.0);
		fireAOESPELL->setAOEAttackDelay(1);
		mGameData.spells.push_back(fireAOESPELL);
		mLifeTime = 0;
	}
	else {
		--mPiercingLayers;
	}

	enemyTargeted->hp -= mDamage;
	return mDamage;
}

// AOE spell
void FireAOESpell::setPosition(Vector position) {
	mPosition = position;
}

Image* FireAOESpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_Fireball_AOE_6.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}
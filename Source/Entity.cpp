#include "Constants.h"
#include "Entity.h"
#include "Game.h"

void createCharacter(GameData& gameData, Image image, int healthPoints, bool animated, int speed, int frames) {
	gameData.player = new Character();
	gameData.player->sprite = createSprite(image);
	gameData.player->radius = returnSpriteSize(image) / 2;
	// Default pickup radius
	gameData.player->pickUpRadius = Constants::DEFAULT_PICKUP_RADIUS;
	gameData.player->hp = healthPoints;
	gameData.player->maxHP = healthPoints;
	gameData.player->animated = animated;
	gameData.player->speed = speed;
	gameData.player->frames = frames;
	gameData.player->experience = 0;
	gameData.player->level = 0;
	gameData.player->levelUp = 1000;
}

void updateEntityPosition(Entity* entity, double delta) {
	// This is where the player exists in the world at this frame
	// -> is short hand for dereferencing a member out of a pointer
	entity->position.x += entity->velocity.x * delta;
	entity->position.y += entity->velocity.y * delta;
}

void updateExperienceOrbPosition(GameData& gameData, ExperienceOrb* experienceOrb, double speed, double delta) {
	Vector offset = {};
	offset = gameData.player->position - experienceOrb->position;
	experienceOrb->velocity = normalize(offset);
	experienceOrb->velocity *= speed;

	experienceOrb->position.x += experienceOrb->velocity.x * delta;
	experienceOrb->position.y += experienceOrb->velocity.y * delta;
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


void drawEntity(GameData& gameData, Entity* entity) {
	SDL_Rect rect;
	// rect.x and y is an int. It is truncating the double when it is cast to an int -> (int)

	if (entity->sprite.width == 80 && entity->sprite.height == 80) {
		rect.w = entity->sprite.width;
		rect.h = entity->sprite.height;
	}
	else if (entity->sprite.width == 160 && entity->sprite.height == 160) {
		rect.w = entity->sprite.width / 2;
		rect.h = entity->sprite.height / 2;
	}
	else if (entity->sprite.width == 190 && entity->sprite.height == 190) {
		rect.w = entity->sprite.width / 2;
		rect.h = entity->sprite.height / 2;
	}
	else {
		rect.w = entity->sprite.width;
		rect.h = entity->sprite.height;
	}

	rect.x = (int)entity->position.x;
	rect.y = (int)entity->position.y;

	rect = convertCameraSpace(gameData.camera, rect);

	R_RenderCopyEx(entity->sprite.image.texture, NULL, &rect, entity->angle, NULL, SDL_FLIP_NONE);
}

void drawEntityAnimated(GameData& gameData, Entity* entity, bool facingDirection) {
	SDL_Rect srcRect = {};
	SDL_Rect destRect = {};

	srcRect.w = entity->sprite.width / entity->frames;
	srcRect.h = entity->sprite.height;

	Uint32 getTicks = SDL_GetTicks();

	srcRect.x = srcRect.w * (int)((getTicks / entity->speed) % entity->frames);

	if (srcRect.w == 160 && srcRect.h == 160) {
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

	destRect.x = (int)entity->position.x;
	destRect.y = (int)entity->position.y;

	destRect = convertCameraSpace(gameData.camera, destRect);
	// True if right
	if (facingDirection) {
		R_RenderCopyEx(entity->sprite.image.texture, &srcRect, &destRect, entity->angle, NULL, SDL_FLIP_HORIZONTAL);
	}
	else {
		R_RenderCopyEx(entity->sprite.image.texture, &srcRect, &destRect, entity->angle, NULL, SDL_FLIP_NONE);
	}
}

void drawDeathAnimation(GameData& gameData, DeathAnimation* deathAnimation, bool facingDirection) {
	if (deathAnimation->currentFrame < deathAnimation->frames) {
		SDL_Rect srcRect = {};
		SDL_Rect destRect = {};

		srcRect.w = deathAnimation->sprite.width / deathAnimation->frames;
		srcRect.h = deathAnimation->sprite.height;

		srcRect.x = srcRect.w * deathAnimation->currentFrame;

		destRect.w = srcRect.w;
		destRect.h = srcRect.h;

		destRect.x = (int)deathAnimation->position.x;
		destRect.y = (int)deathAnimation->position.y;

		destRect = convertCameraSpace(gameData.camera, destRect);

		// True = right
		if (facingDirection) {
			R_RenderCopyEx(deathAnimation->sprite.image.texture, &srcRect, &destRect, deathAnimation->angle, NULL, SDL_FLIP_HORIZONTAL);
		}
		else {
			R_RenderCopyEx(deathAnimation->sprite.image.texture, &srcRect, &destRect, deathAnimation->angle, NULL, SDL_FLIP_NONE);
		}

		// The frame we want it to pause on while being hit
		if (deathAnimation->currentFrame == 0 && deathAnimation->timesHit > 1) {
			if (deathAnimation->timeUntilNextFrame <= 0) {
				// timesHit * the value of the numbers spawned.
				deathAnimation->timeUntilNextFrame = deathAnimation->timesHit * 0.25;
				deathAnimation->currentFrame += 1;
			}
		}
		else {
			if (deathAnimation->timeUntilNextFrame <= 0) {
				// .1 = 100 miliseconds
				deathAnimation->timeUntilNextFrame = 0.1;
				deathAnimation->currentFrame += 1;
			}
		}
	}
}

void drawCharacterIdle(GameData& gameData, Entity* entity, bool right) {
	SDL_Rect srcRect = {};
	SDL_Rect destRect = {};

	srcRect.w = entity->sprite.width / entity->frames;
	srcRect.h = entity->sprite.height;

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

	destRect.x = (int)entity->position.x;
	destRect.y = (int)entity->position.y;

	destRect = convertCameraSpace(gameData.camera, destRect);
	if (right) {
		R_RenderCopyEx(entity->sprite.image.texture, &srcRect, &destRect, entity->angle, NULL, SDL_FLIP_HORIZONTAL);
	}
	else {
		R_RenderCopyEx(entity->sprite.image.texture, &srcRect, &destRect, entity->angle, NULL, SDL_FLIP_NONE);
	}
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

void createDeathAnimation(Image image, Vector position, GameData& gameData, int frames, int timesHit) {
	DeathAnimation deathAnimation = {};

	deathAnimation.sprite = createSprite(image);
	deathAnimation.position = position;
	deathAnimation.frames = frames;
	deathAnimation.currentFrame = 0;
	deathAnimation.timesHit = timesHit;

	gameData.deathAnimations.push_back(deathAnimation);
}

int closestEnemy(Character* player, GameData* gameData) {
	double closestDistance = DBL_MAX;
	int index = -1;
	for (int i = 0; i < gameData->enemies.size(); i++) {
		double distanceBetween = distance(player->position, gameData->enemies[i].position);
		if (distanceBetween < closestDistance) {
			closestDistance = distanceBetween;
			index = i;
		}
	}
	return index;
}

int closestEnemyMS(Vector position, GameData* gameData) {
	double closestDistance = DBL_MAX;
	int index = -1;
	for (int i = 0; i < gameData->enemies.size(); i++) {
		// If enemy targeted
		if (!gameData->enemies[i].targeted) {
			double distanceBetween = distance(position, gameData->enemies[i].position);
			if (distanceBetween < closestDistance) {
				closestDistance = distanceBetween;
				index = i;
			}
		}
	}
	return index;
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

/*
void activateExpKnockback(Character* player, ExperienceOrb* expOrb, double knockbackDistance) {
	Vector knockbackVector = {};
	if (magnitude(expOrb->velocity) == 0) {
		knockbackVector = expOrb->position - player->position;
		// No suitable knockback vector for this case
		if (magnitude(knockbackVector) == 0) {
			knockbackVector = { 100, 100 };
		}
	}
	else {
		knockbackVector = expOrb->velocity;
	}

	knockbackVector = normalize(knockbackVector);

	knockbackVector = knockbackVector * knockbackDistance;

	expOrb->velocity.x += knockbackVector.x;
	expOrb->velocity.y += knockbackVector.y;
}
*/

void drawHealthBar(GameData& gameData) {
	SDL_Rect remainingHP = {};
	SDL_Rect missingHP = {};
	SDL_Rect outlineHP = {};
	int remainingHPOffset = 0;
	int missingHPOffset = 0;

	double remainingHPPercent = (double)gameData.player->hp / (double)gameData.player->maxHP;
	remainingHP.w = (int)(Constants::HEALTH_BAR_W * remainingHPPercent);
	remainingHPOffset = (Constants::HEALTH_BAR_W - remainingHP.w) / 2;
	remainingHP.h = Constants::HEALTH_BAR_H;
	remainingHP.x = (int)(gameData.player->position.x - remainingHPOffset);
	remainingHP.y = (int)(gameData.player->position.y + 47);

	double missingHPPercent = 1 - remainingHPPercent;
	missingHP.w = (int)(Constants::HEALTH_BAR_W * missingHPPercent);
	missingHPOffset = (Constants::HEALTH_BAR_W - missingHP.w) / 2;
	missingHP.h = Constants::HEALTH_BAR_H;
	missingHP.x = (int)(gameData.player->position.x + missingHPOffset);
	missingHP.y = (int)(gameData.player->position.y + 47);

	outlineHP.w = Constants::HEALTH_BAR_W;
	outlineHP.h = Constants::HEALTH_BAR_H;
	outlineHP.x = (int)(gameData.player->position.x);
	outlineHP.y = (int)(gameData.player->position.y + 47);

	SDL_Rect remainingHPRect = convertCameraSpaceScreenWH(gameData.camera, remainingHP);
	drawFilledRectangle(&remainingHPRect, 255, 140, 0, 255);

	SDL_Rect missingHPRect = convertCameraSpaceScreenWH(gameData.camera, missingHP);
	drawFilledRectangle(&missingHPRect, 255, 0, 0, 255);

	SDL_Rect outlineHPRect = convertCameraSpaceScreenWH(gameData.camera, outlineHP);;
	drawNonFilledRectangle(&outlineHPRect, 0, 0, 0, 255);
}

void drawExperienceBar(GameData& gameData) {
	SDL_Rect currentEXP = {};
	SDL_Rect missingEXP = {};
	SDL_Rect outlineEXP = {};
	int currentEXPOffset = 0;
	int missingEXPOffset = 0;

	double currentEXPPercent = (double)gameData.player->experience / (double)gameData.player->levelUp;
	currentEXP.w = (int)(Constants::EXP_BAR_W * currentEXPPercent);
	currentEXPOffset = (Constants::EXP_BAR_W - currentEXP.w) / 2;
	currentEXP.h = Constants::EXP_BAR_H;
	currentEXP.x = (int)(gameData.player->position.x - currentEXPOffset);
	currentEXP.y = (int)(gameData.player->position.y + 425);

	double missingEXPPercent = 1 - currentEXPPercent;
	missingEXP.w = (int)(Constants::EXP_BAR_W * missingEXPPercent);
	missingEXPOffset = (Constants::EXP_BAR_W - missingEXP.w) / 2;
	missingEXP.h = Constants::EXP_BAR_H;
	missingEXP.x = (int)(gameData.player->position.x + missingEXPOffset);
	missingEXP.y = (int)(gameData.player->position.y + 425);

	outlineEXP.w = Constants::EXP_BAR_W;
	outlineEXP.h = Constants::EXP_BAR_H;
	outlineEXP.x = (int)(gameData.player->position.x);
	outlineEXP.y = (int)(gameData.player->position.y + 425);

	SDL_Rect currentEXPRect = convertCameraSpaceScreenWH(gameData.camera, currentEXP);
	drawFilledRectangle(&currentEXPRect, 255, 127, 80, 255);

#if 0
	SDL_Rect missingEXPRect = convertCameraSpaceScreenWH(gameData.camera, missingEXP);
	drawFilledRectangle(&missingEXPRect, 255, 255, 255, 255);
#endif

	SDL_Rect outlineEXPRect = convertCameraSpaceScreenWH(gameData.camera, outlineEXP);;
	drawNonFilledRectangle(&outlineEXPRect, 0, 0, 0, 255);
}

void destroyEnemies(GameData& gameData) {
	for (int i = 0; i < gameData.enemies.size(); i++) {
		gameData.enemies[i].destroyed = true;
	}
}

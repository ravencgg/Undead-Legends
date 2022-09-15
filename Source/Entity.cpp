#include "Constants.h"
#include "Entity.h"
#include "Game.h"

R_Rect MakeEntityRect(Entity* entity)
{
	return MakeRect(entity->position, &entity->sprite);
}

void createCharacter(GameData& gameData, Image image, int healthPoints, bool animated, int speed) {
	gameData.player = new Character();
	gameData.player->sprite = createSprite(image);
	gameData.player->radius = returnSpriteSize(image) / 2;
	// Default pickup radius
	gameData.player->pickUpRadius = Constants::DEFAULT_PICKUP_RADIUS;
	gameData.player->hp = healthPoints;
	gameData.player->maxHP = healthPoints;
	gameData.player->animated = animated;
	gameData.player->speed = speed;
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
	REF(gameData);
	R_Rect rect = MakeEntityRect(entity);
	R_RenderCopyEx(entity->sprite.image.texture, NULL, &rect, entity->angle, NULL, SDL_FLIP_NONE);
}

void drawEntityAnimated(GameData& gameData, Entity* entity, bool facingDirection) {
	REF(gameData);
	R_Rect srcRect = {};
	R_Rect destRect = MakeEntityRect(entity);

	int frames = entity->sprite.image.num_frames;
	if (frames == 0) frames = 1;
	srcRect.w = entity->sprite.width / frames;
	srcRect.h = entity->sprite.height;

	Uint32 getTicks = SDL_GetTicks();
	srcRect.x = srcRect.w * (int)((getTicks / entity->speed) % frames);

	// True if right
	if (facingDirection) {
		R_RenderCopyEx(entity->sprite.image.texture, &srcRect, &destRect, entity->angle, NULL, SDL_FLIP_HORIZONTAL);
	}
	else {
		R_RenderCopyEx(entity->sprite.image.texture, &srcRect, &destRect, entity->angle, NULL, SDL_FLIP_NONE);
	}
}

void drawDeathAnimation(GameData& gameData, DeathAnimation* deathAnimation, bool facingDirection) {
    REF(gameData);
	int frames = deathAnimation->sprite.image.num_frames;
	if (deathAnimation->currentFrame < frames) {
		R_Rect srcRect = {};
		R_Rect destRect = MakeEntityRect(deathAnimation);

		srcRect.w = deathAnimation->sprite.width / frames;
		srcRect.h = deathAnimation->sprite.height;

		srcRect.x = srcRect.w * deathAnimation->currentFrame;

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
	REF(gameData);
	R_Rect srcRect = {};
	R_Rect destRect = MakeEntityRect(entity);

	int frames = entity->sprite.image.num_frames;
	srcRect.w = entity->sprite.width / frames;
	srcRect.h = entity->sprite.height;

	if (right) {
		R_RenderCopyEx(entity->sprite.image.texture, &srcRect, &destRect, entity->angle, NULL, SDL_FLIP_HORIZONTAL);
	}
	else {
		R_RenderCopyEx(entity->sprite.image.texture, &srcRect, &destRect, entity->angle, NULL, SDL_FLIP_NONE);
	}
}


void createEnemy(Image image, Vector position, GameData* gameData, int healthPoints, int damage, bool animated, int speed) {
	Enemy enemy = {};

	enemy.radius = returnSpriteSize(image) / 2;
	enemy.sprite = createSprite(image);
	enemy.position = position;
	enemy.hp = healthPoints;
	enemy.maxHP = healthPoints;
	enemy.damage = damage;
	enemy.timeUntilDamageDealt = 0;
	enemy.animated = animated;
	enemy.speed = speed;

	gameData->enemies.push_back(enemy);
}

void createDeathAnimation(Image image, Vector position, GameData& gameData, int timesHit) {
	DeathAnimation deathAnimation = {};

	deathAnimation.sprite = createSprite(image);
	deathAnimation.position = position;
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

	const Vector hp_size = { (double)Constants::HEALTH_BAR_W, (double)Constants::HEALTH_BAR_H };
	double remainingHPPercent = (double)gameData.player->hp / (double)gameData.player->maxHP;

	Vector hp_offset = { 0, 47 };
	R_Rect hp_rect = MakeRect(gameData.player->position + hp_offset, hp_size);
	R_Rect filled_rect = hp_rect;
	filled_rect.w = (int)(filled_rect.w * remainingHPPercent);

	R_Rect empty_rect = hp_rect;
	empty_rect.w = hp_rect.w - filled_rect.w;
	empty_rect.x = filled_rect.x + filled_rect.w;

	drawFilledRectangle(&filled_rect, 255, 140, 0, 255);
	drawFilledRectangle(&empty_rect, 255, 0, 0, 255);
	drawNonFilledRectangle(&hp_rect, 0, 0, 0, 255);
}

void drawExperienceBar(GameData& gameData) {
	R_Rect currentEXP = {};
	R_Rect missingEXP = {};
	R_Rect outlineEXP = {};

	double currentEXPPercent = (double)gameData.player->experience / (double)gameData.player->levelUp;
	currentEXP.w = (int)(Constants::EXP_BAR_W * currentEXPPercent);
	double currentEXPOffset = ((double)Constants::EXP_BAR_W - currentEXP.w) / 2.0;
	currentEXP.h = Constants::EXP_BAR_H;
	currentEXP.x = (int)(gameData.player->position.x - currentEXPOffset) - currentEXP.w / 2;
	currentEXP.y = (int)(gameData.player->position.y + 425);

	double missingEXPPercent = 1 - currentEXPPercent;
	missingEXP.w = (int)(Constants::EXP_BAR_W * missingEXPPercent);
	double missingEXPOffset = (Constants::EXP_BAR_W - missingEXP.w) / 2.0;
	missingEXP.h = Constants::EXP_BAR_H;
	missingEXP.x = (int)(gameData.player->position.x + missingEXPOffset);
	missingEXP.y = (int)(gameData.player->position.y + 425);

	outlineEXP.w = Constants::EXP_BAR_W;
	outlineEXP.h = Constants::EXP_BAR_H;
	outlineEXP.x = (int)(gameData.player->position.x) - outlineEXP.w / 2;
	outlineEXP.y = (int)(gameData.player->position.y + 425);

	drawFilledRectangle(&currentEXP, 255, 127, 80, 255);

#if 0
	drawFilledRectangle(&missingEXPRect, 255, 255, 255, 255);
#endif

	drawNonFilledRectangle(&outlineEXP, 0, 0, 0, 255);
}

void destroyEnemies(GameData& gameData) {
	for (int i = 0; i < gameData.enemies.size(); i++) {
		gameData.enemies[i].destroyed = true;
	}
}

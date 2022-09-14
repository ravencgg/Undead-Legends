#pragma once
#include <string>
#include "Tools.h"

class GameData;
class Spell;
class Staff;

class Entity {
	public:
		Sprite					sprite;
		Vector					position;
		Vector					velocity;

		double					angle = 0;
		double					radius = 0;

		int						hp = 0;
		int						maxHP = 0;

		int						frames = 0;
		int						speed = 0;
		bool					animated = false;

		uint32_t				mId = 0;

		Entity() {
			static uint32_t previousId = 1;
			mId = previousId++;
		}
};

class Character : public Entity {
	public:
		int						experience = 0;
		int						level = 0;
		int						levelUp = 0;

		double					pickUpRadius = 0.0;

		Staff* staff = nullptr;

		// Definition in Staff.cpp
		void newStaff(Staff* newStaff);
};

class Enemy : public Entity {
	public:
		bool					destroyed = false;
		int						damage = 0;
		double					timeUntilDamageDealt = 0.0;
		double					timeUntilDamageTaken = 0.0;
		int						timesHit = 1;
		bool					targeted = false;
};

class DeathAnimation : public Entity {
	public:
		int						currentFrame = 0;
		double					timeUntilNextFrame = 0.0;
		int						timesHit = 0;
};

class Weapon : public Entity {
	public:
		double					lifeTime = 0.0;
		int						damage = 0;
};

class ExperienceOrb : public Entity {
	public:
		double					lifeTime = 0.0;
		int						experienceGained = 0;
};

void createCharacter(GameData& gameData, Image image, int healthPoints, bool animated, int speed, int frames);

void updateEntityPosition(Entity* entity, double delta);

void drawEntity(GameData& gameData, Entity* entity);

void drawEntityAnimated(GameData& gameData, Entity* entity, bool facingDirection);

void drawDeathAnimation(GameData& gameData, DeathAnimation* deathAnimation, bool facingDirection);

void drawCharacterIdle(GameData& gameData, Entity* entity, bool right);

void createEnemy(Image image, Vector position, GameData* gameData, int healthPoints, int damage, bool animated, int speed, int frames);

void createDeathAnimation(Image image, Vector position, GameData& gameData, int frames, int timesHit);

int closestEnemy(Character* player, GameData* gameData);

int closestEnemyMS(Vector position, GameData* gameData);

void updateExperienceOrbPosition(GameData& gameData, ExperienceOrb* experienceOrb, double speed, double delta);

void updateEnemyPosition(Character* player, Enemy* enemy, double delta);

ExperienceOrb createExperienceOrb(Image image, double positionX, double positionY, double lifeTime);

// void activateExpKnockback(Character* player, ExperienceOrb* expOrb, double knockbackDistance);

void drawHealthBar(GameData& gameData, SDL_Renderer* renderer);

void drawExperienceBar(GameData& gameData, SDL_Renderer* renderer);

void destroyEnemies(GameData& gameData);
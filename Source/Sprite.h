#pragma once
#include "Render.h"

#include <vector>
#include <string>

const int RESOLUTION_X = 1600;
const int RESOLUTION_Y = 900;
const double ENEMY_SPEED = 50;
const double ENEMY_ACCELERATION = 2500;
const int TILE_SIZE = 32;
const double GRAVITY = 2000;
const int HEALTH_BAR_W = 55;
const int HEALTH_BAR_H = 8;
const int EXP_BAR_W = RESOLUTION_X - 20;
const int EXP_BAR_H = 25;

struct Vector {
	double		x;
	double		y;
};

double getTime();

Vector facingDirection(double theta);

Vector normalize(Vector a);

Vector operator+(Vector a, Vector b);

Vector operator-(Vector a, Vector b);

Vector& operator*=(Vector& a, double b);

Vector operator*(Vector a, double b);

double angleFromDirection(Vector a);

enum TileType {
	TILE_ROCK,
	TILE_DIRT,
	TILE_GRASS,

	TILE_COUNT
};

enum EntityType {
	ENTITY_PLAYER,
	ENTITY_ENEMY,

	ENTITY_COUNT
};

enum WeaponType {
	WT_PROJECTILE_SPIKE,
	WT_PROJECTILE_SHADOW_ORB,
	WT_PROJECTILE_FIREBALL,

	WT_AOE_CONSECRATED_GROUND,
	WT_AOE_RESIDUAL_FIREBALL,

	WEAPON_TYPE_COUNT
};

struct Image {
	unsigned char*	pixelData;
	SDL_Texture*	texture;
	int				w;
	int				h;
};

struct Color {
	unsigned char	r, g, b, a;
};

struct Sprite {
	Image		image;
	int			width;
	int			height;
};

struct DamageNumber {
	std::string	damageString;
	Vector		position;
	Vector		velocity;
	int			textSize;
	double		lifeTime;
	EntityType	entityType;
};

struct Entity {
	Sprite		sprite;
	Vector		position;
	Vector		velocity;

	double		angle;
	double		radius;

	int			hp;
	int			maxHP;

	int			frames;
	int			speed;
	bool		animated;
};

struct Character : Entity {
	int			experience;
	int			level;
	int			levelUp;
};

struct Enemy : Entity {
	bool		destroyed = false;
	int			damage;
	double		timeUntilDamageDealt;
	double		timeUntilDamageTakenAOE;
	double		timeUntilDamageTakenProjectile;
};

struct Weapon : Entity {
	double		lifeTime;
	int			damage;
};

struct AOE : Weapon {
	WeaponType	aoeType;
};

struct Projectile : Weapon {
	int			piercingLayer;
	WeaponType	projectileType;
};

struct ExperienceOrb : Entity {
	double		lifeTime;
	int			experienceGained;
};

struct Camera {
	Vector				position;
};

struct Tile {
	TileType			tileType;
	Vector				position;
};

struct ProceduralTile {
	Vector				position;
	int					type;
};

struct GameData {
	SDL_Renderer*				renderer;
	Character					player;
	Camera						camera;
	std::vector<Enemy>			enemies;
	std::vector<Projectile>		projectiles;
	std::vector<AOE>			aoe;
	std::vector<DamageNumber>	damageNumbers;
	std::vector<ExperienceOrb>	experienceOrbs;
	Image						tileTypeArray[TILE_COUNT];
};

struct Font {
	
};

void myMemcpy(void* destination, void const* source, size_t size);

Sprite createSprite(Image image);

Color readPixel(Image image, int x, int y);

double distance(Vector a, Vector b);

double distancePlayer(Vector a, Vector b);

Image loadImage(SDL_Renderer* renderer, const char* fileName);

Image loadFont(SDL_Renderer* renderer, const char* fileName);

double returnSpriteSize(Image image);

Character createCharacter(GameData& gameData, Image image, int healthPoints, bool animated, int speed, int frames);

float randomFloat(float min, float max);

void updateEntityPosition(Entity* entity, double delta);

void updateExperienceOrbPosition(GameData& gameData, ExperienceOrb* experienceOrb, double speed, double delta);

double dotProduct(Vector a, Vector b);

void updateEnemyPosition(Character* player, Enemy* enemy, double delta);

SDL_Rect convertCameraSpace(Camera& camera, SDL_Rect worldSpace);

SDL_Rect convertCameraSpaceScreenWH(Camera& camera, SDL_Rect worldSpace);

void drawEntity(GameData& gameData, Entity& entity);

void drawEntityAnimated(GameData& gameData, Entity& entity, bool right);

void drawCharacterIdle(GameData& gameData, Entity& entity, bool right);

// void drawConsecratedGround(GameData& gameData, Entity& entity);

int getRandomTile();

void drawTile(GameData& gameData, Tile tile, float perlin);

void drawProceduralTile(GameData& gameData, Image image, ProceduralTile tile, int totalTiles);

void createEnemy(Image image, Vector position, GameData* gameData, int healthPoints, int damage, bool animated, int speed, int frames);

Projectile createProjectile(WeaponType projectileType, Image image, int damage, int piercingLayer);

AOE createAOE(WeaponType aoeType, Image image, int damage, int duration);

// AOE createAOEWorldSpace(Image image, int damage, int duration);

int closestEnemy(Character player, GameData* gameData);

void drawCircle(GameData& gameData, Vector position, double radius, int circleOffsetY);

void drawString(Color color, GameData& gameData, SDL_Renderer* renderer, Image* textImage, int size, std::string string, int x, int y);

void drawStringWorldSpace(Color color, GameData& gameData, SDL_Renderer* renderer, Image* textImage, int size, std::string string, int x, int y);

DamageNumber createDamageNumber(EntityType type, int damageNumber, Vector position, Vector velocity, int textSize, double lifeTime);

void drawDamageNumber(GameData& gameData, DamageNumber &damageNumber, Image* textImage, double deltaTime);

ExperienceOrb createExperienceOrb(GameData& gameData, Image image, double positionX, double positionY, double lifeTime);

void drawFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha);

void drawNonFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha);

void drawHealthBar(GameData& gameData, SDL_Renderer* renderer);

void drawExperienceBar(GameData& gameData, SDL_Renderer* renderer);

void destroyEnemies(GameData& gameData);

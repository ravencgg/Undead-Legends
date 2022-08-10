#pragma once
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
	TILE_GRASS,
	TILE_DIRT,

	TILE_COUNT
};

enum EntityType {
	ENTITY_PLAYER,
	ENTITY_ENEMY,

	ENTITY_COUNT
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
};

struct Character : Entity {
};

struct Enemy : Entity {
	bool		destroyed = false;
	int			damage;
	double		timeUntilDamage;
};

struct Weapon : Entity {
	double		lifeTime;
	int			damage;
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

struct GameData {
	SDL_Renderer*		renderer;

	Character					player;
	Camera						camera;
	std::vector<Enemy>			enemies;
	std::vector<Weapon>			weapon;
	std::vector<DamageNumber>	damageNumbers;
	std::vector<ExperienceOrb>	experienceOrbs;
	Image						tileTypeArray[TILE_COUNT];
};

void myMemcpy(void* destination, void const* source, size_t size);

Sprite createSprite(Image image);

Color readPixel(Image image, int x, int y);

double distance(Vector a, Vector b);

double distancePlayer(Vector a, Vector b);

Image loadImage(SDL_Renderer* renderer, const char* fileName);

Image loadFont(SDL_Renderer* renderer, const char* fileName);

double returnSpriteSize(Image image);

Character createCharacter(Image image, int healthPoints);

float randomFloat(float min, float max);

void updateEntityPosition(Entity* entity, double delta);

double dotProduct(Vector a, Vector b);

void updateEnemyPosition(Character* player, Enemy* enemy, double delta);

SDL_Rect convertCameraSpace(Camera& camera, SDL_Rect worldSpace);

SDL_Rect convertCameraSpaceScreenWH(Camera& camera, SDL_Rect worldSpace);

void drawEntity(GameData& gameData, Entity& entity);

int getRandomTile();

void drawTile(GameData& gameData, Tile tile);

void createEnemy(Image image, Vector position, GameData* gameData, int healthPoints, int damage);

Weapon createWeapon(Image image, int damage);

int closestEnemy(Character player, GameData* gameData);

void drawCircle(GameData& gameData, Vector position, double radius, int circleOffsetY);

void drawString(Color color, GameData& gameData, SDL_Renderer* renderer, Image* textImage, int size, std::string string, int x, int y);

DamageNumber createDamageNumber(EntityType type, int damageNumber, Vector position, Vector velocity, int textSize, double lifeTime);

void drawDamageNumber(GameData& gameData, DamageNumber &damageNumber, Image* textImage, double deltaTime);

ExperienceOrb createExperienceOrb(GameData& gameData, Image image, double positionX, double positionY, double lifeTime);

void drawFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha);

void drawNonFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha);

void drawHealthBar(GameData& gameData, SDL_Renderer* renderer);

void destroyEnemies(GameData& gameData);
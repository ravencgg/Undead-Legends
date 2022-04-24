#pragma once
#include <vector>

const int RESOLUTION_X = 1600;
const int RESOLUTION_Y = 900;
const double ENEMY_SPEED = 50;
const double ENEMY_ACCELERATION = 250;

// Extern says this exists but the linker has to
// find out where it is. Bad practice!! Pass through
// functions in the future.

struct Vector {
	double x;
	double y;
};

double getTime();

Vector facingDirection(double theta);

Vector normalize(Vector a);

Vector operator+(Vector a, Vector b);

Vector operator-(Vector a, Vector b);

Vector& operator*=(Vector& a, double b);

Vector operator*(Vector a, double b);

double angleFromDirection(Vector a);

struct Image {
	unsigned char* pixelData;
	SDL_Texture* texture;
	int w;
	int h;
};

struct Color {
	unsigned char r, g, b, a;
};

// struct is a group of variables of different types (good for refering to data of different types)
struct Sprite {
	// SDL_Rect rect;
	// SDL_Texture* texture;
	Image image;
	double angle;
	Vector position;
	// Vector stores x and y
	Vector velocity;
	int width;
	int height;
};

struct Character {
	Sprite sprite;
	double radius;
	int healthPoints;
};

struct Enemy {
	Sprite sprite;
	double radius;
	bool destroyed = false;
	int healthPoints;
	int damage;
	double timeUntilDamage;
	Vector velocity;
};

struct Weapon {
	Sprite sprite;
	double lifeTime;
	double radius;
	int damage;
};

struct GameData {
	Character player;
	std::vector<Enemy> enemies;
	std::vector<Weapon> weaponSpike;
};

void myMemcpy(void* destination, void const* source, size_t size);

Sprite createSprite(Image image);

Color readPixel(Image image, int x, int y);

double distance(Vector a, Vector b);

Image loadImage(SDL_Renderer* renderer, const char* fileName);

double returnSpriteSize(Image image);

Character createCharacter(Image image, int healthPoints);

float randomFloat(float min, float max);

void updateSpritePosition(Sprite* sprite, double delta);

void updateEnemyPosition(Character* player, Enemy* enemy, double delta);

void drawSprite(SDL_Renderer* renderer, Sprite sprite);

void createEnemy(Image image, Vector position, GameData* gameData, int healthPoints, int damage);

Weapon createWeapon(Image image, int damage);

int closestEnemy(Character player, GameData* gameData);

void drawCircle(SDL_Renderer* renderer, Vector position, float radius);
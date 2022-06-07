#pragma once
#include <vector>

const int RESOLUTION_X = 1600;
const int RESOLUTION_Y = 900;
const double ENEMY_SPEED = 100;
const double ENEMY_ACCELERATION = 2500;

// Extern says this exists but the linker has to
// find out where it is. Bad practice!! Pass through
// functions in the future.

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

struct Entity {
	Sprite		sprite;
	Vector		position;
	Vector		velocity;

	double		angle;
	double		radius;

	int			hp;
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

struct Camera {
	Vector				position;
};

struct Renderer {
	
};

struct GameData {
	SDL_Renderer*		renderer;

	Character			player;
	Camera				camera;
	std::vector<Enemy>	enemies;
	std::vector<Weapon> weaponSpike;
};

void myMemcpy(void* destination, void const* source, size_t size);

Sprite createSprite(Image image);

Color readPixel(Image image, int x, int y);

double distance(Vector a, Vector b);

double distancePlayer(Vector a, Vector b);

Image loadImage(SDL_Renderer* renderer, const char* fileName);

double returnSpriteSize(Image image);

Character createCharacter(Image image, int healthPoints);

float randomFloat(float min, float max);

float randomFloatScreen(float min, float max);

void updateEntityPosition(Entity* entity, double delta);

float dotProduct(Vector a, Vector b);

void updateEnemyPosition(Character* player, Enemy* enemy, double delta);

SDL_Rect convertCameraSpace(Camera& camera, SDL_Rect worldSpace);

void drawEntity(GameData& gameData, Entity& entity);

void createEnemy(Image image, Vector position, GameData* gameData, int healthPoints, int damage);

Weapon createWeapon(Image image, int damage);

int closestEnemy(Character player, GameData* gameData);

// Circle offset doesn't make sense
void drawCircle(GameData& gameData, Vector position, float radius, int circleOffsetY);
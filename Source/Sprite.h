#pragma once
#include "Render.h"
#include "Constants.h"
#include <vector>
#include <string>
#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_audiosource.h>
#include <unordered_map>

struct Vector {
	double		x = 0;
	double		y = 0;
};

double getTime();

Vector facingDirection(double theta);

double magnitude(Vector a);

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
	unsigned char*	pixelData = nullptr;
	SDL_Texture*	texture = nullptr;
	int				w = 0;
	int				h = 0;
};

struct Color {
	unsigned char	r, g, b, a;
};

struct Sprite {
	Image		image;
	int			width = 0;
	int			height = 0;
};

struct DamageNumber {
	std::string	damageString;
	Vector		position;
	Vector		velocity;
	int			textSize = 0;
	double		lifeTime = 0;
	EntityType	entityType;
};

// Forward declare the class. Only works for pointers
// and reference types. NOT value types.
class Staff;

class Entity {
	public:
		Sprite		sprite;
		Vector		position;
		Vector		velocity;

		double		angle = 0;
		double		radius = 0;

		int			hp = 0;
		int			maxHP = 0;

		int			frames = 0;
		int			speed = 0;
		bool		animated = false;	

		uint32_t	mId = 0;

		Entity() {
			// massive unsigned integer
			static uint32_t previousId = 1;
			mId = previousId++;
		}
};

class Character : public Entity {
	public:
		int			experience = 0;
		int			level = 0;
		int			levelUp = 0;

		Staff*		staff = nullptr;

		void newStaff(Staff* newStaff);
};

struct Enemy : Entity {
	bool		destroyed = false;
	int			damage;
	double		timeUntilDamageDealt;
	double		timeUntilDamageTaken = 0;
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

class Spell;

struct GameData {
	SDL_Renderer*				renderer;
	Character					player;
	Camera						camera;
	std::vector<Enemy>			enemies;
	std::vector<Projectile>		projectiles;
	std::vector<AOE>			aoe;
	std::vector<Spell*>			spells;
	std::vector<DamageNumber>	damageNumbers;
	std::vector<ExperienceOrb>	experienceOrbs;
	SoLoud::Soloud				soloud;
	std::unordered_map<std::string, SoLoud::Wav> soundFileUMap;
	Image						tileTypeArray[TILE_COUNT];
};

class Spell {
	protected:
		GameData&				mGameData;
		Vector					mPosition;
		Vector					mVelocity;

		int						mDamage = 0;
		int						mPiercingLayers = 0;

		double					mAngle = 0.0;
		double					mLifeTime = 0.0;
		double					mRadius = 0.0;
		double					mKnockbackDistance = 0.0;
		double					mAOEAttackDelay = 0.0;
		double					mCastSound = 0.0;
		double					mSpellAttackDelay = 0.0;

		std::string				mKey;
		std::vector<uint32_t>	mEnemyIds;

	public:
		Spell(GameData& gameData) : mGameData(gameData) {
		}

		// Destructor
		virtual ~Spell() = default;

		void setVelocity(Vector velocity) {
			mVelocity = velocity;
		}
		Vector getVelocity() {
			return mVelocity;
		}
		virtual void setSpellAttackDelay(double spellAttackDelay) {
			mSpellAttackDelay = spellAttackDelay;
		}
		virtual double getSpellAttackDelay() {
			return mSpellAttackDelay;
		}
		virtual void setPosition(Vector position) = 0;
		virtual void setTarget(int spellSpeed);
		virtual void setDamage(int damage) {
			mDamage = damage;
		}
		virtual void setPiercingLayers(int piercingLayers) {
			mPiercingLayers = piercingLayers;
		}
		virtual void setLifeTime(double lifeTime) {
			mLifeTime = lifeTime;
		}
		virtual void setKnockBackDistance(double knockbackDistance) {
			mKnockbackDistance = knockbackDistance;
		}
		virtual void setAOEAttackDelay(double aOEAttackDelay) {
			mAOEAttackDelay = aOEAttackDelay;
		}
		virtual double getAOEAttackDelay() {
			return mAOEAttackDelay;
		}

		virtual int getPiercingLayers() {
			return mPiercingLayers;
		}
		virtual double getLifeTime() {
			return mLifeTime;
		}
		virtual double getKnockbackDistance() {
			return mKnockbackDistance;
		}
		virtual Vector getPosition() {
			return mPosition;
		}

		virtual void activateKnockback(Enemy* enemyTargeted);

		virtual Image* getImage(GameData& gameData) = 0;
		virtual void draw(GameData& gameData);
		// TODO: Make pure virtual function
		virtual void loadCastSound();
		virtual void playCastSound();

		virtual double currentLifeTime(double deltaTime) {
			return mLifeTime -= deltaTime;
		}
		virtual void updatePosition(double deltaTime) {
			REF(deltaTime);
		}

		virtual bool canDamage(Enemy* enemyTargeted) {
			if (enemyTargeted->hp <= 0) {
				return false;
			}
			for (uint32_t id : mEnemyIds) {
				if (id == enemyTargeted->mId) {
					return false;
				}
			}
			return true;
		}
		virtual int applyDamage(Enemy* enemyTargeted) {
			if (getPiercingLayers() == 0) {
				mLifeTime = 0;
			}
			else {
				--mPiercingLayers;
			}

			enemyTargeted->hp -= mDamage;
			return mDamage;
		}

		virtual void collision(Enemy* enemyTargeted);


};

class SpikeSpell : public Spell {
	public:
		// Constructor
		SpikeSpell(GameData& gameData) : Spell(gameData) {}
		Image* getImage(GameData& gameData) override;
		void setPosition(Vector position) override;
		void updatePosition(double deltaTime) override {
			if (currentLifeTime(deltaTime) > 0) {
				mPosition = mPosition + (mVelocity * deltaTime);
			}
		}
		void loadCastSound() override;
		void playCastSound() override;
};

class ShadowOrbSpell : public Spell {
	public: 
		ShadowOrbSpell(GameData& gameData) : Spell(gameData) {}
		void setPosition(Vector position) override;
		Image* getImage(GameData& gameData) override;
		void updatePosition(double deltaTime) override {
			if (currentLifeTime(deltaTime) > 0) {
				mPosition = mPosition + (mVelocity * deltaTime);
			}
		}
};

class ConsecratedGroundSpell : public Spell {
	public:
		ConsecratedGroundSpell(GameData& gameData) : Spell(gameData) {}
		void setPosition(Vector position) override;
		Image* getImage(GameData& gameData) override;
		void updatePosition(double deltaTime) override {
			REF(deltaTime);
			mPosition = mGameData.player.position;
		}
		virtual bool canDamage(Enemy* enemyTargeted) override {
			if (enemyTargeted->hp > 0) {
				if (enemyTargeted->timeUntilDamageTaken <= 0) {
					return true;
				}
			}
			return false;
		}
		virtual int applyDamage(Enemy* enemyTargeted) override {
			enemyTargeted->timeUntilDamageTaken = getAOEAttackDelay();
			setPiercingLayers(-1);
			return Spell::applyDamage(enemyTargeted);
		}
};

class FireballSpell : public Spell {
	public:
		// Constructor
		FireballSpell(GameData& gameData) : Spell(gameData) {}
		Image* getImage(GameData& gameData) override;
		void setPosition(Vector position) override;
		void updatePosition(double deltaTime) override {
			if (currentLifeTime(deltaTime) > 0) {
				mPosition = mPosition + (mVelocity * deltaTime);
			}
		}
		virtual int applyDamage(Enemy* enemyTargeted) override;
};

class FireAOESpell : public Spell {
	public:
		// Constructor
		FireAOESpell(GameData& gameData) : Spell(gameData) {
			mSpellAttackDelay = 0.5;
		}
		Image* getImage(GameData& gameData) override;
		void setPosition(Vector position) override;
		void updatePosition(double deltaTime) override {
			currentLifeTime(deltaTime);
		}
		virtual bool canDamage(Enemy* enemyTargeted) override {
			if (enemyTargeted->hp > 0) {
				if (enemyTargeted->timeUntilDamageTaken <= 0) {
					return true;
				}
			}
			return false;
		}
		virtual int applyDamage(Enemy* enemyTargeted) override {
			enemyTargeted->timeUntilDamageTaken = getAOEAttackDelay();
			setPiercingLayers(-1);
			return Spell::applyDamage(enemyTargeted);
		}
};

class Staff {
	protected:
		GameData&			mGameData;
		Character*			mCaster;
		Vector				mPosition;
		Image				mImage;

		double				mTimeUntilCast = 0;
		double				mSpellAttackDelay = 0;

	public:
		// Initializtion list. Use comma to init more variables
		// By the time you are inside of a constructors scope,
		// all variables are initilized. This method does it before
		// the scope. A reference cannot be bound before the scope.
		Staff(GameData& gameData) : mGameData(gameData) {
			// Lambda for temporary debuging weapons
			std::erase_if(mGameData.spells, [](Spell* spell) {
				return spell;
				}
			);
		}
		// Destructor
		// Default is so you don't need to specify a body for compilation
		virtual ~Staff() = default;
		virtual Spell* cast(double deltaTime) = 0;
		virtual double getSpellAttackDelay() {
			return mSpellAttackDelay;
		}
		// For the children only
		virtual bool canCast(double deltaTime) {
			this->mTimeUntilCast -= deltaTime;
			if (this->mTimeUntilCast <= 0) {
				this->mTimeUntilCast = getSpellAttackDelay();
				return true;
			}
			else {
				return false;
			}
		}
};

class SpikeStaff : public Staff {
	public:
		SpikeStaff(GameData& gameData, Character* caster): Staff(gameData)  {
			mSpellAttackDelay = 0.5;
			this->mCaster = caster;
		}

		Spell* cast(double deltaTime) override {
			if (canCast(deltaTime)) {
				SpikeSpell* spikeSpell = new SpikeSpell(mGameData);
				spikeSpell->setPosition(mCaster->position);
				spikeSpell->setTarget(400);
				spikeSpell->setLifeTime(5);
				spikeSpell->setDamage(25);
				spikeSpell->setKnockBackDistance(500.0);
				spikeSpell->setPiercingLayers(0);
				spikeSpell->setSpellAttackDelay(getSpellAttackDelay());
				spikeSpell->playCastSound();
				return spikeSpell;
			}
			else {
				return nullptr;
			}
		}
};

class ShadowOrbStaff : public Staff {
	public: 
		ShadowOrbStaff(GameData& gameData, Character* caster) : Staff(gameData) {
			mSpellAttackDelay = 1.5;
			mCaster = caster;
		}

		Spell* cast(double deltaTime) override {
			if (canCast(deltaTime)) {
				ShadowOrbSpell* shadowOrb = new ShadowOrbSpell(mGameData);
				shadowOrb->setPosition(mCaster->position);
				shadowOrb->setTarget(200);
				shadowOrb->setLifeTime(5.0);
				shadowOrb->setDamage(50);
				shadowOrb->setKnockBackDistance(400.0);
				shadowOrb->setPiercingLayers(1);
				// shadowOrb->playCastSound();
				return shadowOrb;
			}
			else {
				return nullptr;
			}
		}

};

class ConsecratedGroundStaff : public Staff {
	public:
		ConsecratedGroundStaff(GameData& gameData, Character* caster) : Staff(gameData) {
			mSpellAttackDelay = UINT_MAX;
			mCaster = caster;
		}

		Spell* cast(double deltaTime) override {
			if (canCast(deltaTime)) {
				ConsecratedGroundSpell* consecratedGround = new ConsecratedGroundSpell(mGameData);
				consecratedGround->setPosition(mCaster->position);
				consecratedGround->setLifeTime(UINT_MAX);
				consecratedGround->setDamage(20);
				consecratedGround->setKnockBackDistance(250.0);
				consecratedGround->setPiercingLayers(0);
				consecratedGround->setAOEAttackDelay(1);				
				return consecratedGround;
			}
			else {
				return nullptr;
			}
		}
};

class FireballStaff : public Staff {
public:
	FireballStaff(GameData& gameData, Character* caster) : Staff(gameData) {
		mSpellAttackDelay = 2;
		this->mCaster = caster;
	}

	Spell* cast(double deltaTime) override {
		if (canCast(deltaTime)) {
			FireballSpell* fireballSpell = new FireballSpell(mGameData);
			fireballSpell->setPosition(mCaster->position);
			fireballSpell->setTarget(300);
			fireballSpell->setLifeTime(5);
			fireballSpell->setDamage(50);
			// Spell has no velocity (direction), so there is no direction to
			// apply the knockback.
			fireballSpell->setKnockBackDistance(750.0);
			fireballSpell->setPiercingLayers(0);
			fireballSpell->setSpellAttackDelay(getSpellAttackDelay());
			return fireballSpell;
		}
		else {
			return nullptr;
		}
	}
};

void myMemcpy(void* destination, void const* source, size_t size);

Sprite createSprite(Image image);

Color readPixel(Image image, int x, int y);

double distance(Vector a, Vector b);

double distancePlayer(Vector a, Vector b);

Image loadImage(SDL_Renderer* renderer, const char* fileName);

Image loadFont(SDL_Renderer* renderer, const char* fileName);

double returnSpriteSize(Image image);

void createCharacter(GameData& gameData, Image image, int healthPoints, bool animated, int speed, int frames);

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

void drawString(Color color, SDL_Renderer* renderer, Image* textImage, int size, std::string string, int x, int y);

void drawStringWorldSpace(Color color, GameData& gameData, SDL_Renderer* renderer, Image* textImage, int size, std::string string, int x, int y);

DamageNumber createDamageNumber(EntityType type, int damageNumber, Vector position, Vector velocity, int textSize, double lifeTime);

void drawDamageNumber(GameData& gameData, DamageNumber &damageNumber, Image* textImage, double deltaTime);

ExperienceOrb createExperienceOrb(Image image, double positionX, double positionY, double lifeTime);

void drawFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha);

void drawNonFilledRectangle(SDL_Renderer* renderer, SDL_Rect* rect, int red, int green, int blue, int alpha);

void drawHealthBar(GameData& gameData, SDL_Renderer* renderer);

void drawExperienceBar(GameData& gameData, SDL_Renderer* renderer);

void destroyEnemies(GameData& gameData);

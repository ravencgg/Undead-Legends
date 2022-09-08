#pragma once
#include <vector>
#include <string>
#include "Game.h"

class GameData;

class Spell {
	protected:
		GameData&				mGameData;
		Vector					mPosition;
		Vector					mVelocity;

		int						mDamage = 0;
		int						mNumberOfHits = 1;
		int						mPiercingLayers = 0;
		int						mAmmo = 1;

		double					mAngle = 0.0;
		double					mLifeTime = 0.0;
		double					mRadius = 0.0;
		double					mKnockbackDistance = 0.0;
		double					mAOEAttackDelay = 0.0;
		double					mCastSound = 0.0;
		double					mSpellAttackDelay = 0.0;
		double					mAmmoAttackSpeed = 0.0;
		
		int						mSpellSpeed = 0;
		int						mFrames = 0;

		std::string				mSpellKey;
		std::string				mEnemyHitKey;
		std::vector<uint32_t>	mEnemyIds;

	public:
		Spell(GameData& gameData) : mGameData(gameData) {}

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
		virtual void setNumberOfHits(int numberOfHits) {
			mNumberOfHits = numberOfHits;
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
		virtual void loadEnemyHitSound();
		virtual void playEnemyHitSound();

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
			mPosition = mGameData.player->position;
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
		bool firstHit = true;
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
			// So the aoe doesn't attack right away
			if (firstHit) {
				firstHit = false;
				enemyTargeted->timeUntilDamageTaken = getAOEAttackDelay();
				return -1;
			}
			else {
				enemyTargeted->timeUntilDamageTaken = getAOEAttackDelay();
				setPiercingLayers(-1);
				return Spell::applyDamage(enemyTargeted);
			}
		}
};

class MagicSwordSpell : public Spell {
public:
	// Constructor
	MagicSwordSpell(GameData& gameData) : Spell(gameData) {}
	Image* getImage(GameData& gameData) override;
	void setPosition(Vector position) override;
	/*
	void updatePosition(double deltaTime) override {
		if (currentLifeTime(deltaTime) > 0) {
			mPosition = mPosition + (mVelocity * deltaTime);
		}
	}
	virtual int applyDamage(Enemy* enemyTargeted) override;
	*/
};
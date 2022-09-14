#pragma once

#include "Game.h"
#include "Spell.h"
#include "Tools.h"
#include "Constants.h"

class GameData;
class Character;

class Staff {
	protected:
		GameData&					mGameData;
		Character*					mCaster;
		Vector						mPosition;
		Image						mImage;

		int							mCurrentCharges = 1;
		int							mMaxCharges = 1;
		double						mTimeUntilCast = 0;
		double						mSpellAttackDelay = 0;

		// uint32_t					mEntityID = -1;

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
		virtual ~Staff() = default;
		virtual Spell* cast(double deltaTime) = 0;
		virtual double getSpellAttackDelay() {
			return mSpellAttackDelay;
		}
		// For the children only
		virtual bool canCast(double deltaTime) {
			mTimeUntilCast -= deltaTime;
			if (mTimeUntilCast <= 0) {
				if (mCurrentCharges > 1) {
					mTimeUntilCast = 0.20;
					--mCurrentCharges;
				}
				else {
					setRestockCharges();
					mTimeUntilCast = getSpellAttackDelay();
				}
				return true;
			}
			else {
				return false;
			}
		}
		virtual void setRestockCharges() {
			mCurrentCharges = mMaxCharges;
		}
		virtual void setMaxCharges(int maxCharges) {
			mMaxCharges = maxCharges;
			mCurrentCharges = mMaxCharges;
		}
};

class SpikeStaff : public Staff {
	public:
		SpikeStaff(GameData& gameData, Character* caster) : Staff(gameData) {
			mSpellAttackDelay = 1;
			setMaxCharges(2);
			this->mCaster = caster;
		}

		Spell* cast(double deltaTime) override {
			if (canCast(deltaTime)) {
				SpikeSpell* spikeSpell = new SpikeSpell(mGameData);
				spikeSpell->setPosition(mCaster->position);
				spikeSpell->setTarget(400);
				spikeSpell->setLifeTime(5);
				spikeSpell->setDamage(50);
				spikeSpell->setKnockBackDistance(0.0);
				spikeSpell->setPiercingLayers(0);
				spikeSpell->setSpellAttackDelay(getSpellAttackDelay());
				spikeSpell->setNumberOfHits(2);
				// spikeSpell->playCastSound();
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
			mSpellAttackDelay = 3;
			setMaxCharges(2);
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
				shadowOrb->setNumberOfHits(1);
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
				consecratedGround->setDamage(100);
				consecratedGround->setKnockBackDistance(0.0);
				consecratedGround->setPiercingLayers(0);
				consecratedGround->setAOEAttackDelay(0.75);
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
			setMaxCharges(1);
			this->mCaster = caster;
		}

		Spell* cast(double deltaTime) override {
			if (canCast(deltaTime)) {
				FireballSpell* fireballSpell = new FireballSpell(mGameData);
				fireballSpell->setPosition(mCaster->position);
				fireballSpell->setTarget(200);
				fireballSpell->setLifeTime(10);
				fireballSpell->setDamage(100);
				// Spell has no velocity (direction), so there is no direction to
				// apply the knockback.
				fireballSpell->setKnockBackDistance(750.0);
				fireballSpell->setPiercingLayers(0);
				fireballSpell->setSpellAttackDelay(getSpellAttackDelay());
				fireballSpell->setNumberOfHits(1);
				return fireballSpell;
			}
			else {
				return nullptr;
			}
		}
};

class MagicSwordStaff : public Staff {
	public:
		MagicSwordStaff(GameData& gameData, Character* caster) : Staff(gameData) {
			mSpellAttackDelay = UINT_MAX;
			// Vector subscript out of range bug where there are too many. (Targeting enemies that don't 
			// exist) - Make bosses targetable by multiple enemies (remove the targeted boolean variable)
			// (If no other enemies exist)
			setMaxCharges(1);
			this->mCaster = caster;
		}

		Spell* cast(double deltaTime) override {
			if (canCast(deltaTime)) {
				MagicSwordSpell* magicSwordSpell = new MagicSwordSpell(mGameData);
				magicSwordSpell->setPosition(mCaster->position);
				magicSwordSpell->setTarget(250);
				magicSwordSpell->setLifeTime(UINT32_MAX);
				magicSwordSpell->setDamage(100);
				magicSwordSpell->setKnockBackDistance(0.0);
				// magicSwordSpell->setPiercingLayers(UINT_MAX);
				magicSwordSpell->setSpellAttackDelay(getSpellAttackDelay());
				magicSwordSpell->setTurnSpeed(720);
				magicSwordSpell->setDelayUntilAttack(0.50);
				return magicSwordSpell;
			}
			else {
				return nullptr;
			}
		}
};
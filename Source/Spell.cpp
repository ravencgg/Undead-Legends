#include "Spell.h"
#include "Constants.h"

class GameData;

void Spell::setTarget(int spellSpeed) {
	mSpellSpeed = spellSpeed;
	if (mGameData.enemies.size() > 0) {
		Vector offset = {};
		int nearestEnemy = closestEnemy(mGameData.player, &mGameData);
		offset = mGameData.enemies[nearestEnemy].position - mGameData.player->position;
		setVelocity(normalize(offset) * spellSpeed);
		mAngle = angleFromDirection(mVelocity);
	}
}

void Spell::activateKnockback(Enemy* enemyTargeted) {
	Vector knockbackVector = {};
	if (magnitude(mVelocity) == 0) {
		knockbackVector = enemyTargeted->position - mPosition;
		// No suitable knockback vector for this case
		if (magnitude(knockbackVector) == 0) {
			knockbackVector = { 100, 100 };
		}
	}
	else {
		knockbackVector = mVelocity;
	}

	knockbackVector = normalize(knockbackVector);

	knockbackVector = knockbackVector * getKnockbackDistance();

	enemyTargeted->velocity.x += knockbackVector.x;
	enemyTargeted->velocity.y += knockbackVector.y;
}

void Spell::collision(Enemy* enemyTargeted) {
	double distanceBetween = distance(mPosition, enemyTargeted->position);
	double radiusSum = mRadius + enemyTargeted->radius;
	if (distanceBetween < radiusSum) {
		if (enemyTargeted->hp > 0) {
			int damageDealt = applyDamage(enemyTargeted);
			if (damageDealt >= 0) {
				Vector numberVelocity = {};

				if (mNumberOfHits <= 0) {
					printf("ERROR: Spell::Collision function (numberOfHits is 0) DANGER!! NANS INCOMING");
				}
				else {
					// Truncates integers. Need to fix this at somepoint. Transition to floats?
					int updatedDamageNumber = mDamage / mNumberOfHits;
					double numberDelay = 0.0;
					double updatedLifeTime = Constants::DAMAGE_NUMBER_LIFETIME;
					// Or do the division in here and push back a damage number 5+ times based off hits
					// Remove number of hits from damage number and revert the changes. Damage number should 
					// ONLY draw 1 damage number. It shouldn't know how many to draw.
					for (int i = 0; i < mNumberOfHits; i++) {
						DamageNumber damageNumber = createDamageNumber(ENTITY_ENEMY, DN_FALLING, numberDelay, updatedDamageNumber, enemyTargeted->position,
							{ randomFloat(300, -300), randomFloat(-600, -300) }, Constants::DAMAGE_NUMBER_SIZE_E, Constants::DAMAGE_NUMBER_LIFETIME);
						mGameData.damageNumbers.push_back(damageNumber);
						updatedLifeTime += 0.25;
						numberDelay += 0.25;
					}

					mEnemyIds.push_back(enemyTargeted->mId);

					if (enemyTargeted->hp <= 0) {
						enemyTargeted->destroyed = true;
						// totalEnemiesKilled += 1;
						// Experience Orbs Vector
						static Image experienceOrbImage = loadImage(mGameData.renderer, "Assets/Experience_Orb_1.png");
						ExperienceOrb experienceOrb = createExperienceOrb(experienceOrbImage,
							enemyTargeted->position.x, enemyTargeted->position.y, Constants::EXPERIENCE_ORB_LIFETIME);
						mGameData.experienceOrbs.push_back(experienceOrb);
					}

					// playEnemyHitSound();

					// Knock back enemies
					activateKnockback(enemyTargeted);
				}
			}
			else {
				printf("ERROR: Spell::Collision function (Damage dealt < 0)");
			}
		}
	}
}

void Spell::draw(GameData& gameData) {
	Image* image = getImage(gameData);
	SDL_Rect srcRect = {};
	SDL_Rect destRect = {};

	if (mFrames > 0) {
		srcRect.w = image->w / mFrames;
		srcRect.h = image->h;
		Uint32 getTicks = SDL_GetTicks();
		srcRect.x = srcRect.w * (int)((getTicks / 100) % mFrames);

		destRect.w = srcRect.w;
		destRect.h = srcRect.h;

		destRect.x = (int)mPosition.x;
		destRect.y = (int)mPosition.y;

		destRect = convertCameraSpace(gameData.camera, destRect);

		SDL_RenderCopyEx(gameData.renderer, image->texture, &srcRect, &destRect, mAngle, NULL, SDL_FLIP_NONE);

	}
	else {
		destRect.w = image->w;
		destRect.h = image->h;

		destRect.x = (int)mPosition.x;
		destRect.y = (int)mPosition.y;

		destRect = convertCameraSpace(gameData.camera, destRect);

		SDL_RenderCopyEx(gameData.renderer, image->texture, NULL, &destRect, mAngle, NULL, SDL_FLIP_NONE);
	}
}

void Spell::loadCastSound() {
}

void Spell::playCastSound() {

}

// Currently being done inside drawDamageNumber function
void Spell::loadEnemyHitSound() {
	mEnemyHitKey = "enemyHitSound";
	// Did not find the key
	if (mGameData.soundFileUMap.find(mEnemyHitKey) == mGameData.soundFileUMap.end()) {
		SoLoud::Wav& wav = mGameData.soundFileUMap[mEnemyHitKey];
		// Assets/Audio/mixkit-hard-typewriter-hit-1364.wav
		// Assets/Audio/mixkit-typewriter-hit-1362.wav
		wav.load("Assets/Audio/mixkit-hard-typewriter-hit-1364.wav");
	}
}

void Spell::playEnemyHitSound() {
	loadEnemyHitSound();
	auto result = mGameData.soundFileUMap.find(mEnemyHitKey);
	if (result != mGameData.soundFileUMap.end()) {
		int handle = mGameData.soloud.play(result->second, 0.25f, 0, 1);
		float playSpeed = 0.75f;
		mGameData.soloud.setRelativePlaySpeed(handle, playSpeed);
		mGameData.soloud.setPause(handle, 0);
	}
}

// Spike Spell
void SpikeSpell::setPosition(Vector position) {
	mPosition = position;
}

Image* SpikeSpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_Spike_2.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}

void SpikeSpell::loadCastSound() {
	mSpellKey = "spikeCastSound";
	// Did not find the key
	if (mGameData.soundFileUMap.find(mSpellKey) == mGameData.soundFileUMap.end()) {
		SoLoud::Wav& wav = mGameData.soundFileUMap[mSpellKey];
		wav.load("Assets/Audio/mixkit-arrow-whoosh-1491.wav");
	}
}

void SpikeSpell::playCastSound() {
	loadCastSound();
	auto result = mGameData.soundFileUMap.find(mSpellKey);
	if (result != mGameData.soundFileUMap.end()) {
		int handle = mGameData.soloud.play(result->second, 0.5f, 0, 1);
		float playSpeed = 1.0f / (float)getSpellAttackDelay();
		mGameData.soloud.setRelativePlaySpeed(handle, playSpeed);
		mGameData.soloud.setPause(handle, 0);
	}
}

// Shadow Orb Spell
void ShadowOrbSpell::setPosition(Vector position) {
	mPosition = position;
}

Image* ShadowOrbSpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_ShadowOrb_1.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}

// Consecrated Ground Spell
void ConsecratedGroundSpell::setPosition(Vector position) {
	mPosition = position;
}

Image* ConsecratedGroundSpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_Consecrated_Ground_1.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}

// Fireball spell
void FireballSpell::setPosition(Vector position) {
	mPosition = position;
}

Image* FireballSpell::getImage(GameData& gameData) {
	// "Assets/Weapon_Fireball_1.png"
	// "Assets/Weapon_Fireball_Test.png"
	mFrames = 7;
	static Image spellImage = loadImage(gameData.renderer, "Assets/Fireball_Animated/Fireball_Animated_3_Final-Sheet.png");
	return &spellImage;
}

int FireballSpell::applyDamage(Enemy* enemyTargeted) {
	if (getPiercingLayers() == 0) {
		Spell* fireAOESPELL = new FireAOESpell(mGameData);
		fireAOESPELL->setPosition(mPosition);
		fireAOESPELL->setLifeTime(5);
		fireAOESPELL->setDamage(10);
		fireAOESPELL->setKnockBackDistance(100.0);
		fireAOESPELL->setAOEAttackDelay(5);
		mGameData.spells.push_back(fireAOESPELL);

		mLifeTime = 0;
	}
	else {
		--mPiercingLayers;
	}

	enemyTargeted->hp -= mDamage;
	return mDamage;
}

// FireAOE spell
void FireAOESpell::setPosition(Vector position) {
	mPosition = position;
}

Image* FireAOESpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_Fireball_AOE_6.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}

// Magic Sword Spell
void MagicSwordSpell::setPosition(Vector position) {
	mPosition = position;
}

Image* MagicSwordSpell::getImage(GameData& gameData) {
	static Image spellImage = loadImage(gameData.renderer, "Assets/Weapon_Spike_1.png");
	mRadius = returnSpriteSize(spellImage);
	return &spellImage;
}
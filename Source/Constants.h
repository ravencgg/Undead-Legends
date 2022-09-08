#pragma once

#include <float.h>

namespace Constants {
	static const int		RESOLUTION_X = 1600;
	static const int		RESOLUTION_Y = 900;
	static const double		ENEMY_SPEED = 50;
	static const double		ENEMY_ACCELERATION = 2500;
	static const int		TILE_SIZE = 32;
	static const double		GRAVITY = 2000;
	static const int		HEALTH_BAR_W = 55;
	static const int		HEALTH_BAR_H = 8;
	static const int		EXP_BAR_W = RESOLUTION_X - 20;
	static const int		EXP_BAR_H = 25;

	static const int		DAMAGE_NUMBER_SIZE_E = 1;
	static const int		DAMAGE_NUMBER_SIZE_P = 1;
	static const double		DAMAGE_NUMBER_LIFETIME = 0.75;
	static const double		EXPERIENCE_ORB_LIFETIME = DBL_MAX;
	static const double		EXPERIENCE_RADIUS = 6;
	static const double		EXPERIENCE_ORB_SPEED = 250;
	static const int		EXPERIENCE_ORB_EXPERIENCE = 50;
	static const double		DEFAULT_PICKUP_RADIUS = 200;
};
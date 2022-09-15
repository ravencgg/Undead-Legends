#pragma once
#include "Render.h"

struct Vector {
	double				x = 0;
	double				y = 0;
};

struct Image {
	unsigned char*		pixelData = nullptr;
	R_Texture*			texture = nullptr;
	int					w = 0;
	int					h = 0;
	int					num_frames = 0;
};

struct Color {
	unsigned char		r = {}, g = {}, b = {}, a = {};
};

struct Sprite {
	Image				image;
	int					width = 0;
	int					height = 0;
};

Sprite createSprite(Image image);

Color readPixel(Image image, int x, int y);

double distance(Vector a, Vector b);

double returnSpriteSize(Image image);
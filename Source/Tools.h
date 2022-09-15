#pragma once
#include "Sprite.h"

R_Rect MakeRect(Vector center, Vector size);
R_Rect MakeRect(Vector center, Sprite* sprite);
R_Rect MakeRect(Vector center, Image* image);

double getTime();

Vector facingDirection(double theta);

double angleFromDirection(Vector a);

double magnitude(Vector a);

Vector normalize(Vector a);

Vector operator+(Vector a, Vector b);

Vector operator-(Vector a, Vector b);

Vector& operator*=(Vector& a, double b);

Vector operator*(Vector a, double b);
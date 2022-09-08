#pragma once
#include "Sprite.h"

double getTime();

Vector facingDirection(double theta);

double angleFromDirection(Vector a);

double magnitude(Vector a);

Vector normalize(Vector a);

Vector operator+(Vector a, Vector b);

Vector operator-(Vector a, Vector b);

Vector& operator*=(Vector& a, double b);

Vector operator*(Vector a, double b);
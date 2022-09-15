#include "Tools.h"

R_Rect MakeRect(Vector center, Vector size)
{
	R_Rect result;
	double hw = size.x / 2.0;
	double hh = size.y / 2.0;
	result.x = (center.x - hw);
	result.y = (center.y - hh);
	result.w = size.x;
	result.h = size.y;
	return result;
}

R_Rect MakeRect(Vector center, Sprite* sprite)
{
	Vector size = { (double)sprite->width / sprite->image.num_frames, (double)sprite->height };
	return MakeRect(center, size);
}

R_Rect MakeRect(Vector center, Image* image)
{
	Vector size = { (double)image->w / image->num_frames, (double)image->h };
	return MakeRect(center, size);
}

double getTime() {
	return SDL_GetTicks() / 1000.0;
}

Vector facingDirection(double theta) {
	Vector result = {};
	// M_PI / 180 is converting from degrees to radians
	// 90 = Pi / 2
	// The cos and sin of any angle is 1
	result.x = cos(theta * M_PI / 180);
	result.y = sin(theta * M_PI / 180);
	// Returns JUST a direction of that angle
	return result;
}

double angleFromDirection(Vector a) {
	double result = (180 / M_PI) * atan2(a.y, a.x);
	return result;
}

double magnitude(Vector a) {
	return sqrt(a.x * a.x + a.y * a.y);
}

Vector normalize(Vector a) {
	double m = magnitude(a);
	Vector normalized = { a.x / m, a.y / m };
	return normalized;
}

Vector operator+(Vector a, Vector b) {
	Vector result = {};
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

Vector operator-(Vector a, Vector b) {
	Vector result = {};
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

Vector& operator*=(Vector& a, double b) {
	a.x *= b;
	a.y *= b;
	return a;
}

Vector operator*(Vector a, double b) {
	Vector result = {};
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

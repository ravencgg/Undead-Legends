#include "Sprite.h"

Sprite createSprite(Image image) {
	Sprite result = {};
	R_QueryTexture(image.texture, NULL, NULL, &result.width, &result.height);
	result.image = image;
	return result;
}

Color readPixel(Image image, int x, int y) {
	// This is a cast from 1 unsigned character to 4 unsigned characters
	Color* pixels = (Color*)image.pixelData;
	// The square brackets are derefencing the pointer
	Color c = pixels[y * image.w + x];

	return c;
}

double distance(Vector a, Vector b) {
	Vector to_a = {};
	// Calculate the offset
	to_a.x = a.x - b.x;
	to_a.y = a.y - b.y;
	// Calculate the length
	double result = sqrt(to_a.x * to_a.x + to_a.y * to_a.y);
	return result;
}

double returnSpriteSize(Image image) {
	Vector center = {};
	center.x = (double)image.w / 2;
	center.y = (double)image.h / 2;

	Vector pixel = {};

	double maxDistance = 0;

	for (int i = 0; i < image.w; i++) {
		pixel.x = i;
		for (int j = 0; j < image.h; j++) {
			pixel.y = j;
			Color alphaTest = (readPixel(image, i, j));

			if (alphaTest.a > 0) {
				double distance1 = distance(center, pixel);
				if (distance1 > maxDistance) {
					maxDistance = distance1;
				}
			}
		}
	}

	return maxDistance;
}
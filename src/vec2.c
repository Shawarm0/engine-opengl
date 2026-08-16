#include <stdio.h>
#include "vec2.h"
#include <math.h>


// Takes two input vectors and returns their x and y positions added together.
Vec2 v2_add(Vec2 a, Vec2 b) {
	return (Vec2){ a.x + b.x, a.y + b.y };
}

// Takes an input vector and returns each x and y coordinate multiplied by a scale.
Vec2 v2_scale(Vec2 a, double s) {
	return (Vec2){ a.x * s, a.y * s };
}

// Takes two input vectors and returns their x and y positions subtracted.
Vec2 v2_sub(Vec2 a, Vec2 b) {
	return (Vec2){ a.x - b.x, a.y - b.y};
}

// Returns the magnitude of a Vector
double v2_len(Vec2 a) {
	return sqrt(a.x*a.x + a.y*a.y);
}

// Returns the squared magnitude of a Vector
double v2_len2(Vec2 a) {
	return a.x*a.x+a.y*a.y;
}

// Normalises a vector returns the unit vector in the same direction.
Vec2 v2_norm(Vec2 a) {
	double l = v2_len(a);
	return l > 0.0 ? v2_scale(v, 1.0/l) : (Vec2){0.0, 0.0};
}

// Returns the dot product of two vectors.
double v2_dot(Vec2 a, Vec2 b) {
	return a.x * b.x + a.y * b.y;	
}


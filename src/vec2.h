
#ifndef VEC2_H
#define VEC2_H


// Define a vector.
typedef struct {
	double x, y;
} Vec2;

Vec2 v2_add(Vec2 a, Vec2 b);
Vec2 v2_scale(Vec2 v, double s);
Vec2 v2_sub(Vec2 a, Vec2 b);
double v2_len(Vec2 a);
double v2_len2(Vec2 a);
Vec2 v2_norm(Vec2 a);
double v2_dot(Vec2 a, Vec2 b);


#endif

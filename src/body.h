#ifndef BODY_H
#define BODY_H

#include <stddef.h>   /* size_t */
#include "vec2.h"     /* Vec2 */

#define BODYARRAY_INIT_CAP 30
#define G          10000.0
#define SOFTENING  10.0



typedef struct {
	double mass;
	Vec2 pos, vel, force;
} Body;

typedef struct {
	Body *data;
	size_t count;
	size_t capacity;
} BodyArray;

Body bodyCreate(double mass, Vec2 pos, Vec2 vel);
void compute_forces(Body *b, size_t n);
void integrate(Body *b, size_t n, double dt);
int  bodyArray_init(BodyArray *array);
int  bodyArray_push(BodyArray *array, Body b);
int  bodyArray_delete(BodyArray *array, size_t index);
void bodyArray_clear(BodyArray *array);
void bodyArray_free(BodyArray *array);

#endif /* BODY_H */

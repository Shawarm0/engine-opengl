#include "vec2.h"
#include "body.h"
#include <math.h>
#include <stdlib.h>


/* Purpose
 * - Set the array into a valid empty state
 * - No planets yet
 * - Safe to call Add afterwards
 */
int bodyArray_init(BodyArray *array) {
	// If no pointer passed	
	if (array == NULL) return 0;

	// Placeholder data
	array->data = NULL;
	array->count = 0;
	array->capacity =0;

	Body *data = malloc(BODYARRAY_INIT_CAP * sizeof(Body));

	// If failed assigning memory to the data pointer
	if (data == NULL) return 0;

	// Assign values
	array->data = data;
	array->capacity = BODYARRAY_INIT_CAP;
	array->count = 0;

	return 1;
}

/* Purpose
 * - Add one Body
 * - Grow storage if needed
 */
int bodyArray_push(BodyArray *array, Body b) {
	if (array == NULL) return 0;

	if (array->count == array->capacity) {
		size_t newcap = array->capacity ? array->capacity * 2 : 8;
		Body *tmp = realloc(array->data, newcap * sizeof(Body)); // If full we double the size of array.
																  
		if (tmp == NULL) return 0;

		array -> data = tmp;
		array -> capacity = newcap;
	}
	array->data[array->count] = b;
	array->count++;
	return 1;
}

/* Purpose 
 * - Delete an index
 * - Shift all planets 1 to the left 
 */
int bodyArray_delete(BodyArray *array, size_t index) {
    if (array == NULL || index >= array->count) return 0;
    for (size_t i = index; i + 1 < array->count; i++) {
        array->data[i] = array->data[i + 1];
    }
    array->count--;
    return 1;
}

/* Purpose
 * - Removes all planets but keep memory
 */
void bodyArray_clear(BodyArray *array) {
	if (array == NULL) return;
	array->count = 0;
}



/* Purpose
 * - Create a body
 */
Body bodyCreate(double mass, Vec2 pos, Vec2 vel) {
	return (Body){
		.mass = mass,
		.pos = pos,
		.vel = vel,
		.force = (Vec2){0, 0},
	};
}

void integrate(Body *b, size_t n, double dt) {
	for (size_t i = 0; i < n; i++) {
		Vec2 acc = v2_scale(b[i].force, 1.0 / b[i].mass);
		b[i].vel = v2_add(b[i].vel, v2_scale(acc, dt));
		b[i].pos = v2_add(b[i].pos, v2_scale(b[i].vel, dt));
	}
}

void compute_forces(Body *b, size_t n) {
	for (size_t i = 0; i < n; i++) {
		b[i].force = (Vec2){0, 0};
	}

	for (size_t i = 0; i < n; i++) {
		for (size_t j = i + 1; j < n; j++) {
			Vec2 d = v2_sub(b[j].pos, b[i].pos);

			double r2 = v2_len2(d) + SOFTENING * SOFTENING;
			double r3 = r2 * sqrt(r2);

			Vec2 f = v2_scale(d, G * b[i].mass * b[j].mass / r3);

			b[i].force = v2_add(b[i].force, f);
			b[j].force = v2_sub(b[j].force, f);
		}
	}
}

/* Purpose
 * - Release all memory 
 * - Leave the array unusable unless re-initialized
 */
void bodyArray_free(BodyArray *array) {
	if (array == NULL) return;

	free(array->data);
	array->data = NULL;
	array->count = 0;
	array->capacity =0;
}

#include "vec2.h"
#include <stdlib.h>


#define BODYARRAY_INIT_CAP 30

// Define a body;
typedef struct {
	double mass;
	Vec2 pos, vel, force;
} Body;

typedef struct {
	Body *data;
	size_t count;
	size_t capacity;
} BodyArray;


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
int bodyArray_delete(BodyArray *array, int index) {
	if (array == NULL || index < 0 || index >= array->count) return 0;

	for (int i = index; i < array->count - 1; i++) {
		array->data[i] = array->data[i+1];
	}

	array->count--;
	return 1;
}

/* Purpose
 * - Removes all planets but keep memory
 */
void bodyArray_Clear(BodyArray *array) {
	if (array == NULL) return;

	for (int i = 0; i < array->count; i++) {
		array->data[i] = (Body){
			.mass = 0.0f,
			.pos = (Vec2){0, 0},
			.vel = (Vec2){0, 0},
			.force = (Vec2){0, 0},
		};
	}
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


/* Purpose
 * - Release all memory 
 * - Leave the array unusable unless re-initialized
 */
void bodyArray_Free(BodyArray *array) {
	if (array == NULL) return;

	free(array->data);
	array->data = NULL;
	array->count = 0;
	array->capacity =0;
}

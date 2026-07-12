#include <exercise.h>

void addExercisePoint(exercise_t *exercise, exercise_point_t *point) {
    if (!exercise->list) {
        exercise->list = point;
        exercise->last = point;

        return;
    }

    exercise->last->next = point;
    exercise->last = point;
}
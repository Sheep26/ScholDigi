#include <exercise.h>

void addExercisePoint(exercise_t *exercise, exercise_point_t *point) {
    if (!exercise->list) {
        exercise->list = point;
        exercise->last = point;

        return;
    }

    if (exercise->last)
        exercise->last->next = point;
    else {
        // Fallback incase exercise->last is not set yet.
        exercise_point_t *p = exercise->list;

        while (p->next)
            p = p->next;

        p->next = point;
    }

    exercise->last = point;
}
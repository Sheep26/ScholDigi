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
        // It's ugly I know, however it should never run so it doesn't matter.
        // I'm wasting cpu cycles checking with this but I need a fallback just incase.
        exercise_point_t *p = exercise->list;

        while (p->next)
            p = p->next;

        p->next = point;
    }

    exercise->last = point;
}
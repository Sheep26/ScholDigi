#include <exercise.h>
#include <haversine.h>
#include <stdlib.h>

ssd1306_handle_t display_handle;
exercise_t *current_exercise;

void setup_display_handle(ssd1306_handle_t handle) {
    display_handle = handle;
}

void addExercisePoint(exercise_t *exercise, exercise_point_t *point) {
    if (!exercise->list)
        exercise->list = point;

    if (exercise->last)
        exercise->last->next = point;

    exercise->last = point;
}

exercise_t *getExercise() {
    return current_exercise;
}

void toggleExercise(time_t *time_epoch) {
    if (!current_exercise)
        current_exercise = (exercise_t*) calloc(1, sizeof(exercise_t));
    else {
        // Handle putting exercise into storage eventually.
        exercise_point_t *p = current_exercise->list;
        exercise_point_t *l = (void*) 0;

        int div = 0;

        while (p) {
            // Add exercise distance.
            if (l) {
                current_exercise->distance += distanceBetweenPoints(l, p);

                l = (void*) 0;
            } else
                l = p;

            current_exercise->max_alt = p->alt > current_exercise->max_alt ? p->alt : current_exercise->max_alt;
            current_exercise->min_alt = (p->alt < current_exercise->min_alt || !current_exercise->min_alt) ? p->alt : current_exercise->min_alt;
            current_exercise->top_speed = p->speed > current_exercise->top_speed ? p->speed : current_exercise->top_speed;

            current_exercise->alt_diff = current_exercise->max_alt - current_exercise->min_alt;
            current_exercise->avg_alt += p->alt;
            current_exercise->avg_speed += p->speed;

            div++;                    
            p = p->next;
        }

        current_exercise->avg_alt = current_exercise->avg_alt / div;
        current_exercise->avg_speed = current_exercise->avg_speed / div;

        current_exercise->stop = *time_epoch;
        // We have calculated all our things so we need to store it now.

        

        // Free exercise from memory.
        exercise_point_t *pointah = current_exercise->list;

        while (pointah) {
            exercise_point_t *next = p->next;

            free(pointah);
            pointah = next;
        }

        free(current_exercise);
        current_exercise = (void*) 0; // Set current exercise to null instead of having it reference unallocated memory.
    }
}
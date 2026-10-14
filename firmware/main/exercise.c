#include <exercise.h>
#include <haversine.h>
#include <stdlib.h>
#include <bluetooth.h>
#include <time.h>
#include <sys/time.h>
#include <gettime.h>

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

void toggleExercise() {
    struct tm current_time = getTime();
    time_t time_epoch = mktime(&current_time);

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

        current_exercise->stop = time_epoch;

        exercise_send_t *send_struct = (exercise_send_t *) calloc(1, sizeof(exercise_send_t));
        send_struct->distance = current_exercise->distance;
        send_struct->max_alt = current_exercise->max_alt;
        send_struct->min_alt = current_exercise->min_alt;
        send_struct->top_speed = current_exercise->top_speed;
        send_struct->alt_diff = current_exercise->alt_diff;
        send_struct->avg_alt = current_exercise->avg_alt;
        send_struct->avg_speed = current_exercise->avg_speed;

        ble_send_data(send_struct, sizeof(exercise_send_t));

        // Free exercise from memory.
        exercise_point_t *pointah = current_exercise->list;

        while (pointah) {
            exercise_point_t *next = p->next;
            exercise_point_send_t *send_point_struct = (exercise_point_send_t *) calloc(1, sizeof(exercise_point_send_t));
            send_point_struct->alt = next->alt;
            send_point_struct->lat = next->lat;
            send_point_struct->lng = next->lng;
            send_point_struct->speed = next->speed;
            send_point_struct->time = next->time;

            ble_send_data(send_point_struct, sizeof(exercise_point_send_t));

            free(pointah);
            pointah = next;
        }

        free(current_exercise);
        current_exercise = (void*) 0; // Set current exercise to null instead of having it reference unallocated memory.
    }
}
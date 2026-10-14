#include <exercise.h>
#include <haversine.h>
#include <stdlib.h>
#include <bluetooth.h>
#include <time.h>
#include <sys/time.h>
#include <gettime.h>
#include <string.h>
#include <global.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

ssd1306_handle_t display_handle;
exercise_t *current_exercise;

void setup_display_handle(ssd1306_handle_t handle) {
    display_handle = handle;
}

void addExercisePoint(exercise_t *exercise, exercise_point_t *point) {
    if (!exercise || !point)
        return;

    point->next = NULL;

    if (exercise->last)
        exercise->last->next = point;
    else
        exercise->list = point;

    exercise->last = point;
}

exercise_t *getExercise() {
    return current_exercise;
}

void toggleExercise() {
    time_t time_epoch = getTimeEpoch();

    if (!current_exercise) {
        current_exercise = (exercise_t*) calloc(1, sizeof(exercise_t));
        current_exercise->start = time_epoch;
    } else {
        if (current_exercise->list) {
            exercise_point_t *p = current_exercise->list;

            while (p && p->next) {
                double segment_distance = distanceBetweenPoints(p, p->next);
                current_exercise->distance += segment_distance;
                p = p->next;
            }

            p = current_exercise->list;
            int div = 0;

            while (p) {
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

            exercise_send_t *send_struct = (exercise_send_t *) malloc(sizeof(exercise_send_t));
            memcpy(send_struct, current_exercise, sizeof(exercise_send_t));

            ble_send_data(send_struct, sizeof(exercise_send_t));

            // Send exercise points.
            exercise_point_t *pointah = current_exercise->list;

            while (pointah) {
                exercise_point_t *next = pointah->next;
                exercise_point_send_t *send_point_struct = (exercise_point_send_t *) malloc(sizeof(exercise_point_send_t));
                memcpy(send_point_struct, pointah, sizeof(exercise_point_send_t));

                ble_send_data(send_point_struct, sizeof(exercise_point_send_t));

                free(pointah);
                //free(send_point_struct);
                pointah = next;
            }

            free(send_struct);

            display_next_update = 3000;
            ssd1306_clear_display(display_handle, false);

            ssd1306_display_text(display_handle, 0, "Exercise", false);
            char buf[64];

            snprintf(buf, sizeof(buf), "Distance: %dm", (int) current_exercise->distance);
            ssd1306_display_text(display_handle, 1, buf, false);

            snprintf(buf, sizeof(buf), "Avg Speed: %dm", (int) current_exercise->avg_speed);
            ssd1306_display_text(display_handle, 2, buf, false);

            snprintf(buf, sizeof(buf), "Min Alt: %dm", (int) current_exercise->min_alt);
            ssd1306_display_text(display_handle, 3, buf, false);

            snprintf(buf, sizeof(buf), "Max Alt: %dm", (int) current_exercise->max_alt);
            ssd1306_display_text(display_handle, 4, buf, false);
        }

        free(current_exercise);
        current_exercise = (void*) 0; // Set current exercise to null instead of having it reference unallocated memory.

        char *done = "DONE";
        ble_send_data(done, sizeof(done));
    }
}
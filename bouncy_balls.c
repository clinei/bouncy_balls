#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define BALL_COUNT 20
#define MIN_RADIUS 10
#define MAX_RADIUS 60
#define DELTA_RADIUS MAX_RADIUS - MIN_RADIUS
#define MIN_SPEED 50
#define MAX_SPEED 100
#define DELTA_SPEED MAX_SPEED - MIN_SPEED

EMSCRIPTEN_KEEPALIVE
float randf() {
    return rand() / (float)RAND_MAX;
}

typedef unsigned int table_id_t;
typedef unsigned int uint;
typedef unsigned char bool;
enum { false, true };

struct timespec start_timestamp;
struct timespec curr_time;
struct timespec prev_time;

// stop must be bigger than start
void timespec_diff(const struct timespec* stop,
                   const struct timespec* start,
                         struct timespec* result) {

    if (stop->tv_nsec < start->tv_nsec) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    }
    else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}
float timespec_to_float(const struct timespec* spec) {
    // maybe return ms instead of seconds
    return (float)spec->tv_sec + (float)(spec->tv_nsec / 1000000) / 1000;
}
float timespec_diff_float(const struct timespec* stop, const struct timespec* start) {
    struct timespec delta;
    timespec_diff(stop, start, &delta);
    return timespec_to_float(&delta);
}

void step();
bool paused = false;

EMSCRIPTEN_KEEPALIVE
void start_time() {
    clock_gettime(CLOCK_REALTIME, &start_timestamp);
    prev_time.tv_sec  = 0;
    prev_time.tv_nsec = 0;
    emscripten_set_main_loop(&step, 0, false);
}
EMSCRIPTEN_KEEPALIVE
void stop_time() {
    emscripten_cancel_main_loop();
}
float step_time() {
    clock_gettime(CLOCK_REALTIME, &curr_time);
    timespec_diff(&curr_time, &start_timestamp, &curr_time);
    const float delta = timespec_diff_float(&curr_time, &prev_time);
    prev_time = curr_time;
    return delta;
}

int screen_width, screen_height;
EMSCRIPTEN_KEEPALIVE
void set_screen_size(const int width, const int height) {
    screen_width = width;
    screen_height = height;
}

struct Physics_Balls {
    size_t count;
    float* x;
    float* y;
    float* x_speed;
    float* y_speed;
    float* radius;
};

struct Physics_Balls* physics_balls;
void alloc_physics_balls(size_t count) {
    physics_balls = malloc(sizeof(struct Physics_Balls));
    physics_balls->count = count;
    physics_balls->x = malloc(count * sizeof(float));
    physics_balls->y = malloc(count * sizeof(float));
    physics_balls->x_speed = malloc(count * sizeof(float));
    physics_balls->y_speed = malloc(count * sizeof(float));
    physics_balls->radius = malloc(count * sizeof(float));
}

EMSCRIPTEN_KEEPALIVE
void* get_physics_balls() {
    return physics_balls;
}

void get_distance_to_point(float x, float y,
                           float x2, float y2,
                           float* dx, float* dy,
                           float * distance) {

    *dx = x2 - x;
    *dy = y2 - y;
    *distance = sqrt((*dx)*(*dx) + (*dy)*(*dy));
}
void get_direction_to_point(float  x,  float  y,
                            float  x2, float  y2,
                            float* dx, float* dy,
                            float* distance,
                            float* dir_x, float* dir_y) {

    get_distance_to_point(x, y, x2, y2, dx, dy, distance);
    *dir_x = (*dx) / (*distance);
    *dir_y = (*dy) / (*distance);
}

EMSCRIPTEN_KEEPALIVE
void init(const int width, const int height) {

    alloc_physics_balls(BALL_COUNT);

    set_screen_size(width, height);

    for (table_id_t i = 0; i < physics_balls->count; i += 1) {
        const float radius = randf() * DELTA_RADIUS + MIN_RADIUS;
        physics_balls->x[i] = randf() * (screen_width - radius * 2) + radius;
        physics_balls->y[i] = randf() * (screen_height - radius * 2) + radius;
        const float speed = randf() * DELTA_SPEED + MIN_SPEED;
        const float dir = randf() * M_PI * 2;
        physics_balls->x_speed[i] = cos(dir) * DELTA_SPEED + MIN_SPEED;
        physics_balls->y_speed[i] = sin(dir) * DELTA_SPEED + MIN_SPEED;
        physics_balls->radius[i] = radius;
    }

    start_time();
}

void step_physics_balls(float delta) {
    // collide with other balls
    for (table_id_t i = 0; i < physics_balls->count; i += 1) {
        const float x = physics_balls->x[i];
        const float y = physics_balls->y[i];
        const float radius = physics_balls->radius[i];
        for (table_id_t j = 0; j < physics_balls->count; j += 1) {
            if (j != i) {
                const float j_x = physics_balls->x[j];
                const float j_y = physics_balls->y[j];
                const float j_radius = physics_balls->radius[j];

                float dx, dy, distance;
                get_distance_to_point(x, y, j_x, j_y, &dx, &dy, &distance);

                if (distance < radius + j_radius) {
                    const float mid_x = dx / 2;
                    const float mid_y = dy / 2;
                    const float dir_x = dx / distance;
                    const float dir_y = dy / distance;
                    const float power = (radius + j_radius - distance);
                    float push_x = (mid_x + dir_x * radius) * power;
                    float push_y = (mid_y + dir_y * radius) * power;
                    physics_balls->x_speed[i] -= push_x / radius;
                    physics_balls->y_speed[i] -= push_y / radius;
                    physics_balls->x_speed[j] += push_x / j_radius;
                    physics_balls->y_speed[j] += push_y / j_radius;
                }
            }
        }
    }
    // apply speed
    for (table_id_t i = 0; i < physics_balls->count; i += 1) {
        physics_balls->x[i] += physics_balls->x_speed[i] * delta;
        physics_balls->y[i] += physics_balls->y_speed[i] * delta;
    }
    // collide with walls
    for (table_id_t i = 0; i < physics_balls->count; i += 1) {
        const float x = physics_balls->x[i];
        const float y = physics_balls->y[i];
        const float x_speed = physics_balls->x_speed[i];
        const float y_speed = physics_balls->y_speed[i];
        const float radius = physics_balls->radius[i];
        if ((x - radius < 0            && x_speed < 0 ) ||
            (x + radius > screen_width && x_speed > 0)) {
            physics_balls->x_speed[i] *= -1;
        }
        if ((y - radius < 0             && y_speed < 0) ||
            (y + radius > screen_height && y_speed > 0)) {
            physics_balls->y_speed[i] *= -1;
        }
    }
}

EMSCRIPTEN_KEEPALIVE
void step() {
    float delta = step_time();
    step_physics_balls(delta);
}

int main(int argc, char** argv) {
}
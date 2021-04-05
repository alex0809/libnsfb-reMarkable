#include <libevdev/libevdev.h>
#include <stdbool.h>
#include <pthread.h>

#include "libnsfb.h"
#include "ringbuf.h"

#define EVENTS_DIR "/dev/input"

#define RM1_MACHINE_NAME_1  "reMarkable 1.0"
#define RM1_MACHINE_NAME_2  "reMarkable Prototype 1"
#define RM2_MACHINE_NAME    "reMarkable 2.0"

typedef struct input_single_state_s {
    int tracking_id;
    bool tracking_id_changed;

    // these positions are already translated to screen coordinates
    int position_x;
    int position_y;
    bool position_changed;

    int orientation;
    int distance;
    int pressure;

    int touch_major;
    int touch_minor;
    int width_major;
    int width_minor;
} input_single_state_t;

typedef struct input_pen_state_s {
    int min_x;
    int max_x;
    int min_y;
    int max_y;

    int distance;
    int pressure;

    // these positions are already translated to screen coordinates
    int position_x;
    int position_y;
    bool position_changed;

    bool touched;
    bool touch_state_changed;
} input_pen_state_t;

typedef struct input_multitouch_state_s {
    int min_x;
    int max_x;
    int min_y;
    int max_y;

    int current_slot;

    // a crude way to handle multitouch: disregard all slots after first
    int first_pressed_slot;

    input_single_state_t *slots;
} input_multitouch_state_t;

typedef struct input_state_s {
    enum { RM1, RM2 } model;

    struct libevdev *pen_dev;
    struct libevdev *gpio_dev;
    struct libevdev *touch_dev;

    ring_buf_t events_buf;
    input_multitouch_state_t multitouch_state;
    input_pen_state_t pen_state;

    bool poll_active;
    bool events_requested;
    pthread_t poll_thread;

    int screen_width;
    int screen_height;
} input_state_t;

int input_initialize(input_state_t *input_state, nsfb_t *nsfb);
int input_finalize(input_state_t *input_state);
bool input_get_next_event(input_state_t *input_state, nsfb_t *nsfb, nsfb_event_t *event, int timeout);

int input_get_next_multitouch_event(input_state_t *input_state);
int input_get_next_gpio_event(input_state_t *input_state);
int input_get_next_pen_event(input_state_t *input_state);
int input_identify_input_devices(input_state_t *input_state);
int input_read_next_event(struct input_event *ev, struct libevdev *dev);
int input_evdev_single_device_open(char *path, struct libevdev **dev);

/// Push updated touch position into buffer, if it has changed
int input_push_new_touch_position(input_state_t *input_state);
/// Push updated pen position into buffer, if it has changed
int input_push_new_pen_position(input_state_t *input_state);

void *input_async_handler(void *context);

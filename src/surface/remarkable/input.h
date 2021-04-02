#include <libevdev/libevdev.h>
#include <stdbool.h>

#include "libnsfb.h"
#include "ringbuf.h"

struct TouchEvent {
    enum { DOWN, UP } type;
};

/// Locations of the input devices
struct InputDevices {
    enum { RM1, RM2 } model;
    char* touch;
    char* pen;
    char* gpio;
};

struct MultitouchState {
    int min_x;
    int max_x;
    int min_y;
    int max_y;

    int current_slot;
    struct MultitouchSlotState *slots;
};

struct MultitouchSlotState {
    int tracking_id;
    bool tracking_id_changed;

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

    int tool_type;
    int tool_x;
    int tool_y;
};

bool get_next_event(nsfb_t *nsfb, nsfb_event_t *event);
int get_next_multitouch_event(nsfb_t *nsfb, ring_buf_t *buf);
int get_next_gpio_event(nsfb_t *nsfb, ring_buf_t *buf);
int get_next_pen_event(nsfb_t *nsfb, ring_buf_t *buf);
int read_next_event(struct input_event *ev, struct libevdev *dev);

int identify_input_devices(struct InputDevices *devices);
int evdev_open_all(void);
int evdev_close_all(void);
int evdev_single_device_open(char *path, struct libevdev **dev);

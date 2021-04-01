#include <libevdev/libevdev.h>

struct TouchEvent {
    enum { DOWN, UP } type;
};

/// Locations of the input devices
struct InputDevices {
    char* touch;
    char* pen;
    char* gpio;
};

int get_next_event(void);
int identify_input_devices(struct InputDevices *devices);
int evdev_open_all(void);
int evdev_close_all(void);
int evdev_single_device_open(char *path, struct libevdev **dev);

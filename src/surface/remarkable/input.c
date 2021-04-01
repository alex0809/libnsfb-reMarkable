#include <stdlib.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <libudev.h>
#include <linux/input.h>
#include <fcntl.h>
#include <errno.h>

#include "log.h"
#include "input.h"

struct libevdev *pen_dev = NULL;
struct libevdev *gpio_dev = NULL;
struct libevdev *touch_dev = NULL;

int get_next_event(void)
{
    struct input_event ev;

    int rc;
    rc = libevdev_next_event(pen_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc == -EAGAIN) 
        rc = libevdev_next_event(gpio_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    if (rc == -EAGAIN)
        rc = libevdev_next_event(touch_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

    if (rc == 0) {
        DEBUG_LOG("Event: %s %s %d",
            libevdev_event_type_get_name(ev.type),
            libevdev_event_code_get_name(ev.type, ev.code),
            ev.value);
    }
    return 0;
}

int evdev_open_all(void)
{
    if (gpio_dev != NULL && pen_dev != NULL && touch_dev != NULL) {
        DEBUG_LOG("evdev_open skipped: all devices are already open");
        return 0;
    }

    struct InputDevices devices;
    if (identify_input_devices(&devices) != 0) {
        ERROR_LOG("Could not determine input device locations.");
        return -1;
    }

    if (pen_dev == NULL)
    {
        if (evdev_single_device_open(devices.pen, &pen_dev) != 0) {
            return -1;
        }
        DEBUG_LOG("Pen device opened at %s", devices.pen);
    }
    if (gpio_dev == NULL)
    {
        if (evdev_single_device_open(devices.gpio, &gpio_dev) != 0) {
            return -1;
        }
        DEBUG_LOG("GPIO device opened at %s", devices.gpio);
    }
    if (touch_dev == NULL)
    {
        if (evdev_single_device_open(devices.touch, &touch_dev) != 0) {
            return -1;
        }
        DEBUG_LOG("Touch device opened at %s", devices.touch);
    }

    return 0;
}

int evdev_close_all(void)
{
    libevdev_free(gpio_dev);
    libevdev_free(pen_dev);
    libevdev_free(touch_dev);
    DEBUG_LOG("All evdev devices were closed");
    return 0;
}

int evdev_single_device_open(char *path, struct libevdev **dev)
{
    int fd = open(path, O_RDONLY|O_NONBLOCK) ;
    if (fd < 0) {
        ERROR_LOG("Could not open device file %s", path);
    }
    if (libevdev_new_from_fd(fd, dev) != 0) {
        ERROR_LOG("Could not create device from fd for path %s", path);
        return -1;
    }
    return 0;
}

int identify_input_devices(struct InputDevices *devices)
{
    // TODO actually identify those devices
    devices->pen = "/dev/input/event0";
    devices->touch = "/dev/input/event1";
    devices->gpio = "/dev/input/event2";
    return 0;
}

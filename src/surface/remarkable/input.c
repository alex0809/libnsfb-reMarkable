#include <stdlib.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <libudev.h>
#include <linux/input.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <stdbool.h>

#include "log.h"
#include "input.h"
#include "libnsfb_event.h"
#include "nsfb.h"

struct libevdev *pen_dev = NULL;
struct libevdev *gpio_dev = NULL;
struct libevdev *touch_dev = NULL;

struct InputDevices input_devices;
struct MultitouchState multi_touch_state;

ring_buf_t events_buf;

bool get_next_event(nsfb_t *nsfb, nsfb_event_t *event)
{
    if (ring_buf_read(&events_buf, event)) {
        return true;
    }

    if (get_next_multitouch_event(nsfb, &events_buf) > 0) {
        return false;
    }
    if (get_next_gpio_event(nsfb, &events_buf) > 0) {
        return false;
    }
    if (get_next_pen_event(nsfb, &events_buf) > 0) {
        return false;
    }

    return false;
}

int get_next_multitouch_event(nsfb_t *nsfb, ring_buf_t *buf)
{
    struct input_event ev;
    int read_res = read_next_event(&ev, touch_dev);
    if (read_res <= 0) {
        return read_res;
    }

    switch (ev.code) {
        case ABS_MT_TOUCH_MAJOR:
            multi_touch_state.slots[multi_touch_state.current_slot].touch_major = ev.value;
            break;
        case ABS_MT_TOUCH_MINOR:
            multi_touch_state.slots[multi_touch_state.current_slot].touch_minor = ev.value;
            break;
        case ABS_MT_WIDTH_MAJOR:
            multi_touch_state.slots[multi_touch_state.current_slot].width_major = ev.value;
            break;
        case ABS_MT_WIDTH_MINOR:
            multi_touch_state.slots[multi_touch_state.current_slot].width_minor = ev.value;
            break;
        case ABS_MT_PRESSURE:
            multi_touch_state.slots[multi_touch_state.current_slot].pressure = ev.value;
            break;
        case ABS_MT_DISTANCE:
            multi_touch_state.slots[multi_touch_state.current_slot].distance = ev.value;
            break;
        case ABS_MT_ORIENTATION:
            multi_touch_state.slots[multi_touch_state.current_slot].orientation = ev.value;
            break;
        case ABS_MT_POSITION_X:
            multi_touch_state.slots[multi_touch_state.current_slot].position_x = ev.value;
            multi_touch_state.slots[multi_touch_state.current_slot].position_changed = true;
            break;
        case ABS_MT_POSITION_Y:
            multi_touch_state.slots[multi_touch_state.current_slot].position_y = ev.value;
            multi_touch_state.slots[multi_touch_state.current_slot].position_changed = true;
            break;
        case ABS_MT_TRACKING_ID:
            multi_touch_state.slots[multi_touch_state.current_slot].tracking_id = ev.value;
            multi_touch_state.slots[multi_touch_state.current_slot].tracking_id_changed = true;
            break;
        case ABS_MT_TOOL_TYPE:
            multi_touch_state.slots[multi_touch_state.current_slot].tool_type = ev.value;
            break;
        case ABS_MT_TOOL_X:
            multi_touch_state.slots[multi_touch_state.current_slot].tool_x = ev.value;
            break;
        case ABS_MT_TOOL_Y:
            multi_touch_state.slots[multi_touch_state.current_slot].tool_y = ev.value;
            break;
        case ABS_MT_SLOT:
            multi_touch_state.current_slot = ev.value;
            break;
        case SYN_REPORT:
            // On SYN:
            // 1. If tracking_id has been released before the SYN, send corresponding event, then
            // 2. If position has changed, send coresponding event, then
            // 3. If tracking_id has been added before the SYN, send corresponding event
            ;
            struct MultitouchSlotState current_slot = multi_touch_state.slots[multi_touch_state.current_slot];

            if (current_slot.tracking_id_changed && current_slot.tracking_id == -1) {
                nsfb_event_t up_event;
                current_slot.tracking_id_changed = false;
                up_event.type = NSFB_EVENT_KEY_UP;
                up_event.value.keycode = NSFB_KEY_MOUSE_1;
                ring_buf_write(buf, &up_event);
            }

            if (current_slot.position_changed) {
                nsfb_event_t position_event;
                current_slot.position_changed = false;
                // TODO this is not the correct way to calculate the corresponding coordinates,
                // because visible width/height may be smaller than the full screen.
                int translated_x = nsfb->width - (multi_touch_state.slots[multi_touch_state.current_slot].position_x * 
                        nsfb->width / multi_touch_state.max_x);
                int translated_y = (multi_touch_state.slots[multi_touch_state.current_slot].position_y *
                        nsfb->height / multi_touch_state.max_y);
                // invert y-axis only on RM1
                if (input_devices.model == RM1) {
                    translated_y = nsfb->height - translated_y;
                }

                position_event.type = NSFB_EVENT_MOVE_ABSOLUTE;
                position_event.value.vector.x = translated_x;
                position_event.value.vector.y = translated_y;
                ring_buf_write(buf, &position_event);
            }

            if (current_slot.tracking_id_changed) {
                nsfb_event_t down_event;
                current_slot.tracking_id_changed = false;
                down_event.type = NSFB_EVENT_KEY_DOWN;
                down_event.value.keycode = NSFB_KEY_MOUSE_1;
                ring_buf_write(buf, &down_event);
            }

            return 1;
        default:
            DEBUG_LOG("Unhandled ev.code %d", ev.code);
    }
    return 0;
}

int get_next_gpio_event(nsfb_t *nsfb, ring_buf_t *buf)
{
    struct input_event ev;
    int read_res = read_next_event(&ev, gpio_dev);
    if (read_res <= 0) {
        return read_res;
    }

    TRACE_LOG("GPIO event unhandled!");
    return 1;
}

int get_next_pen_event(nsfb_t *nsfb, ring_buf_t *buf)
{
    struct input_event ev;
    int read_res = read_next_event(&ev, pen_dev);
    if (read_res <= 0) {
        return read_res;
    }

    TRACE_LOG("Pen event unhandled!");
    return 1;
}

int read_next_event(struct input_event *ev, struct libevdev *dev)
{
    int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, ev);
    if (rc == -EAGAIN) {
        return 0;
    }
    if (rc < 0) {
        ERROR_LOG("Error attempting to get next event. Errcode %d", rc);
        return -1;
    }
    return 1;
}

int evdev_open_all(void)
{
    if (gpio_dev != NULL && pen_dev != NULL && touch_dev != NULL) {
        DEBUG_LOG("evdev_open skipped: all devices are already open");
        return 0;
    }

    if (identify_input_devices(&input_devices) != 0) {
        ERROR_LOG("Could not determine input device locations.");
        return -1;
    }

    if (pen_dev == NULL)
    {
        if (evdev_single_device_open(input_devices.pen, &pen_dev) != 0) {
            return -1;
        }
        DEBUG_LOG("Pen device opened at %s", input_devices.pen);
    }
    if (gpio_dev == NULL)
    {
        if (evdev_single_device_open(input_devices.gpio, &gpio_dev) != 0) {
            return -1;
        }
        DEBUG_LOG("GPIO device opened at %s", input_devices.gpio);
    }
    if (touch_dev == NULL)
    {
        if (evdev_single_device_open(input_devices.touch, &touch_dev) != 0) {
            return -1;
        }
        int num_slots = libevdev_get_num_slots(touch_dev);
        if (num_slots == -1) {
            ERROR_LOG("Can't determine number of slots for multitouch device (%s)", input_devices.touch);
            return -1;
        }
        multi_touch_state.slots = malloc(sizeof(struct MultitouchSlotState) * num_slots);

        const struct input_absinfo *absinfo_x = libevdev_get_abs_info(touch_dev, ABS_MT_POSITION_X);
        const struct input_absinfo *absinfo_y = libevdev_get_abs_info(touch_dev, ABS_MT_POSITION_Y);

        multi_touch_state.min_x = absinfo_x->minimum;
        multi_touch_state.min_y = absinfo_y->minimum;
        multi_touch_state.max_x = absinfo_x->maximum;
        multi_touch_state.max_y = absinfo_y->maximum;

        DEBUG_LOG("Multitouch device opened at %s (%d slots, x between %d and %d, y between %d and %d)", 
                input_devices.touch, num_slots,
                absinfo_x->minimum, absinfo_x->maximum,
                multi_touch_state.min_y, multi_touch_state.max_y);
    }

    // initialize ringbuffer for events
    ring_buf_init(&events_buf, 50, sizeof(nsfb_event_t));

    return 0;
}

int evdev_close_all(void)
{
    libevdev_free(gpio_dev);
    libevdev_free(pen_dev);
    libevdev_free(touch_dev);
    ring_buf_free(&events_buf);
    DEBUG_LOG("All evdev devices and the event buffer were closed");

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
    devices->model = RM1;
    devices->pen = "/dev/input/event0";
    devices->touch = "/dev/input/event1";
    devices->gpio = "/dev/input/event2";
    return 0;
}


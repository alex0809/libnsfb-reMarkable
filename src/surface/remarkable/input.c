#include <stdlib.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <libudev.h>
#include <linux/input.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "log.h"
#include "input.h"
#include "libnsfb_event.h"
#include "nsfb.h"

struct timespec poll_sleep;

bool input_get_next_event(input_state_t *input_state, nsfb_t *nsfb, nsfb_event_t *event)
{
    // Value will probably only be available on next call of this function - that's fine
    sem_post(&input_state->event_requested);
    return ring_buf_read(&input_state->events_buf, event);
}

void *input_async_handler(void *context)
{
    input_state_t *input_state = (input_state_t*)context;
    while (input_state->poll_active) {
        while (input_get_next_multitouch_event(input_state) > 0);
        while (input_get_next_gpio_event(input_state) > 0);
        while (input_get_next_pen_event(input_state) > 0);

        int val;
        sem_getvalue(&input_state->event_requested, &val);
        if (val > 0) {
            input_push_new_touch_position(input_state);
            input_push_new_pen_position(input_state);
            sem_wait(&input_state->event_requested);
        }

        nanosleep(&poll_sleep, &poll_sleep);
    }

    return 0;
}

int input_get_next_multitouch_event(input_state_t *input_state)
{
    struct input_event ev;
    int read_res = input_read_next_event(&ev, input_state->touch_dev);
    if (read_res <= 0) {
        return read_res;
    }

    switch (ev.type) {
        case EV_ABS:
            switch (ev.code) {
                case ABS_MT_TOUCH_MAJOR:
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].touch_major = ev.value;
                    break;
                case ABS_MT_TOUCH_MINOR:
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].touch_minor = ev.value;
                    break;
                case ABS_MT_WIDTH_MAJOR:
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].width_major = ev.value;
                    break;
                case ABS_MT_WIDTH_MINOR:
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].width_minor = ev.value;
                    break;
                case ABS_MT_PRESSURE:
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].pressure = ev.value;
                    break;
                case ABS_MT_DISTANCE:
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].distance = ev.value;
                    break;
                case ABS_MT_ORIENTATION:
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].orientation = ev.value;
                    break;
                case ABS_MT_POSITION_X:
                    ;
                    int translated_x = input_state->screen_width - (ev.value * input_state->screen_width / input_state->multitouch_state.max_x);
                    if (input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].position_x == translated_x) {
                        break;
                    }

                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].position_x = translated_x;
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].position_changed = true;
                    break;
                case ABS_MT_POSITION_Y:
                    ;
                    int translated_y = (ev.value * input_state->screen_height / input_state->multitouch_state.max_y);
                    // invert y-axis only on RM1
                    if (input_state->model == RM1) {
                        translated_y = input_state->screen_height - translated_y;
                    }
                    if (input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].position_y == translated_y) {
                        break;
                    }

                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].position_y = translated_y;
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].position_changed = true;
                    break;
                case ABS_MT_TRACKING_ID:
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].tracking_id = ev.value;
                    input_state->multitouch_state.slots[input_state->multitouch_state.current_slot].tracking_id_changed = true;

                    // Set/reset the first pressed slot - all others will be effectively ignored
                    if (input_state->multitouch_state.first_pressed_slot == -1 && ev.value != -1) {
                        input_state->multitouch_state.first_pressed_slot = input_state->multitouch_state.current_slot;
                    }
                    if (input_state->multitouch_state.first_pressed_slot != input_state->multitouch_state.current_slot && ev.value == -1) {
                        input_state->multitouch_state.first_pressed_slot = -1;
                    }
                    break;
                case ABS_MT_SLOT:
                    input_state->multitouch_state.current_slot = ev.value;
                    break;
                default:
                    ERROR_LOG("input_get_next_multitouch_event: Unhandled ev.code %s for ABS event", libevdev_event_code_get_name(ev.type, ev.code));
                    return -1;
            }
            break;
        case EV_SYN:
            switch (ev.code) {
                case SYN_REPORT:
                    // TODO do I have to spell it out?
                    if (input_state->multitouch_state.first_pressed_slot != -1 && input_state->multitouch_state.current_slot != input_state->multitouch_state.first_pressed_slot) {
                        DEBUG_LOG("input_get_next_multitouch_event: Disregarding multitouch slot %d, I only care about first pressed slot %d", 
                                input_state->multitouch_state.current_slot, input_state->multitouch_state.first_pressed_slot);
                        break;
                    }

                    input_single_state_t *current_slot = &input_state->multitouch_state.slots[input_state->multitouch_state.current_slot];

                    // If the touch state of the first pressed slot has changed, push the updates immediately to buffer
                    // Otherwise, don't send new x/y coordinates here, we only do that if asked nicely for it

                    // On touch up, first transmit key up and then new position
                    if (current_slot->tracking_id_changed && current_slot->tracking_id == -1) {
                        input_state->multitouch_state.first_pressed_slot = -1;
                        nsfb_event_t up_event;
                        current_slot->tracking_id_changed = false;
                        up_event.type = NSFB_EVENT_KEY_UP;
                        up_event.value.keycode = NSFB_KEY_MOUSE_1;
                        ring_buf_write(&input_state->events_buf, &up_event);
                        TRACE_LOG("input_get_next_multitouch_event: Sent mouse up event (from touch)");

                        input_push_new_touch_position(input_state);
                    }

                    // On touch down, first transmit new position and then key down
                    if (current_slot->tracking_id_changed) {
                        input_push_new_touch_position(input_state);

                        input_state->multitouch_state.first_pressed_slot = input_state->multitouch_state.current_slot;
                        nsfb_event_t down_event;
                        current_slot->tracking_id_changed = false;
                        down_event.type = NSFB_EVENT_KEY_DOWN;
                        down_event.value.keycode = NSFB_KEY_MOUSE_1;
                        ring_buf_write(&input_state->events_buf, &down_event);
                        TRACE_LOG("input_get_next_multitouch_event: Sent mouse down event (from touch)");
                    }

                    break;
                case SYN_DROPPED:
                    // TODO recover from dropped event
                    DEBUG_LOG("input_get_next_multitouch_event: Events were dropped! State is now probably invalid.");
                    return -1;
                default:
                    ERROR_LOG("input_get_next_multitouch_event: Unhandled ev.code %s for SYN event", libevdev_event_code_get_name(ev.type, ev.code));
                    return -1;
            }
            break;
        default:
            ERROR_LOG("input_get_next_multitouch_event: received unexpected event type %s", libevdev_event_type_get_name(ev.type));
            return -1;
    }
    return 1;
}

int input_push_new_touch_position(input_state_t *input_state)
{
    if (input_state->multitouch_state.first_pressed_slot == -1 ||
            !input_state->multitouch_state.slots[input_state->multitouch_state.first_pressed_slot].position_changed) {
        return 0;
    }
    
    nsfb_event_t position_event;
    input_single_state_t *single_state = &input_state->multitouch_state.slots[input_state->multitouch_state.first_pressed_slot];
    single_state->position_changed = false;

    position_event.type = NSFB_EVENT_MOVE_ABSOLUTE;
    position_event.value.vector.x = single_state->position_x;
    position_event.value.vector.y = single_state->position_y;
    TRACE_LOG("input_push_new_touch_position: Updating touch position: x=%d, y=%d", single_state->position_x, single_state->position_y);
    ring_buf_write(&input_state->events_buf, &position_event);
    return 1;
}

int input_get_next_gpio_event(input_state_t *input_state)
{
    struct input_event ev;
    int read_res = input_read_next_event(&ev, input_state->gpio_dev);
    if (read_res <= 0) {
        return read_res;
    }

    switch (ev.type) {
        case EV_KEY:
            switch (ev.code) {
                case KEY_LEFT:
                    ;
                    nsfb_event_t left_event;
                    if (ev.value == 0) {
                        left_event.type = NSFB_EVENT_KEY_UP;
                    } else {
                        left_event.type = NSFB_EVENT_KEY_DOWN;
                    }
                    left_event.value.keycode = NSFB_KEY_PAGEUP;
                    ring_buf_write(&input_state->events_buf, &left_event);
                    TRACE_LOG("input_get_next_gpio_event: Sent gpio pgup event");
                    break;
                case KEY_HOME:
                    ;
                    nsfb_event_t home_event;
                    if (ev.value == 0) {
                        home_event.type = NSFB_EVENT_KEY_UP;
                    } else {
                        home_event.type = NSFB_EVENT_KEY_DOWN;
                    }
                    home_event.value.keycode = NSFB_KEY_HOME;
                    ring_buf_write(&input_state->events_buf, &home_event);
                    TRACE_LOG("input_get_next_gpio_event: Sent gpio home event");
                    break;
                case KEY_RIGHT:
                    ;
                    nsfb_event_t right_event;
                    if (ev.value == 0) {
                        right_event.type = NSFB_EVENT_KEY_UP;
                    } else {
                        right_event.type = NSFB_EVENT_KEY_DOWN;
                    }
                    right_event.value.keycode = NSFB_KEY_PAGEDOWN;
                    ring_buf_write(&input_state->events_buf, &right_event);
                    TRACE_LOG("input_get_next_gpio_event: Sent gpio pgdown event");
                    break;
                default:
                    ERROR_LOG("input_get_next_multitouch_event: Unhandled ev.code %s for KEY event", libevdev_event_code_get_name(ev.type, ev.code));
                    return -1;
            }
            break;
        case EV_SYN:
            // let's just handle it directly on EV_KEY for now
            break;
        default:
            ERROR_LOG("input_get_next_gpio_event: received unexpected event type %s", libevdev_event_type_get_name(ev.type));
            return -1;
    }

    return 0;
}

int input_get_next_pen_event(input_state_t *input_state)
{
    struct input_event ev;
    int read_res = input_read_next_event(&ev, input_state->pen_dev);
    if (read_res <= 0) {
        return read_res;
    }

    switch (ev.type) {
        case EV_ABS:
            switch (ev.code) {
                case ABS_DISTANCE:
                    input_state->pen_state.distance = ev.value;
                    break;
                case ABS_X:
                    ;
                    int translated_y = input_state->screen_height - (ev.value * input_state->screen_height / input_state->pen_state.max_x);
                    if (input_state->pen_state.position_y == translated_y) {
                        break;
                    }

                    input_state->pen_state.position_y = translated_y;
                    input_state->pen_state.position_changed = true;
                    break;
                case ABS_Y:
                    ;
                    int translated_x = (ev.value * input_state->screen_width / input_state->pen_state.max_y);
                    if (input_state->pen_state.position_x == translated_x) {
                        break;
                    }

                    input_state->pen_state.position_x = translated_x;
                    input_state->pen_state.position_changed = true;
                    break;
                case ABS_PRESSURE:
                    input_state->pen_state.pressure = ev.value;
                    break;
                case ABS_TILT_X:
                    break;
                case ABS_TILT_Y:
                    break;
                default:
                    ERROR_LOG("input_get_next_pen_event: Unhandled ev.code %s for ABS event", libevdev_event_code_get_name(ev.type, ev.code));
                    return -1;
            }
            break;
        case EV_KEY:
            switch (ev.code) {
                case BTN_TOOL_PEN:
                    // noop for now
                    break;
                case BTN_TOUCH:
                    if (ev.value == 1) {
                        input_state->pen_state.touched = true;
                        input_state->pen_state.touch_state_changed = true;
                    } else {
                        input_state->pen_state.touched = false;
                        input_state->pen_state.touch_state_changed = true;
                    }
                    break;
                default:
                    ERROR_LOG("input_get_next_pen_event: Unhandled ev.code %s for KEY event", libevdev_event_code_get_name(ev.type, ev.code));
                    return -1;
            }
            break;
        case EV_SYN:
            switch (ev.code) {
                case SYN_REPORT:
                    // If the touch state of the pen has changed, push the updates immediately to buffer
                    // Otherwise, don't send new x/y coordinates here, we only do that if asked nicely for it

                    // On touch up, first transmit key up and then new position
                    if (input_state->pen_state.touch_state_changed && !input_state->pen_state.touched) {
                        nsfb_event_t up_event;
                        input_state->pen_state.touch_state_changed = false;
                        up_event.type = NSFB_EVENT_KEY_UP;
                        up_event.value.keycode = NSFB_KEY_MOUSE_1;
                        ring_buf_write(&input_state->events_buf, &up_event);
                        TRACE_LOG("input_get_next_pen_event: Sent mouse up event (from pen touch)");

                        input_push_new_pen_position(input_state);
                    }

                    // On touch down, first transmit new position and then key down
                    if (input_state->pen_state.touch_state_changed) {
                        input_push_new_pen_position(input_state);

                        input_state->multitouch_state.first_pressed_slot = input_state->multitouch_state.current_slot;
                        nsfb_event_t down_event;
                        input_state->pen_state.touch_state_changed = false;
                        down_event.type = NSFB_EVENT_KEY_DOWN;
                        down_event.value.keycode = NSFB_KEY_MOUSE_1;
                        ring_buf_write(&input_state->events_buf, &down_event);
                        TRACE_LOG("input_get_next_pen_event: Sending mouse down event (from pen touch)");
                    }
                    break;
                case SYN_DROPPED:
                    // TODO recover from dropped event
                    DEBUG_LOG("input_get_next_pen_event: Events were dropped! State is now probably invalid.");
                    return -1;
                default:
                    ERROR_LOG("input_get_next_pen_event: Unhandled event code %s for SYN event", libevdev_event_code_get_name(ev.type, ev.code));
                    return -1;
            }
            break;
        default:
            ERROR_LOG("input_get_next_pen_event: received unexpected event type %s", libevdev_event_type_get_name(ev.type));
            return -1;
        }
    return 1;
}

int input_push_new_pen_position(input_state_t *input_state)
{
    if (!input_state->pen_state.position_changed) {
        return 0;
    }

    input_state->pen_state.position_changed = false;

    nsfb_event_t position_event;
    position_event.type = NSFB_EVENT_MOVE_ABSOLUTE;
    position_event.value.vector.x = input_state->pen_state.position_x;
    position_event.value.vector.y = input_state->pen_state.position_y;
    TRACE_LOG("input_get_next_pen_event: Updating touch position: x=%d, y=%d", 
            input_state->pen_state.position_x, input_state->pen_state.position_y);
    ring_buf_write(&input_state->events_buf, &position_event);
    return 1;
}


int input_read_next_event(struct input_event *ev, struct libevdev *dev)
{
    int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, ev);
    if (rc == -EAGAIN) {
        return 0;
    }
    if (rc < 0) {
        ERROR_LOG("input_read_next_event: error attempting to get next event. Errcode %d", rc);
        return -1;
    }
    return 1;
}

int input_initialize(input_state_t *input_state, nsfb_t *nsfb)
{
    if (input_identify_input_devices(input_state) != 0) {
        ERROR_LOG("input_initialize: Could not determine input device locations.");
        return -1;
    }

    int num_slots = libevdev_get_num_slots(input_state->touch_dev);
    if (num_slots == -1) {
        ERROR_LOG("input_initialize: Can't determine number of slots for multitouch device");
        return -1;
    }
    input_state->multitouch_state.slots = malloc(sizeof(input_single_state_t) * num_slots);

    const struct input_absinfo *absinfo_x = libevdev_get_abs_info(input_state->touch_dev, ABS_MT_POSITION_X);
    const struct input_absinfo *absinfo_y = libevdev_get_abs_info(input_state->touch_dev, ABS_MT_POSITION_Y);

    input_state->multitouch_state.min_x = absinfo_x->minimum;
    input_state->multitouch_state.min_y = absinfo_y->minimum;
    input_state->multitouch_state.max_x = absinfo_x->maximum;
    input_state->multitouch_state.max_y = absinfo_y->maximum;
    input_state->multitouch_state.first_pressed_slot = -1;

    DEBUG_LOG("input_initialize: Multitouch device has %d slots, x is between %d and %d, y is between %d and %d)", 
            num_slots, input_state->multitouch_state.min_x, input_state->multitouch_state.max_x,
            input_state->multitouch_state.min_y, input_state->multitouch_state.max_y);

    const struct input_absinfo *pen_absinfo_x = libevdev_get_abs_info(input_state->pen_dev, ABS_X);
    const struct input_absinfo *pen_absinfo_y = libevdev_get_abs_info(input_state->pen_dev, ABS_Y);

    input_state->pen_state.min_x = pen_absinfo_x->minimum;
    input_state->pen_state.min_y = pen_absinfo_y->minimum;
    input_state->pen_state.max_x = pen_absinfo_x->maximum;
    input_state->pen_state.max_y = pen_absinfo_y->maximum;

    DEBUG_LOG("input_initialize: pen device x is between %d and %d, y is between %d and %d)", 
            input_state->pen_state.min_x, input_state->pen_state.max_x,
            input_state->pen_state.min_y, input_state->pen_state.max_y);

    // initialize ringbuffer for events
    ring_buf_init(&input_state->events_buf, 50, sizeof(nsfb_event_t));

    // set width/height (assume these are unchanging for now)
    input_state->screen_height = nsfb->height;
    input_state->screen_width = nsfb->width;

    poll_sleep.tv_nsec = 10000000;
    poll_sleep.tv_sec = 0;
    input_state->poll_active = true;
    sem_t sem;
    sem_init(&sem, 0, 0);
    input_state->event_requested = sem;
    pthread_t thread;
    int thread_create_result = pthread_create(&thread, NULL, input_async_handler, input_state);
    if (thread_create_result != 0) {
        ERROR_LOG("input_initialize: could not initialize async poll thread");
        return -1;
    }
    input_state->poll_thread = thread;

    return 0;
}

int input_finalize(input_state_t *input_state)
{
    close(libevdev_get_fd(input_state->gpio_dev));
    close(libevdev_get_fd(input_state->pen_dev));
    close(libevdev_get_fd(input_state->touch_dev));
    libevdev_free(input_state->gpio_dev);
    libevdev_free(input_state->pen_dev);
    libevdev_free(input_state->touch_dev);
    ring_buf_free(&input_state->events_buf);
    DEBUG_LOG("input_finalize: All evdev devices and the event buffer were closed");

    input_state->poll_active = false;
    pthread_join(input_state->poll_thread, NULL);
    DEBUG_LOG("input_finalize: Poll thread stopped");

    return 0;
}

int input_evdev_single_device_open(char *path, struct libevdev **dev)
{
    int fd = open(path, O_RDONLY|O_NONBLOCK);
    if (fd < 0) {
        ERROR_LOG("input_evdev_single_device_open: Could not open device file %s", path);
        return -1;
    }
    if (libevdev_new_from_fd(fd, dev) < 0) {
        ERROR_LOG("input_evdev_single_device_open: Could not create device from fd for path %s", path);
        return -1;
    }
    return 0;
}

int input_identify_input_devices(input_state_t *input_state)
{
    // Thanks to https://github.com/canselcik/libremarkable/
    // for providing an example of what to check for
    
    // Determine the model by machine name
    int dev_name_fd = open("/sys/devices/soc0/machine", O_RDONLY|O_NONBLOCK);
    if (dev_name_fd < 0) {
        ERROR_LOG("input_identify_input_devices: Can't open machine name file");
        return -1;
    }
    char machine_name[50];
    int len_read = read(dev_name_fd, machine_name, sizeof(machine_name));
    close(dev_name_fd);
    machine_name[len_read] = '\0';
    if (strncmp(RM1_MACHINE_NAME_1, machine_name, strlen(RM1_MACHINE_NAME_1)) == 0 || strncmp(RM1_MACHINE_NAME_2, machine_name, strlen(RM1_MACHINE_NAME_2)) == 0) {
        DEBUG_LOG("input_identify_input_devices: Device identified as RM1");
        input_state->model = RM1;
    }
    else if (strncmp(RM2_MACHINE_NAME, machine_name, strlen(RM2_MACHINE_NAME)) == 0) {
        DEBUG_LOG("input_identify_input_devices: Device identified as RM2");
        input_state->model = RM2;
    }
    else {
        ERROR_LOG("input_identify_input_devices: Could not identify device type (machine name: %s)", machine_name);
        return -1;
    }

    // Iterate through all event-devices and match them by supported events
    struct dirent *entry;
    DIR *dev_dir = opendir(EVENTS_DIR);
    if (dev_dir == NULL) {
        ERROR_LOG("input_identify_input_devices: Can't open device directory.");
        return -1;
    }
    char filename[PATH_MAX+1];
    while ((entry = readdir(dev_dir)) != NULL) {
        if (strncmp("event", entry->d_name, strlen("event")) == 0) {
            sprintf(filename, "%s/%s", EVENTS_DIR, entry->d_name);
            int fd = open(filename, O_RDONLY|O_NONBLOCK);
            if (fd < 0) {
                ERROR_LOG("input_identify_input_devices: Could not open device file %s (errno %d)", filename, errno);
                return -1;
            }
            struct libevdev *dev;
            if (libevdev_new_from_fd(fd, &dev) < 0) {
                ERROR_LOG("input_identify_input_devices: Could not create device from fd for path %s", filename);
                return -1;
            }
            if (libevdev_has_event_code(dev, EV_ABS, ABS_MT_SLOT)) {
                DEBUG_LOG("input_identify_input_devices: Identified %s as multitouch input", filename);
                input_state->touch_dev = dev;
            }
            if (libevdev_has_event_code(dev, EV_KEY, KEY_POWER)) {
                DEBUG_LOG("input_identify_input_devices: Identified %s as GPIO input", filename);
                input_state->gpio_dev = dev;
            }
            if (libevdev_has_event_code(dev, EV_KEY, BTN_STYLUS)) {
                DEBUG_LOG("input_identify_input_devices: Identified %s as pen input", filename);
                input_state->pen_dev = dev;
            }
        }
    }
    if (input_state->pen_dev == NULL || input_state->touch_dev == NULL || input_state->gpio_dev == NULL) {
        ERROR_LOG("input_identify_input_devices: Could not identify all required devices.");
        return -1;
    }
    DEBUG_LOG("input_identify_input_devices: Identified all devices.");

    return 0;
}


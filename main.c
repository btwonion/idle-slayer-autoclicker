#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

XWindowAttributes *scan_for_idle_slayer_window(Display *display, Window *root_window, Window **child_window) {
    unsigned int i, children_number;
    Window root, parent, *children;
    XWindowAttributes window_attributes;
    char *children_name;

    XQueryTree(display, *root_window, &root, &parent, &children, &children_number);

    for (i = 0; i < children_number; i++) {
        if (!XGetWindowAttributes(display, children[i], &window_attributes) || window_attributes.map_state != IsViewable) continue;
        XFetchName(display, children[i], &children_name);
        if (children_name == NULL) continue;
        if (strcmp(children_name, "Idle Slayer") == 0) {
            *child_window = malloc(sizeof(Window));
            **child_window = children[i];
            XWindowAttributes *alloc_attributes = malloc(sizeof(window_attributes));
            *alloc_attributes = window_attributes;

            XFree(children);
            XFree(children_name);
            return alloc_attributes;
        }
    }

    if (children) XFree(children);
    return NULL;
}

XWindowAttributes *wait_for_is_window(Display *display, Window *root_window, Window **child_window) {
    XWindowAttributes *local_attributes = NULL; 

    printf("Start scanning for Idle Slayer window\n");
    while (local_attributes == NULL) {
        local_attributes = scan_for_idle_slayer_window(display, root_window, child_window);
        if (local_attributes != NULL) {
            printf("Found Idle Slayer's window\n");
            return local_attributes;
        }

        printf("Didn't find Idle Slayer. Waiting a sec and checking again!\n");
        usleep(1E6);
    }
}

typedef struct {
    Bool *is_stopped;
    Display *display;
    Window *window;
} thread_param;

void *create_keyboard_event_thread(void *thr_data) {
    XEvent some_event;
    thread_param *param_handle = thr_data;

    XSelectInput(param_handle->display, *param_handle->window, KeyPressMask);

    printf("Launched keyboard listener in new thread!\n");
    while (true) {
        XNextEvent(param_handle->display, &some_event);

        switch (some_event.type) {
            case KeyPress: 
                if (XKeysymToKeycode(param_handle->display, XK_K) == some_event.xkey.keycode) {
                    *param_handle->is_stopped = !*param_handle->is_stopped;
                }
                
                break;
        }
    }
}

void click_in_center(Display *display, Window *root_window, Window *is_window, XWindowAttributes *is_attributes, int cps) {
    int center_x = is_attributes->x + is_attributes->width / 2, center_y = is_attributes->y + is_attributes->height / 2;

    XEvent event;

    memset(&event, 0, sizeof(event));

    event.xbutton.type = ButtonPress;
    event.xbutton.window = *is_window;
    event.xbutton.root = *root_window;
    event.xbutton.subwindow = None;
    event.xbutton.time = CurrentTime;
    event.xbutton.x = is_attributes->width / 2;
    event.xbutton.y = is_attributes->height / 2;
    event.xbutton.x_root = center_x; 
    event.xbutton.y_root = center_y;
    event.xbutton.state = 0;
    event.xbutton.button = Button1;
    event.xbutton.same_screen = true;

    printf("Start clicking\n");

    Bool *is_stopped = malloc(sizeof(Bool));
    *is_stopped = false;

    thread_param *param = malloc(sizeof(thread_param));
    param->display = display;
    param->is_stopped = is_stopped;
    param->window = is_window;
    pthread_t thread;
    pthread_create(&thread, NULL, create_keyboard_event_thread, param);

    long ms_interval = 1E6 / cps;
    while (true) {
        if (*is_stopped) {
            usleep(1E5);
            continue;
        }
        
        event.type = ButtonPress;
        XSendEvent(display, PointerWindow, True, ButtonPressMask, &event);
        XFlush(display);

        usleep(5E4);

        event.type = ButtonRelease;
        XSendEvent(display, PointerWindow, True, ButtonReleaseMask, &event);
        XFlush(display);

        usleep(ms_interval);
    }
}

int main(int argc, char *argv[]) {
    XInitThreads();
    int cps = atoi(argv[1]);

    printf("Starting auto clicker with CPS of %i\n", cps);
    printf("Try accessing the display\n");
    Display *display = XOpenDisplay(NULL);

    if (display == NULL) {
        printf("Connection to XServer failed!\n");
        return 1;
    }

    printf("Successfully connected to XServer\n");

    Window root_window = XDefaultRootWindow(display);

    Window *is_window;
    XWindowAttributes *is_attributes = wait_for_is_window(display, &root_window, &is_window);

    click_in_center(display, &root_window, is_window, is_attributes, cps);

    XCloseDisplay(display);
    printf("Closed connection to XServer\n");
    return 0;
}
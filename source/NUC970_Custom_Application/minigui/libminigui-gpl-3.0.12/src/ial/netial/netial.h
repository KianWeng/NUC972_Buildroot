
#ifndef GUI_IAL_NET_H
    #define GUI_IAL_NET_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
#include "customial.h"

#define IAL_MOUSE_LEFTBUTTON    1
#define IAL_MOUSE_RIGHTBUTTON   2
#define IAL_MOUSE_MIDDLEBUTTON  4
#define IAL_MOUSE_FOURTHBUTTON  8
#define IAL_MOUSE_FIFTHBUTTON   16
#define IAL_MOUSE_SIXTHBUTTON   32
#define IAL_MOUSE_RESETBUTTON   64

#define IAL_MOUSEEVENT          1
#define IAL_KEYEVENT            2
/*
typedef struct tagINPUT
{
    char*   id;

    // Initialization and termination
    BOOL (*init_input) (struct tagINPUT *input, const char* mdev, const char* mtype);
    void (*term_input) (void);

    // Mouse operations
    int  (*update_mouse) (void);
    void (*get_mouse_xy) (int* x, int* y);
    void (*set_mouse_xy) (int x, int y);
    int  (*get_mouse_button) (void);
    void (*set_mouse_range) (int minx, int miny, int maxx, int maxy);
    void (*suspend_mouse) (void);
    int (*resume_mouse) (void);

    // Keyboard operations
    int  (*update_keyboard) (void);
    const char* (*get_keyboard_state) (void);
    void (*suspend_keyboard) (void);
    int (*resume_keyboard) (void);
    void (*set_leds) (unsigned int leds);

    // Event
    int (*wait_event) (int which, int maxfd, fd_set *in, fd_set *out, 
            fd_set *except, struct timeval *timeout);

    char mdev [MAX_PATH + 1];
}INPUT;
*/
#ifdef _MGIAL_NET
extern BOOL InitNetInput (INPUT* input, const char* mdev, const char* mtype);
extern void TermNetInput (void);
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* GUI_IAL_CUSTOM_H */



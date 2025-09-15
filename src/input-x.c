#include "../include/input.h"

#include <X11/Xlib.h>

extern Display *g_dpy;
extern Window   g_win;
static int g_grab = 0;

void input_init(int grab) {
	g_grab = grab ? 1 : 0;

	long mask = KeyPressMask | KeyReleaseMask
		| ButtonPressMask | ButtonReleaseMask
		| PointerMotionMask | StructureNotifyMask;

	XSelectInput(g_dpy, g_win, mask);
	XSync(g_dpy, False);

	if (!g_grab)
		return;

	XGrabKeyboard(g_dpy, g_win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
	XGrabPointer(g_dpy, g_win, True,
			ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
	XSync(g_dpy, False);
}

void input_poll(void) {

	while (XPending(g_dpy)) {
		XEvent ev;
		XNextEvent(g_dpy, &ev);

		switch (ev.type) {
		case KeyPress: {
			int kc = ev.xkey.keycode;
			input_call(kc - 8, 1, 1);
		} break;
		case KeyRelease: {
			int kc = ev.xkey.keycode;
			input_call(kc - 8, 0, 1);
		} break;
		case ButtonPress: {
			int btn = ev.xbutton.button;
			input_call(btn, 1, 2);
		} break;
		case ButtonRelease: {
			int btn = ev.xbutton.button;
			input_call(btn, 0, 2);
		} break;
		case MotionNotify: {
			/* pack x/y in a simple way if precisas; aqui só sinalizamos movimento */
			input_call(0, 0, 3);
		} break;
		case ConfigureNotify: {
			/* ev.xconfigure.width/height disponíveis se precisares */
			input_call(0, 0, 4);
		} break;
		default: break;
		}
	}
}

void input_deinit(void) {
	if (!g_grab)
		return;

	XUngrabKeyboard(g_dpy, CurrentTime);
	XUngrabPointer(g_dpy, CurrentTime);
	XSync(g_dpy, False);
}

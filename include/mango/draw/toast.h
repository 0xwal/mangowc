#ifndef __MANGO_DRAW_TOAST_H__
#define __MANGO_DRAW_TOAST_H__ 1

/* [fork] toast overlay: reusable centered, non-interactive label flashed on a
 * monitor or client window (e.g. layer-keybind feedback); styled like the jump
 * label, hard shown, auto-hidden after ~500ms. */

struct Monitor;
struct Client;

void mango_toast_show(struct Monitor *mon, const char *text);
void mango_toast_show_client_centered(struct Client *c, const char *text);
void mango_toast_destroy(struct Monitor *mon);

#endif // __MANGO_DRAW_TOAST_H__

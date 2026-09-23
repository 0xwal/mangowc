#include "mango/draw/toast.h"

#include <stdlib.h>
#include <wayland-server-core.h>

#include "mango/common/server.h"
#include "mango/config/parse_config.h"
#include "mango/draw/text-node.h"
#include "mango/manage/client.h"
#include "mango/manage/monitor.h"

/* [fork] toast overlay: display duration in milliseconds. */
#define MANGO_TOAST_MS 500

struct mango_toast {
	struct Monitor *mon;
	MangoJumpLabel *label;
	struct wl_event_source *timer;
};

static void toast_teardown(struct mango_toast *t) {
	if (t->timer) {
		wl_event_source_remove(t->timer);
		t->timer = NULL;
	}
	if (t->label) {
		mango_jump_label_node_destroy(t->label);
		t->label = NULL;
	}
	if (t->mon)
		t->mon->toast = NULL;
	free(t);
}

static int32_t toast_expire_cb(void *data) {
	toast_teardown(data);
	return 1;
}

static void toast_show_at(struct Monitor *mon, const char *text, int mid_x,
						  int mid_y) {
	if (!mon || !text || !text[0])
		return;

	/* [fork] toast overlay: last-wins replace of any active toast */
	if (mon->toast)
		toast_teardown(mon->toast);

	MangoJumpLabel *label =
		mango_jump_label_node_create(server.layers[LyrOverlay],
									 config.jumplabeldata);
	if (!label)
		return;

	mango_jump_label_node_update(label, text, mon->wlr_output->scale);

	/* raster failure: the label never got a valid size */
	if (label->logical_width <= 0 || label->logical_height <= 0) {
		mango_jump_label_node_destroy(label);
		return;
	}

	wlr_scene_node_set_position(&label->scene->node,
								mid_x - label->logical_width / 2,
								mid_y - label->logical_height / 2);
	wlr_scene_node_raise_to_top(&label->scene->node);
	wlr_scene_node_set_enabled(&label->scene->node, true);

	struct mango_toast *t = calloc(1, sizeof(struct mango_toast));
	if (!t) {
		mango_jump_label_node_destroy(label);
		return;
	}
	t->mon = mon;
	t->label = label;
	t->timer = wl_event_loop_add_timer(server.event_loop, toast_expire_cb, t);
	if (!t->timer) {
		toast_teardown(t);
		return;
	}
	wl_event_source_timer_update(t->timer, MANGO_TOAST_MS);
	mon->toast = t;
}

void mango_toast_show(struct Monitor *mon, const char *text) {
	if (!mon)
		return;
	toast_show_at(mon, text, mon->m.x + mon->m.width / 2,
				  mon->m.y + mon->m.height / 2);
}

/* [fork] toast overlay: center on the client window (geom is layout-relative) */
void mango_toast_show_client_centered(struct Client *c, const char *text) {
	if (!c || !c->mon || !text || !text[0])
		return;
	toast_show_at(c->mon, text, c->geom.x + c->geom.width / 2,
				  c->geom.y + c->geom.height / 2);
}

void mango_toast_destroy(struct Monitor *mon) {
	if (mon && mon->toast)
		toast_teardown(mon->toast);
}

/*
    This file is part of darktable,
    Copyright (C) 2011-2021 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common/ratings.h"
#include "common/collection.h"
#include "common/debug.h"
#include "common/selection.h"
#include "control/control.h"
#include "dtgtk/button.h"
#include "gui/draw.h"
#include "gui/gtk.h"
#include "gui/accelerators.h"
#include "libs/lib.h"
#include "libs/lib_api.h"

DT_MODULE(1)

typedef struct dt_lib_ratings_t
{
    gint current;
    gint rating;
    gint pointerx;
    gint pointery;
    GtkWidget *drawing;
} dt_lib_ratings_t;

/* redraw the ratings */
static void _lib_ratings_draw_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height,
                                       gpointer user_data);
/* motion notify handler*/
static void _lib_ratings_motion_notify_callback(GtkEventControllerMotion *controller, double x,
                                                double y, gpointer user_data);
/* motion leavel handler */
static void _lib_ratings_leave_notify_callback(GtkEventControllerMotion *controller,
                                               gpointer user_data);
/* button press handler */
static void _lib_ratings_button_press_callback(GtkGestureSingle *gesture, int n_press, double x,
                                               double y, gpointer user_data);

static int _selected_rating(void)
{
    GList *imgs = dt_selection_get_list(darktable.selection, FALSE, FALSE);
    int rating = -1;
    for (GList *image = imgs; image; image = g_list_next(image))
    {
        const int image_rating = dt_ratings_get(GPOINTER_TO_INT(image->data));
        if (rating < 0)
            rating = image_rating;
        else if (rating != image_rating)
        {
            rating = -1;
            break;
        }
    }
    g_list_free(imgs);
    return rating;
}

static void _ratings_selection_changed(gpointer instance, dt_lib_module_t *self)
{
    dt_lib_gui_queue_update(self);
}

static void _ratings_metadata_changed(gpointer instance, const int type, dt_lib_module_t *self)
{
    dt_lib_gui_queue_update(self);
}

const char *name(dt_lib_module_t *self)
{
    return _("ratings");
}

dt_view_type_flags_t views(dt_lib_module_t *self)
{
    return DT_VIEW_LIGHTTABLE;
}

uint32_t container(dt_lib_module_t *self)
{
    return DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_RIGHT;
}

gboolean expandable(dt_lib_module_t *self)
{
    return FALSE;
}

int position(const dt_lib_module_t *self)
{
    return 1002;
}

void gui_init(dt_lib_module_t *self)
{
    /* initialize ui widgets */
    dt_lib_ratings_t *d = g_malloc0(sizeof(dt_lib_ratings_t));
    self->data = (void *)d;

    self->widget = GTK_WIDGET(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_widget_set_halign(self->widget, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(self->widget, GTK_ALIGN_CENTER);

    GtkWidget *drawing = gtk_drawing_area_new();
    d->drawing = drawing;

    /* connect callbacks */
    gtk_widget_set_tooltip_text(drawing, _("set star rating for selected images"));
#if !GTK_CHECK_VERSION(4, 0, 0)
    gtk_widget_set_app_paintable(drawing, TRUE);
#endif
    dt_gui_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing), _lib_ratings_draw_callback, self,
                                      NULL);
    dt_gui_connect_click_all(drawing, _lib_ratings_button_press_callback, NULL, self);
    dt_gui_connect_motion(drawing, _lib_ratings_motion_notify_callback, NULL,
                          _lib_ratings_leave_notify_callback, self);

    gtk_box_pack_start(GTK_BOX(self->widget), drawing, TRUE, TRUE, 0);

    /* set size of navigation draw area */
    gtk_widget_set_name(self->widget, "lib-rating-stars");
    dt_action_t *ac = dt_action_define(&darktable.control->actions_thumb, NULL, N_("rating"),
                                       drawing, &dt_action_def_rating);
    dt_shortcut_register(ac, 0, 0, GDK_KEY_0, 0);
    dt_shortcut_register(ac, 1, 0, GDK_KEY_1, 0);
    dt_shortcut_register(ac, 2, 0, GDK_KEY_2, 0);
    dt_shortcut_register(ac, 3, 0, GDK_KEY_3, 0);
    dt_shortcut_register(ac, 4, 0, GDK_KEY_4, 0);
    dt_shortcut_register(ac, 5, 0, GDK_KEY_5, 0);
    dt_shortcut_register(ac, 6, 0, GDK_KEY_r, 0);

    DT_CONTROL_SIGNAL_HANDLE(DT_SIGNAL_SELECTION_CHANGED, _ratings_selection_changed);
    DT_CONTROL_SIGNAL_HANDLE(DT_SIGNAL_METADATA_CHANGED, _ratings_metadata_changed);
}

void gui_cleanup(dt_lib_module_t *self)
{
    g_free(self->data);
    self->data = NULL;
}

static void _lib_ratings_draw_callback(GtkDrawingArea *area, cairo_t *cr, int width, int height,
                                       gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET(area);
    dt_lib_module_t *self = (dt_lib_module_t *)user_data;
    dt_lib_ratings_t *d = self->data;

    if (!dt_control_running())
        return;

    const float star_size = height;
    const float star_spacing = (width - 5.0 * star_size) / 4.0;

#if !GTK_CHECK_VERSION(4, 0, 0)
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_render_background(context, cr, 0, 0, width, height);
#endif

    /* get current style */
    GdkRGBA fg_color;
    dt_gui_widget_get_color(widget, &fg_color);

    /* lets draw stars */
    int x = 0;
    cairo_set_line_width(cr, DT_PIXEL_APPLY_DPI(1));
    gdk_cairo_set_source_rgba(cr, &fg_color);
    const gboolean hovering = d->pointerx > 0;
    d->current = 0;
    for (int k = 0; k < 5; k++)
    {
        /* outline star */
        dt_draw_star(cr, star_size / 2.0 + x, star_size / 2.0, star_size / 2.0,
                     star_size / (2.0 * 2.5));
        const gboolean active = hovering ? x < d->pointerx : k < d->rating;
        if (active)
        {
            cairo_fill_preserve(cr);
            cairo_set_source_rgba(cr, fg_color.red, fg_color.green, fg_color.blue,
                                  fg_color.alpha * (hovering ? 0.5 : 1.0));
            cairo_stroke(cr);
            gdk_cairo_set_source_rgba(cr, &fg_color);
            if (hovering && (k + 1) > d->current)
                d->current = darktable.control->element = (k + 1);
        }
        else
            cairo_stroke(cr);
        x += star_size + star_spacing;
    }
}

void gui_update(dt_lib_module_t *self)
{
    dt_lib_ratings_t *d = self->data;
    d->rating = _selected_rating();
    gtk_widget_queue_draw(d->drawing);
}

static void _lib_ratings_motion_notify_callback(GtkEventControllerMotion *controller, double x,
                                                double y, gpointer user_data)
{
    (void)controller;
    dt_lib_module_t *self = (dt_lib_module_t *)user_data;
    dt_lib_ratings_t *d = self->data;

    d->pointerx = x;
    d->pointery = y;
    gtk_widget_queue_draw(d->drawing);
}

static void _lib_ratings_button_press_callback(GtkGestureSingle *gesture, int n_press, double x,
                                               double y, gpointer user_data)
{
    (void)n_press;
    (void)x;
    (void)y;
    dt_gui_claim(gesture);
    dt_lib_module_t *self = (dt_lib_module_t *)user_data;
    dt_lib_ratings_t *d = self->data;
    if (d->current > 0)
    {
        GList *imgs = dt_act_on_get_images(FALSE, TRUE, FALSE);
        dt_ratings_apply_on_list(imgs, d->current, TRUE);
        dt_collection_update_query(darktable.collection, DT_COLLECTION_CHANGE_RELOAD,
                                   DT_COLLECTION_PROP_RATING_RANGE, imgs);

        dt_control_queue_redraw_center();
    }
}

static void _lib_ratings_leave_notify_callback(GtkEventControllerMotion *controller,
                                               gpointer user_data)
{
    (void)controller;
    dt_lib_module_t *self = (dt_lib_module_t *)user_data;
    dt_lib_ratings_t *d = self->data;
    d->pointery = d->pointerx = 0;
    gtk_widget_queue_draw(d->drawing);
}

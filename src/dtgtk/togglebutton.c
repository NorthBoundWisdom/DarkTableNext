/*
    This file is part of darktable,
    Copyright (C) 2010-2021 darktable developers.

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
#include "togglebutton.h"
#include "bauhaus/bauhaus.h"
#include "button.h"
#include "gui/gtk.h"
#include <string.h>

G_DEFINE_TYPE(GtkDarktableToggleButton, dtgtk_togglebutton, GTK_TYPE_TOGGLE_BUTTON);

static void dtgtk_togglebutton_init(GtkDarktableToggleButton *slider)
{
}

#if GTK_CHECK_VERSION(4, 0, 0)
static void _togglebutton_snapshot(GtkWidget *widget, GtkSnapshot *snapshot)
{
    g_return_if_fail(widget != NULL);
    g_return_if_fail(DTGTK_IS_TOGGLEBUTTON(widget));

    GTK_WIDGET_CLASS(dtgtk_togglebutton_parent_class)->snapshot(widget, snapshot);

    GtkDarktableToggleButton *button = DTGTK_TOGGLEBUTTON(widget);
    if (!button->icon || !button->canvas)
        return;

    graphene_rect_t bounds;
    if (!gtk_widget_compute_bounds(button->canvas, widget, &bounds))
        return;

    int flags = button->icon_flags;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        flags |= CPF_ACTIVE;
    else
        flags &= ~CPF_ACTIVE;

    if ((button->icon_data == dt_dev_gui_module()) && dt_dev_gui_module())
        flags |= CPF_FOCUS;
    else
        flags &= ~CPF_FOCUS;

    if (gtk_widget_get_state_flags(widget) & GTK_STATE_FLAG_PRELIGHT)
        flags |= CPF_PRELIGHT;
    else
        flags &= ~CPF_PRELIGHT;

    if (bounds.size.width <= 0 || bounds.size.height <= 0)
        return;

    GdkRGBA fg_color;
    dt_gui_widget_get_color(widget, &fg_color);
    cairo_t *cr = gtk_snapshot_append_cairo(snapshot, &bounds);
    gdk_cairo_set_source_rgba(cr, &fg_color);
    button->icon(cr, bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height,
                 flags, button->icon_data);
    cairo_destroy(cr);
}
#else
static gboolean _togglebutton_draw(GtkWidget *widget, cairo_t *cr);

static gboolean _togglebutton_draw(GtkWidget *widget, cairo_t *cr)
{
    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(DTGTK_IS_TOGGLEBUTTON(widget), FALSE);

    GtkStateFlags state = gtk_widget_get_state_flags(widget);

    GdkRGBA fg_color;
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    dt_gui_widget_get_color(widget, &fg_color);

    /* fetch flags */
    int flags = DTGTK_TOGGLEBUTTON(widget)->icon_flags;

    /* update active state paint flag */
    const gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (active)
        flags |= CPF_ACTIVE;
    else
        flags &= ~CPF_ACTIVE;

    /* update focus state paint flag */
    const gboolean hasfocus =
        ((DTGTK_TOGGLEBUTTON(widget)->icon_data == dt_dev_gui_module()) && dt_dev_gui_module());
    if (hasfocus)
        flags |= CPF_FOCUS;
    else
        flags &= ~CPF_FOCUS;

    /* prelight */
    if (state & GTK_STATE_FLAG_PRELIGHT)
        flags |= CPF_PRELIGHT;
    else
        flags &= ~CPF_PRELIGHT;

    /* begin cairo drawing */
    /* get button total allocation */
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    const int width = allocation.width;
    const int height = allocation.height;

    /* get the css geometry properties of the button */
    GtkBorder margin, border, padding;
    gtk_style_context_get_margin(context, state, &margin);
    gtk_style_context_get_border(context, state, &border);
    gtk_style_context_get_padding(context, state, &padding);

    /* for button frame and background, we remove css margin from allocation */
    int startx = margin.left;
    int starty = margin.top;
    int cwidth = width - margin.left - margin.right;
    int cheight = height - margin.top - margin.bottom;

    /* draw standard button background and borders */
    gtk_render_background(context, cr, startx, starty, cwidth, cheight);
    gtk_render_frame(context, cr, startx, starty, cwidth, cheight);

    gdk_cairo_set_source_rgba(cr, &fg_color);

    /* draw icon */
    if (DTGTK_TOGGLEBUTTON(widget)->icon)
    {
        /* calculate the button content allocation */
        startx += border.left + padding.left;
        starty += border.top + padding.top;
        cwidth -= border.left + border.right + padding.left + padding.right;
        cheight -= border.top + border.bottom + padding.top + padding.bottom;

        /* we have to leave some breathing room to the cairo icon paint function to possibly    */
        /* draw slightly outside the bounding box, for optical alignment and balancing of icons */
        /* we do this by putting a drawing area widget inside the button and using the CSS      */
        /* margin property in px of the drawing area as extra room in percent (DPI safe)        */
        /* we do this because Gtk+ does not support CSS size in percent                         */
        /* this extra margin can be also (slightly) negative                                    */
        GtkStyleContext *ccontext =
            gtk_widget_get_style_context(DTGTK_TOGGLEBUTTON(widget)->canvas);
        GtkBorder cmargin;
        gtk_style_context_get_margin(ccontext, state, &cmargin);

        startx += round(cmargin.left * cwidth / 100.0f);
        starty += round(cmargin.top * cheight / 100.0f);
        cwidth = round((float)cwidth * (1.0 - (cmargin.left + cmargin.right) / 100.0f));
        cheight = round((float)cheight * (1.0 - (cmargin.top + cmargin.bottom) / 100.0f));

        void *icon_data = DTGTK_TOGGLEBUTTON(widget)->icon_data;

        if (cwidth > 0 && cheight > 0)
            DTGTK_TOGGLEBUTTON(widget)->icon(cr, startx, starty, cwidth, cheight, flags, icon_data);
    }

    return FALSE;
}
#endif

static void dtgtk_togglebutton_class_init(GtkDarktableToggleButtonClass *klass)
{
    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;

#if GTK_CHECK_VERSION(4, 0, 0)
    widget_class->snapshot = _togglebutton_snapshot;
#else
    widget_class->draw = _togglebutton_draw;
#endif
}

// Public functions
GtkWidget *dtgtk_togglebutton_new(DTGTKCairoPaintIconFunc paint, gint paintflags, void *paintdata)
{
    GtkDarktableToggleButton *button;
    button = g_object_new(dtgtk_togglebutton_get_type(), NULL);
    button->icon = paint;
    button->icon_flags = paintflags;
    button->icon_data = paintdata;
    button->canvas = gtk_drawing_area_new();
    dt_gui_button_set_child(GTK_BUTTON(button), button->canvas);
    dt_gui_add_class(GTK_WIDGET(button), "dt_module_btn");
    gtk_widget_set_name(GTK_WIDGET(button->canvas), "button-canvas");
    g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(gtk_widget_queue_draw), NULL);
    return (GtkWidget *)button;
}

void dtgtk_togglebutton_set_paint(GtkDarktableToggleButton *button, DTGTKCairoPaintIconFunc paint,
                                  gint paintflags, void *paintdata)
{
    g_return_if_fail(button != NULL);
    button->icon = paint;
    button->icon_flags = paintflags;
    button->icon_data = paintdata;
}

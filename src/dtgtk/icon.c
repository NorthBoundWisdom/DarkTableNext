/*
    This file is part of darktable,
    Copyright (C) 2011-2020 darktable developers.

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
#include "icon.h"
#include "gui/gtk.h"
#include <string.h>

G_DEFINE_TYPE(GtkDarktableIcon, dtgtk_icon, GTK_TYPE_DRAWING_AREA);

static void _icon_draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);

static void dtgtk_icon_class_init(GtkDarktableIconClass *klass)
{
}

static void dtgtk_icon_init(GtkDarktableIcon *icon)
{
    dt_gui_drawing_area_set_draw_func(GTK_DRAWING_AREA(icon), _icon_draw, icon, NULL);
}

static void _icon_draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data)
{
    GtkWidget *widget = GTK_WIDGET(area);
    g_return_if_fail(widget != NULL);
    g_return_if_fail(DTGTK_IS_ICON(widget));

    GdkRGBA fg_color;
    dt_gui_widget_get_color(widget, &fg_color);

    gdk_cairo_set_source_rgba(cr, &fg_color);

    /* draw icon */
    if (DTGTK_ICON(widget)->icon)
        DTGTK_ICON(widget)->icon(cr, 0, 0, width, height,
                                 DTGTK_ICON(widget)->icon_flags, DTGTK_ICON(widget)->icon_data);
    (void)user_data;
}

// Public functions
GtkWidget *dtgtk_icon_new(DTGTKCairoPaintIconFunc paint, gint paintflags, void *paintdata)
{
    GtkDarktableIcon *icon;
    icon = g_object_new(dtgtk_icon_get_type(), NULL);
    icon->icon = paint;
    icon->icon_flags = paintflags;
    icon->icon_data = paintdata;
    gtk_widget_set_name(GTK_WIDGET(icon), "dt-icon");
    return (GtkWidget *)icon;
}

void dtgtk_icon_set_paint(GtkWidget *icon, DTGTKCairoPaintIconFunc paint, gint paintflags,
                          void *paintdata)
{
    g_return_if_fail(icon != NULL);
    DTGTK_ICON(icon)->icon = paint;
    DTGTK_ICON(icon)->icon_flags = paintflags;
    DTGTK_ICON(icon)->icon_data = paintdata;
    gtk_widget_queue_draw(icon);
}

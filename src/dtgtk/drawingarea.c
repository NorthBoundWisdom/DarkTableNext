/*
    This file is part of darktable,
    Copyright (C) 2014-2020 darktable developers.

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

#include "dtgtk/drawingarea.h"

G_DEFINE_TYPE(GtkDarktableDrawingArea, dtgtk_drawing_area, GTK_TYPE_DRAWING_AREA);

static GtkSizeRequestMode dtgtk_drawing_area_get_request_mode(GtkWidget *widget)
{
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
};

#if GTK_CHECK_VERSION(4, 0, 0)
static void dtgtk_drawing_area_measure(GtkWidget *widget, GtkOrientation orientation, int for_size,
                                       int *minimum, int *natural, int *minimum_baseline,
                                       int *natural_baseline)
{
    if (orientation != GTK_ORIENTATION_VERTICAL)
    {
        GTK_WIDGET_CLASS(dtgtk_drawing_area_parent_class)
            ->measure(widget, orientation, for_size, minimum, natural, minimum_baseline,
                      natural_baseline);
        return;
    }

    GtkDarktableDrawingArea *da = DTGTK_DRAWING_AREA(widget);
    const int width = MAX(for_size, 0);
    const int height = da->height == 0 ? width :
                       da->height == -1 ? width * da->aspect :
                                          da->height;
    *minimum = *natural = height;
    *minimum_baseline = *natural_baseline = -1;
}
#else
static void dtgtk_drawing_area_get_preferred_height_for_width(GtkWidget *widget, gint for_width,
                                                              gint *min_height, gint *nat_height)
{
    GtkDarktableDrawingArea *da = DTGTK_DRAWING_AREA(widget);

    if (da->height == 0)
    {
        // initialize with height = width
        *min_height = *nat_height = for_width;
    }
    else if (da->height == -1)
    {
        // initialize with aspect ratio
        *min_height = *nat_height = for_width * da->aspect;
    }
    else
    {
        *min_height = *nat_height = da->height;
    }
}
#endif

static void dtgtk_drawing_area_class_init(GtkDarktableDrawingAreaClass *class)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    widget_class->get_request_mode = dtgtk_drawing_area_get_request_mode;
#if GTK_CHECK_VERSION(4, 0, 0)
    widget_class->measure = dtgtk_drawing_area_measure;
#else
    widget_class->get_preferred_height_for_width =
        dtgtk_drawing_area_get_preferred_height_for_width;
#endif
}

static void dtgtk_drawing_area_init(GtkDarktableDrawingArea *da)
{
}

// public functions
GtkWidget *dtgtk_drawing_area_new_with_aspect_ratio(double aspect)
{
    GtkDarktableDrawingArea *da;
    da = g_object_new(dtgtk_drawing_area_get_type(), NULL);
    da->aspect = aspect;
    da->height = -1;

    return (GtkWidget *)da;
}

GtkWidget *dtgtk_drawing_area_new_with_height(int height)
{
    GtkDarktableDrawingArea *da;
    da = g_object_new(dtgtk_drawing_area_get_type(), NULL);
    da->aspect = 1.0f; // not used
    da->height = height;

    return (GtkWidget *)da;
}

void dtgtk_drawing_area_set_aspect_ratio(GtkWidget *widget, double aspect)
{
    GtkDarktableDrawingArea *da = DTGTK_DRAWING_AREA(widget);
    da->aspect = aspect;
    da->height = -1;
    gtk_widget_queue_resize(widget);
}

void dtgtk_drawing_area_set_height(GtkWidget *widget, int height)
{
    GtkDarktableDrawingArea *da = DTGTK_DRAWING_AREA(widget);
    da->aspect = 1.0f; // not used
    da->height = height < 0 ? 0 : height;
    gtk_widget_queue_resize(widget);
}

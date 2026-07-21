/*
    This file is part of darktable,
    Copyright (C) 2009-2026 darktable developers.

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

#pragma once

#include "common/atomic.h"
#include "common/darktable.h"
#include "common/dtpthread.h"

#include <gtk/gtk.h>
#include <stdint.h>

G_BEGIN_DECLS

#define DT_GUI_THUMBSIZE_REDUCE 0.7f

/* helper macro that applies the DPI transformation to fixed pixel
 * values. input should be defaulting to 96 DPI */
#define DT_PIXEL_APPLY_DPI(value) ((value) * darktable.gui->dpi_factor)

#define DT_RESIZE_HANDLE_SIZE DT_PIXEL_APPLY_DPI(5)

typedef struct dt_gui_widgets_t
{
    // Borders
    GtkWidget *left_border;
    GtkWidget *right_border;
    GtkWidget *left_border_overlay;
    GtkWidget *right_border_overlay;
    GtkWidget *bottom_border;
    GtkWidget *top_border;

    /* resize of left/right panels */
    gboolean panel_handle_dragging;
    int panel_handle_x, panel_handle_y;
} dt_gui_widgets_t;

typedef struct dt_gui_scrollbars_t
{
    GtkWidget *vscrollbar;
    GtkWidget *hscrollbar;

    gboolean visible;
} dt_gui_scrollbars_t;

typedef enum dt_gui_color_t
{
    DT_GUI_COLOR_BG = 0,
    DT_GUI_COLOR_DARKROOM_BG,
    DT_GUI_COLOR_DARKROOM_PREVIEW_BG,
    DT_GUI_COLOR_LIGHTTABLE_BG,
    DT_GUI_COLOR_LIGHTTABLE_PREVIEW_BG,
    DT_GUI_COLOR_LIGHTTABLE_FONT,
    DT_GUI_COLOR_PRINT_BG,
    DT_GUI_COLOR_BRUSH_CURSOR,
    DT_GUI_COLOR_BRUSH_TRACE,
    DT_GUI_COLOR_BUTTON_FG,
    DT_GUI_COLOR_THUMBNAIL_BG,
    DT_GUI_COLOR_THUMBNAIL_SELECTED_BG,
    DT_GUI_COLOR_THUMBNAIL_HOVER_BG,
    DT_GUI_COLOR_THUMBNAIL_OUTLINE,
    DT_GUI_COLOR_THUMBNAIL_SELECTED_OUTLINE,
    DT_GUI_COLOR_THUMBNAIL_HOVER_OUTLINE,
    DT_GUI_COLOR_THUMBNAIL_FONT,
    DT_GUI_COLOR_THUMBNAIL_SELECTED_FONT,
    DT_GUI_COLOR_THUMBNAIL_HOVER_FONT,
    DT_GUI_COLOR_THUMBNAIL_BORDER,
    DT_GUI_COLOR_THUMBNAIL_SELECTED_BORDER,
    DT_GUI_COLOR_FILMSTRIP_BG,
    DT_GUI_COLOR_CULLING_SELECTED_BORDER,
    DT_GUI_COLOR_CULLING_FILMSTRIP_SELECTED_BORDER,
    DT_GUI_COLOR_PREVIEW_HOVER_BORDER,
    DT_GUI_COLOR_LOG_BG,
    DT_GUI_COLOR_LOG_FG,
    DT_GUI_COLOR_MAP_COUNT_SAME_LOC,
    DT_GUI_COLOR_MAP_COUNT_DIFF_LOC,
    DT_GUI_COLOR_MAP_COUNT_BG,
    DT_GUI_COLOR_MAP_LOC_SHAPE_HIGH,
    DT_GUI_COLOR_MAP_LOC_SHAPE_LOW,
    DT_GUI_COLOR_MAP_LOC_SHAPE_DEF,
    DT_GUI_COLOR_COLOR_ASSESSMENT_BG,
    DT_GUI_COLOR_COLOR_ASSESSMENT_FG,
    DT_GUI_COLOR_LAST
} dt_gui_color_t;

typedef enum dt_gui_session_type_t
{
    DT_GUI_SESSION_UNKNOWN,
    DT_GUI_SESSION_X11,
    DT_GUI_SESSION_QUARTZ,
    DT_GUI_SESSION_WAYLAND,
} dt_gui_session_type_t;

typedef struct dt_gui_gtk_t
{
    struct dt_ui_t *ui;

    dt_gui_widgets_t widgets;

    dt_gui_scrollbars_t scrollbars;

    cairo_surface_t *surface; // cached prior image when config var ui/loading_screen is FALSE
    gboolean drawing_snapshot;

    char *last_preset;

    dt_atomic_int reset;
    GdkRGBA colors[DT_GUI_COLOR_LAST];

    int32_t hide_tooltips;

    gboolean grouping;
    dt_imgid_t expanded_group_id;

    gboolean show_overlays;
    gboolean show_focus_peaking;
    gboolean touchpad_gestures_enabled;
    double overlay_red, overlay_blue, overlay_green, overlay_contrast;

    double dpi, dpi_factor, ppd, ppd_thb;
    gboolean have_pen_pressure;

    int icon_size; // size of top panel icons

    // store which gtkrc we loaded:
    char gtkrc[PATH_MAX];

    gint scroll_mask;

    cairo_filter_t filter_image; // filtering used to scale images to screen
} dt_gui_gtk_t;

typedef struct _gui_collapsible_section_t
{
    GtkBox *parent;             // the parent widget
    gchar *confname;            // configuration name for the toggle status
    GtkWidget *toggle;          // toggle button
    GtkWidget *expander;        // the expanded
    GtkWidget *label;           // the label containing the section's title text
    GtkBox *container;          // the container for all widgets into the section
    struct dt_action_t *module; // the lib or iop module that contains this section
} dt_gui_collapsible_section_t;

static inline cairo_surface_t *dt_cairo_image_surface_create(cairo_format_t format, int width,
                                                             int height)
{
    cairo_surface_t *cst =
        cairo_image_surface_create(format, width * darktable.gui->ppd, height * darktable.gui->ppd);
    cairo_surface_set_device_scale(cst, darktable.gui->ppd, darktable.gui->ppd);
    return cst;
}

static inline cairo_surface_t *dt_cairo_image_surface_create_for_data(unsigned char *data,
                                                                      cairo_format_t format,
                                                                      int width, int height,
                                                                      int stride)
{
    cairo_surface_t *cst = cairo_image_surface_create_for_data(data, format, width, height, stride);
    cairo_surface_set_device_scale(cst, darktable.gui->ppd, darktable.gui->ppd);
    return cst;
}

static inline cairo_surface_t *dt_cairo_image_surface_create_from_png(const char *filename)
{
    cairo_surface_t *cst = cairo_image_surface_create_from_png(filename);
    cairo_surface_set_device_scale(cst, darktable.gui->ppd, darktable.gui->ppd);
    return cst;
}

static inline int dt_cairo_image_surface_get_width(cairo_surface_t *surface)
{
    return cairo_image_surface_get_width(surface) / darktable.gui->ppd;
}

static inline int dt_cairo_image_surface_get_height(cairo_surface_t *surface)
{
    return cairo_image_surface_get_height(surface) / darktable.gui->ppd;
}

static inline GdkPixbuf *dt_gdk_pixbuf_new_from_file_at_size(const char *filename, int width,
                                                             int height, GError **error)
{
    return gdk_pixbuf_new_from_file_at_size(filename, width * darktable.gui->ppd,
                                            height * darktable.gui->ppd, error);
}

// call class function to add or remove CSS classes (need to be set on top of this file as first function is used in this file)
void dt_gui_add_class(GtkWidget *widget, const gchar *class_name);
void dt_gui_remove_class(GtkWidget *widget, const gchar *class_name);
void dt_gui_widget_get_color(GtkWidget *widget, GdkRGBA *color);

typedef void (*dt_gui_drawing_area_draw_func_t)(GtkDrawingArea *, cairo_t *, int, int, gpointer);
void dt_gui_drawing_area_set_draw_func(GtkDrawingArea *area, dt_gui_drawing_area_draw_func_t draw,
                                       gpointer data, GDestroyNotify destroy);

void dt_open_url(const char *url);
int dt_gui_theme_init(dt_gui_gtk_t *gui);
int dt_gui_gtk_init(dt_gui_gtk_t *gui);
void dt_gui_gtk_run(dt_gui_gtk_t *gui);
void dt_gui_gtk_cleanup(dt_gui_gtk_t *gui);
void dt_gui_gtk_quit();
void dt_gui_store_last_preset(const char *name);
int dt_gui_gtk_load_config();
int dt_gui_gtk_write_config();
void dt_gui_gtk_set_source_rgb(cairo_t *cr, dt_gui_color_t color);
void dt_gui_gtk_set_source_rgba(cairo_t *cr, dt_gui_color_t color, const float opacity_coef);
double dt_get_system_gui_ppd(GtkWidget *widget);
double dt_get_screen_resolution(GtkWidget *widget);

/*
 * new ui api
 */

typedef enum dt_ui_container_t
{
    /* the top container of left panel, the top container
     disables the module expander and does not scroll with other modules
  */
    DT_UI_CONTAINER_PANEL_LEFT_TOP = 0,

    /* the center container of left panel, the center container
     contains the scrollable area that all plugins are placed within and last
     widget is the end marker.
     This container will always expand|fill empty vertical space
  */
    DT_UI_CONTAINER_PANEL_LEFT_CENTER = 1,

    /* the bottom container of left panel, this container works just like
     the top container but will be attached to bottom in the panel, such as
     plugins like background jobs module in lighttable and the plugin selection
     module in darkroom,
  */
    DT_UI_CONTAINER_PANEL_LEFT_BOTTOM = 2,

    DT_UI_CONTAINER_PANEL_RIGHT_TOP = 3,
    DT_UI_CONTAINER_PANEL_RIGHT_CENTER = 4,
    DT_UI_CONTAINER_PANEL_RIGHT_BOTTOM = 5,

    /* the top header bar, left slot where darktable name is placed */
    DT_UI_CONTAINER_PANEL_TOP_LEFT = 6,
    /* center which is expanded as wide it can */
    DT_UI_CONTAINER_PANEL_TOP_CENTER = 7,
    /* right side were the different views are accessed */
    DT_UI_CONTAINER_PANEL_TOP_RIGHT = 8,

    DT_UI_CONTAINER_PANEL_CENTER_TOP_LEFT = 9,
    DT_UI_CONTAINER_PANEL_CENTER_TOP_CENTER = 10,
    DT_UI_CONTAINER_PANEL_CENTER_TOP_RIGHT = 11,

    DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_LEFT = 12,
    DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_CENTER = 13,
    DT_UI_CONTAINER_PANEL_CENTER_BOTTOM_RIGHT = 14,

    /* this panel is placed at bottom of ui
     only used by the filmstrip if shown */
    DT_UI_CONTAINER_PANEL_BOTTOM = 15,

    /* Count of containers */
    DT_UI_CONTAINER_SIZE
} dt_ui_container_t;

typedef enum dt_ui_panel_t
{
    /* the header panel */
    DT_UI_PANEL_TOP,
    /* center top toolbar panel */
    DT_UI_PANEL_CENTER_TOP,
    /* center bottom toolbar panel */
    DT_UI_PANEL_CENTER_BOTTOM,
    /* left panel */
    DT_UI_PANEL_LEFT,
    /* right panel */
    DT_UI_PANEL_RIGHT,
    /* bottom panel */
    DT_UI_PANEL_BOTTOM,

    DT_UI_PANEL_SIZE
} dt_ui_panel_t;

typedef enum dt_ui_border_t
{
    DT_UI_BORDER_TOP,
    DT_UI_BORDER_BOTTOM,
    DT_UI_BORDER_LEFT,
    DT_UI_BORDER_RIGHT,

    DT_UI_BORDER_SIZE
} dt_ui_border_t;

typedef void (*dt_gui_widget_callback_t)(GtkWidget *widget, gpointer user_data);

/** \brief swap the container in the left and right panels */
void dt_ui_container_swap_left_right(struct dt_ui_t *ui, gboolean swap);
/** \brief add's a widget to a defined container */
void dt_ui_container_add_widget(const struct dt_ui_t *ui, const dt_ui_container_t c, GtkWidget *w);
/** \brief gives a widget focus in the container */
void dt_ui_container_focus_widget(const struct dt_ui_t *ui, const dt_ui_container_t c,
                                  GtkWidget *w);
/** \brief calls a callback on all children widgets from container */
void dt_ui_container_foreach(const struct dt_ui_t *ui, const dt_ui_container_t c,
                             dt_gui_widget_callback_t callback);
/** \brief destroy all child widgets from container */
void dt_ui_container_destroy_children(const struct dt_ui_t *ui, const dt_ui_container_t c);
/** \brief shows/hide a panel */
void dt_ui_panel_show(const struct dt_ui_t *ui, const dt_ui_panel_t, const gboolean show,
                      const gboolean write);
/** \brief restore saved state of panel visibility for current view */
void dt_ui_restore_panels(const struct dt_ui_t *ui);
/** \brief update scrollbars for current view */
void dt_ui_update_scrollbars(struct dt_ui_t *ui);
/** show or hide scrollbars */
void dt_ui_scrollbars_show(struct dt_ui_t *ui, const gboolean show);
/** \brief toggle view of panels eg. collapse/expands to previous view state */
void dt_ui_toggle_panels_visibility(const struct dt_ui_t *ui);
/** \brief draw user's attention */
void dt_ui_notify_user();
/** \brief get visible state of panel */
gboolean dt_ui_panel_visible(const struct dt_ui_t *ui, const dt_ui_panel_t);
/**  \brief get width of right, left, or bottom panel */
int dt_ui_panel_get_size(struct dt_ui_t *ui, const dt_ui_panel_t p);
/**  \brief set width of right, left, or bottom panel */
void dt_ui_panel_set_size(const struct dt_ui_t *ui, const dt_ui_panel_t p, int s);
/** \brief is the panel ancestor of widget */
gboolean dt_ui_panel_ancestor(const struct dt_ui_t *ui, const dt_ui_panel_t p, GtkWidget *w);
/** \brief get the center drawable widget */
GtkWidget *dt_ui_center(const struct dt_ui_t *ui);
GtkWidget *dt_ui_center_base(const struct dt_ui_t *ui);
GtkWidget *dt_ui_snapshot(const struct dt_ui_t *ui);
/** \brief get the main window widget */
GtkWidget *dt_ui_main_window(const struct dt_ui_t *ui);
/** \brief get the main thumb table */
struct dt_thumbtable_t *dt_ui_thumbtable(const struct dt_ui_t *ui);
/** \brief get the Lighttable-only filmstrip thumb table */
struct dt_thumbtable_t *dt_ui_lighttable_filmstrip(const struct dt_ui_t *ui);
/** \brief get the log message widget */
GtkWidget *dt_ui_log_msg(const struct dt_ui_t *ui);
/** \brief get the toast message widget */
GtkWidget *dt_ui_toast_msg(const struct dt_ui_t *ui);

GtkBox *dt_ui_get_container(const struct dt_ui_t *ui, const dt_ui_container_t c);

/*  activate ellipsization of the combox entries */
void dt_ellipsize_combo(GtkComboBox *cbox);

static inline void dt_ui_section_label_set(GtkWidget *label)
{
    gtk_widget_set_halign(label, GTK_ALIGN_FILL); // make it span the whole available width
    gtk_label_set_xalign(GTK_LABEL(label), 0.5f);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END); // ellipsize labels
    dt_gui_add_class(label, "dt_section_label"); // make sure that we can style these easily
}

static inline GtkWidget *dt_ui_section_label_new(const gchar *str)
{
    GtkWidget *label = gtk_label_new(str);
    dt_ui_section_label_set(label);
    return label;
};

static inline GtkWidget *dt_ui_label_new(const gchar *str)
{
    GtkWidget *label = gtk_label_new(str);
    g_object_set(label, "halign", GTK_ALIGN_START, "xalign", 0.0f, "ellipsize", PANGO_ELLIPSIZE_END,
                 (void *)0);
    return label;
};

static inline void dt_gui_editable_set_width_chars(GtkEditable *editable, gint width_chars)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_editable_set_width_chars(editable, width_chars);
#else
    gtk_entry_set_width_chars(GTK_ENTRY(editable), width_chars);
#endif
}

static inline void dt_gui_editable_set_max_width_chars(GtkEditable *editable, gint width_chars)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_editable_set_max_width_chars(editable, width_chars);
#else
    gtk_entry_set_max_width_chars(GTK_ENTRY(editable), width_chars);
#endif
}

static inline GtkWidget *dt_ui_entry_new(gint width_chars)
{
    GtkWidget *entry = gtk_entry_new();
    gtk_drag_dest_unset(entry);
    dt_gui_editable_set_width_chars(GTK_EDITABLE(entry), width_chars);
    return entry;
};

extern DT_CORE_API const struct dt_action_def_t dt_action_def_tabs_all_rgb;
extern DT_CORE_API const struct dt_action_def_t dt_action_def_tabs_rgb;
extern DT_CORE_API const struct dt_action_def_t dt_action_def_tabs_none;

GtkNotebook *dt_ui_notebook_new(struct dt_action_def_t *def);
void dt_ui_notebook_scroll(GtkNotebook *notebook, int delta);

GtkWidget *dt_ui_notebook_page(GtkNotebook *notebook, const char *text, const char *tooltip);

// show a dialog box with 2 buttons in case some user interaction is
// required BEFORE dt's gui is initialised.  this expects gtk_init()
// to be called already which should be the case during most of dt's
// init phase.
gboolean dt_gui_show_standalone_yes_no_dialog(const char *title, const char *markup,
                                              const char *no_text, const char *yes_text);

// similar to the one above. this one asks the user for some
// string. the hint is shown in the empty entry box
char *dt_gui_show_standalone_string_dialog(const char *title, const char *markup,
                                           const char *placeholder, const char *no_text,
                                           const char *yes_text);

// returns TRUE if YES was answered, FALSE otherwise
gboolean dt_gui_show_yes_no_dialog(const char *title, const char *wname, const char *format, ...);

void dt_gui_add_help_link(GtkWidget *widget, const char *link);
char *dt_gui_get_help_url(GtkWidget *widget);
void dt_gui_dialog_add_help(GtkDialog *dialog, const char *topic);
void dt_gui_show_help(GtkWidget *widget);

// load a CSS theme
void dt_gui_load_theme(const char *theme); // read them and add user tweaks
void dt_gui_apply_theme();                 // apply the loaded theme to darktable's windows

// reload GUI scalings
void dt_configure_ppd_dpi(dt_gui_gtk_t *gui);

// return modifier keys currently pressed, independent of any key event
GdkModifierType dt_key_modifier_state();

GtkWidget *dt_ui_resize_wrap(GtkWidget *w, const gint min_size, char *config_str);

// Check whether the given widget has any direct children.
gboolean dt_gui_container_has_children(GtkWidget *widget);
// Return a count of the direct children of the given widget.
int dt_gui_container_num_children(GtkWidget *widget);
// Return the first direct child of the given widget.
GtkWidget *dt_gui_container_first_child(GtkWidget *widget);
// Return the requested direct child of the given widget, or NULL if it has fewer children.
GtkWidget *dt_gui_container_nth_child(GtkWidget *widget, const int which);
// Set the focusability of direct GtkFlowBox child wrappers.
void dt_gui_flow_box_set_children_can_focus(GtkFlowBox *flow_box, gboolean can_focus);

// remove all of the children we've added to the container.  Any which
// no longer have any references will be destroyed.
void dt_gui_container_remove_children(GtkWidget *container);

// delete all of the children we've added to the container.  Use this
// function only if you are SURE there are no other references to any
// of the children (if in doubt, use dt_gui_container_remove_children
// instead; it's a bit slower but safer).
void dt_gui_container_destroy_children(GtkWidget *container);

void dt_gui_menu_popup(GtkMenu *menu, GtkWidget *button, GdkGravity widget_anchor,
                       GdkGravity menu_anchor);

void dt_gui_draw_rounded_rectangle(cairo_t *cr, const float width, const float height,
                                   const float x, const float y);

void dt_gui_widget_reallocate_now(GtkWidget *widget);

#if !GTK_CHECK_VERSION(4, 0, 0)
// GTK 3 event handler for "key-press-event" of GtkTreeView to decide if
// focus switches to GtkSearchEntry
gboolean dt_gui_search_start(GtkWidget *widget, GdkEventKey *event, GtkSearchEntry *entry);
#endif

// event handler for "stop-search" of GtkSearchEntry
void dt_gui_search_stop(GtkSearchEntry *entry, GtkWidget *widget);

// create a collapsible section, insert in parent, return the container
void dt_gui_new_collapsible_section(dt_gui_collapsible_section_t *cs, const char *confname,
                                    const char *label, GtkBox *parent, struct dt_action_t *module);
// update the collapsible section's label text
void dt_gui_collapsible_section_set_label(dt_gui_collapsible_section_t *cs, const char *label);
// routine to be called from gui_update
void dt_gui_update_collapsible_section(const dt_gui_collapsible_section_t *cs);

// routine to hide the collapsible section
void dt_gui_hide_collapsible_section(const dt_gui_collapsible_section_t *cs);

// is delay between first and second click/press longer than double-click time?
gboolean dt_gui_long_click(const guint second, const guint first);
// Snapshot the current controller callback's event metadata. Call only from a controller or
// gesture callback; GTK 3 obtains and immediately releases its temporary GdkEvent copy.
guint32 dt_gui_controller_get_current_event_time(GtkEventController *controller);
GdkModifierType dt_gui_controller_get_current_event_state(GtkEventController *controller);
typedef struct dt_gui_controller_scroll_event_t
{
    GdkDevice *device; // borrowed and only valid for the duration of the callback
    GdkScrollDirection direction;
    GdkModifierType state;
    double x, y;
    double delta_x, delta_y;
    gboolean is_stop;
    gboolean pointer_emulated;
} dt_gui_controller_scroll_event_t;
gboolean dt_gui_controller_get_current_scroll_event(
    GtkEventController *controller, dt_gui_controller_scroll_event_t *event);
gboolean dt_gui_scroll_event_get_deltas(const dt_gui_controller_scroll_event_t *event,
                                        gdouble *delta_x, gdouble *delta_y);
gboolean dt_gui_scroll_event_get_unit_deltas(const dt_gui_controller_scroll_event_t *event,
                                             int *delta_x, int *delta_y);
gboolean dt_gui_scroll_event_get_unit_delta(const dt_gui_controller_scroll_event_t *event,
                                            int *delta);
typedef struct dt_gui_controller_crossing_event_t
{
    GdkCrossingMode mode;
    GdkNotifyType detail;
} dt_gui_controller_crossing_event_t;
gboolean dt_gui_controller_get_current_crossing_event(
    GtkEventController *controller, dt_gui_controller_crossing_event_t *event);

typedef void (*dt_gui_click_callback_t)(GtkGestureSingle *, int, double, double, gpointer);
typedef void (*dt_gui_drag_callback_t)(GtkGestureDrag *, double, double, gpointer);
typedef void (*dt_gui_motion_callback_t)(GtkEventControllerMotion *, double, double, gpointer);
typedef void (*dt_gui_motion_leave_callback_t)(GtkEventControllerMotion *, gpointer);
typedef gboolean (*dt_gui_key_callback_t)(GtkEventControllerKey *, guint, guint, GdkModifierType,
                                          gpointer);
typedef void (*dt_gui_scroll_callback_t)(GtkEventControllerScroll *, double, double, gpointer);
typedef gboolean (*dt_gui_scroll_handled_callback_t)(GtkEventControllerScroll *, double, double,
                                                     gpointer);

GtkGestureSingle *(dt_gui_connect_click)(GtkWidget *widget, dt_gui_click_callback_t pressed,
                                         dt_gui_click_callback_t released, gpointer data);
#define dt_gui_connect_click(widget, pressed, released, data)                                      \
    (dt_gui_connect_click)(GTK_WIDGET(widget), (pressed), (released), (data))
#define dt_gui_connect_click_all(widget, pressed, released, data)                                  \
    gtk_gesture_single_set_button(dt_gui_connect_click(widget, pressed, released, data), 0)

GtkGesture *(dt_gui_connect_drag)(GtkWidget *widget, dt_gui_drag_callback_t drag_begin,
                                  dt_gui_drag_callback_t drag_end,
                                  dt_gui_drag_callback_t drag_update, gpointer data);
#define dt_gui_connect_drag(widget, drag_begin, drag_end, drag_update, data)                       \
    (dt_gui_connect_drag)(GTK_WIDGET(widget), (drag_begin), (drag_end), (drag_update), (data))

GtkEventController *(dt_gui_connect_motion)(GtkWidget *widget, dt_gui_motion_callback_t motion,
                                            dt_gui_motion_callback_t enter,
                                            dt_gui_motion_leave_callback_t leave, gpointer data);
#define dt_gui_connect_motion(widget, motion, enter, leave, data)                                  \
    (dt_gui_connect_motion)(GTK_WIDGET(widget), (motion), (enter), (leave), (data))

GtkEventController *(dt_gui_connect_key)(GtkWidget *widget, dt_gui_key_callback_t key_pressed,
                                         gpointer data);
#define dt_gui_connect_key(widget, key_pressed, data)                                                \
    (dt_gui_connect_key)(GTK_WIDGET(widget), (key_pressed), (data))
GtkEventController *(dt_gui_connect_key_bubble)(GtkWidget *widget,
                                                dt_gui_key_callback_t key_pressed, gpointer data);
#define dt_gui_connect_key_bubble(widget, key_pressed, data)                                         \
    (dt_gui_connect_key_bubble)(GTK_WIDGET(widget), (key_pressed), (data))

GtkEventController *(dt_gui_connect_scroll)(GtkWidget *widget, GtkEventControllerScrollFlags flags,
                                            dt_gui_scroll_callback_t scroll, gpointer data);
#define dt_gui_connect_scroll(widget, flags, scroll, data)                                         \
    (dt_gui_connect_scroll)(GTK_WIDGET(widget), (flags), (scroll), (data))
GtkEventController *(dt_gui_connect_scroll_handled)(
    GtkWidget *widget, GtkEventControllerScrollFlags flags, dt_gui_scroll_handled_callback_t scroll,
    gpointer data);
#define dt_gui_connect_scroll_handled(widget, flags, scroll, data)                                 \
    (dt_gui_connect_scroll_handled)(GTK_WIDGET(widget), (flags), (scroll), (data))

#define dt_gui_claim(gesture)                                                                      \
    gtk_gesture_set_state(GTK_GESTURE(gesture), GTK_EVENT_SEQUENCE_CLAIMED)
#define dt_gui_deny(gesture) gtk_gesture_set_state(GTK_GESTURE(gesture), GTK_EVENT_SEQUENCE_DENIED)

#define dt_modifier_eq(controller, mask)                                                        \
    dt_modifier_is(dt_gui_controller_get_current_event_state(GTK_EVENT_CONTROLLER(controller)), \
                   (mask))

// control whether the mouse pointer displays as a "busy" cursor, e.g. watch or timer
// the calls may be nested, but must be matched
void dt_gui_cursor_set_busy();
void dt_gui_cursor_clear_busy();

// run all pending Gtk/GDK events
// should be called after making Gtk calls if we won't resume the main event loop for a while
// (i.e. the current function will do a lot of work before returning)
void dt_gui_process_events();

#ifdef __cplusplus
extern "C++"
{
    template <typename... Widgets>
    inline GtkWidget *dt_gui_box_add(gpointer box, Widgets *...w)
    {
        // Fold expression: add every child while preserving the hbox/vbox construction order.
#if GTK_CHECK_VERSION(4, 0, 0)
        (gtk_box_append(GTK_BOX(box), GTK_WIDGET(w)), ...);
#else
        (gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(w)), ...);
#endif
        return GTK_WIDGET(box);
    }
}
#else
GtkWidget *(dt_gui_box_add)(const char *file, const int line, const char *function, GtkBox *box,
                            gpointer list[]);
#define dt_gui_box_add(box, ...)                                                                   \
    dt_gui_box_add(__FILE__, __LINE__, __FUNCTION__, GTK_BOX(box),                                 \
                   (gpointer[]){__VA_ARGS__ __VA_OPT__(, )(gpointer) - 1})
#endif
#define dt_gui_hbox(...)                                                                           \
    dt_gui_box_add(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0) __VA_OPT__(, ) __VA_ARGS__)
#define dt_gui_vbox(...)                                                                           \
    dt_gui_box_add(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0) __VA_OPT__(, ) __VA_ARGS__)
#define dt_gui_dialog_add(dialog, ...)                                                             \
    dt_gui_box_add(gtk_dialog_get_content_area(GTK_DIALOG(dialog)), __VA_ARGS__)
#define dt_gui_expand(widget) dt_gui_expand(GTK_WIDGET(widget))
#define dt_gui_align_right(widget) dt_gui_align_right(GTK_WIDGET(widget))

static inline void dt_gui_flow_box_append(GtkFlowBox *flow_box, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_flow_box_append(flow_box, child);
#else
    gtk_container_add(GTK_CONTAINER(flow_box), child);
#endif
}

static inline void dt_gui_box_remove(GtkBox *box, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_box_remove(box, child);
#else
    gtk_container_remove(GTK_CONTAINER(box), child);
#endif
}

static inline GtkWidget *(dt_gui_expand)(GtkWidget * widget)
{
    gtk_widget_set_hexpand(widget, TRUE);
    return widget;
}

static inline GtkWidget *(dt_gui_align_right)(GtkWidget * widget)
{
    gtk_widget_set_halign(widget, GTK_ALIGN_END);
    return dt_gui_expand(widget);
}

static inline void dt_gui_window_set_child(GtkWindow *window, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_window_set_child(window, child);
#else
    gtk_container_add(GTK_CONTAINER(window), child);
#endif
}

static inline void dt_gui_window_remove_child(GtkWindow *window, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    g_return_if_fail(gtk_window_get_child(window) == child);
    gtk_window_set_child(window, NULL);
#else
    gtk_container_remove(GTK_CONTAINER(window), child);
#endif
}

static inline void dt_gui_overlay_set_child(GtkOverlay *overlay, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_overlay_set_child(overlay, child);
#else
    gtk_container_add(GTK_CONTAINER(overlay), child);
#endif
}

static inline GtkWidget *dt_gui_overlay_get_child(GtkOverlay *overlay)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_overlay_get_child(overlay);
#else
    return gtk_bin_get_child(GTK_BIN(overlay));
#endif
}

static inline void dt_gui_scrolled_window_set_child(GtkScrolledWindow *scrolled_window,
                                                    GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_scrolled_window_set_child(scrolled_window, child);
#else
    gtk_container_add(GTK_CONTAINER(scrolled_window), child);
#endif
}

static inline GtkWidget *dt_gui_scrolled_window_get_child(GtkScrolledWindow *scrolled_window)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_scrolled_window_get_child(scrolled_window);
#else
    return gtk_bin_get_child(GTK_BIN(scrolled_window));
#endif
}

static inline void dt_gui_viewport_set_child(GtkViewport *viewport, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_viewport_set_child(viewport, child);
#else
    gtk_container_add(GTK_CONTAINER(viewport), child);
#endif
}

static inline GtkWidget *dt_gui_viewport_get_child(GtkViewport *viewport)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_viewport_get_child(viewport);
#else
    return gtk_bin_get_child(GTK_BIN(viewport));
#endif
}

static inline void dt_gui_popover_set_child(GtkPopover *popover, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_popover_set_child(popover, child);
#else
    gtk_container_add(GTK_CONTAINER(popover), child);
#endif
}

static inline void dt_gui_stack_add_child(GtkStack *stack, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_stack_add_child(stack, child);
#else
    gtk_container_add(GTK_CONTAINER(stack), child);
#endif
}

static inline void dt_gui_stack_remove_child(GtkStack *stack, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_stack_remove(stack, child);
#else
    gtk_container_remove(GTK_CONTAINER(stack), child);
#endif
}

static inline void dt_gui_grid_remove_child(GtkGrid *grid, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_grid_remove(grid, child);
#else
    gtk_container_remove(GTK_CONTAINER(grid), child);
#endif
}

static inline void dt_gui_button_set_child(GtkButton *button, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_button_set_child(button, child);
#else
    gtk_container_add(GTK_CONTAINER(button), child);
#endif
}

static inline GtkWidget *dt_gui_button_get_child(GtkButton *button)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_button_get_child(button);
#else
    return gtk_bin_get_child(GTK_BIN(button));
#endif
}

static inline void dt_gui_check_button_set_child(GtkCheckButton *button, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_check_button_set_child(button, child);
#else
    gtk_container_add(GTK_CONTAINER(button), child);
#endif
}

static inline GtkWidget *dt_gui_check_button_get_child(GtkCheckButton *button)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_check_button_get_child(button);
#else
    return gtk_bin_get_child(GTK_BIN(button));
#endif
}

static inline void dt_gui_frame_set_child(GtkFrame *frame, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_frame_set_child(frame, child);
#else
    gtk_container_add(GTK_CONTAINER(frame), child);
#endif
}

static inline void dt_gui_revealer_set_child(GtkRevealer *revealer, GtkWidget *child)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_revealer_set_child(revealer, child);
#else
    gtk_container_add(GTK_CONTAINER(revealer), child);
#endif
}

static inline GtkWidget *dt_gui_revealer_get_child(GtkRevealer *revealer)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_revealer_get_child(revealer);
#else
    return gtk_bin_get_child(GTK_BIN(revealer));
#endif
}

static inline GtkWidget *dt_gui_scrolled_window_new(GtkAdjustment *hadjustment,
                                                     GtkAdjustment *vadjustment)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    if (hadjustment)
        gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(scrolled_window), hadjustment);
    if (vadjustment)
        gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window), vadjustment);
    return scrolled_window;
#else
    return gtk_scrolled_window_new(hadjustment, vadjustment);
#endif
}

static inline GtkWidget *dt_gui_toplevel_window_new(void)
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gtk_window_new();
#else
    return gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif
}

static inline GtkWidget *dt_gui_scroll_wrap(GtkWidget *widget)
{
    GtkWidget *scrolled_window = dt_gui_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    dt_gui_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), widget);
    return scrolled_window;
}

// Simulate a mouse button event (button is 1, 2, 3 - mouse button) sent to a Widget
void dt_gui_simulate_button_event(GtkWidget *widget, const GdkEventType eventtype,
                                  const int button);

// Setup auto-commit on focus loss for editable renderers
void dt_gui_commit_on_focus_loss(GtkCellRenderer *renderer, GtkCellEditable **active_editable);

// restore dialog size from config file
void dt_gui_dialog_restore_size(GtkDialog *dialog, const char *conf);

// returns the session type at runtime
dt_gui_session_type_t dt_gui_get_session_type(void);

#if !defined(__cplusplus) && !defined(_MSC_VER)
#undef G_CALLBACK
static inline GCallback G_CALLBACK(void *f)
{
    return (GCallback)f;
} // as a macro it gets expanded before reaching here
#define DISABLINGPREFIXG_CALLBACK
#define BOOLSIGNAL(s, signal) || !strcmp(s, #signal)
#if GTK_CHECK_VERSION(4, 0, 0)
#define BOOLSCROLLSIGNAL(s) BOOLSIGNAL(s, scroll)
#else
#define BOOLSCROLLSIGNAL(s)
#endif
#undef _Static_assert
#undef g_signal_connect
// clang-format off
#define g_signal_connect(instance, signal, c_handler, user_data) do { \
  _Static_assert(((strlen(signal)>4 && !strcmp("event", &signal[strlen(signal)-5])) \
    BOOLSIGNAL(signal, drag-motion) \
    BOOLSIGNAL(signal, drag-failed) \
    BOOLSIGNAL(signal, drag-drop) \
    BOOLSIGNAL(signal, key-pressed) \
    BOOLSCROLLSIGNAL(signal) \
    BOOLSIGNAL(signal, focus) \
    BOOLSIGNAL(signal, draw) \
    BOOLSIGNAL(signal, popup-menu) \
    BOOLSIGNAL(signal, query-tooltip) \
    BOOLSIGNAL(signal, match-selected) \
    BOOLSIGNAL(signal, get-child-position) \
    ) == _Generic((DISABLINGPREFIX##c_handler), gboolean(*)(): TRUE, default: FALSE), \
    "signal " signal " return type does not match specified handler " #c_handler); \
  g_signal_connect_data((instance), (signal), (GCallback)(c_handler), (user_data), NULL, (GConnectFlags) 0); } while(0)
// clang-format on
#endif // !__cplusplus && !_MSC_VER

G_END_DECLS

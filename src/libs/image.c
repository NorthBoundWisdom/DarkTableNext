/*
    This file is part of darktable,
    Copyright (C) 2010-2024 darktable developers.

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

#include "bauhaus/bauhaus.h"
#include "common/collection.h"
#include "common/darktable.h"
#include "common/debug.h"
#include "common/image_cache.h"
#include "common/ratings.h"
#include "common/colorlabels.h"
#include "common/undo.h"
#include "common/metadata.h"
#include "common/tags.h"
#include "control/control.h"
#include "control/jobs.h"
#include "control/jobs/control_jobs.h"
#include "dtgtk/button.h"
#include "dtgtk/paint.h"
#include "gui/accelerators.h"
#include "gui/gtk.h"
#include "libs/lib.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include "libs/lib_api.h"

DT_MODULE(1)

typedef struct dt_lib_image_t
{
    GtkWidget *rotate_cw_button, *rotate_ccw_button, *remove_button;
    GtkWidget *delete_button, *create_hdr_button;
    GtkWidget *duplicate_button, *reset_button, *move_button, *copy_button;
    GtkWidget *group_button, *ungroup_button, *cache_button, *uncache_button;
    GtkWidget *refresh_button, *set_monochrome_button, *set_color_button;
    GtkWidget *copy_metadata_button, *paste_metadata_button, *clear_metadata_button;
    GtkWidget *rating_flag, *colors_flag, *metadata_flag, *tags_flag;
    GtkWidget *page1; // retained for extension pages
} dt_lib_image_t;

const char *name(dt_lib_module_t *self)
{
    return _("actions on selection");
}

dt_view_type_flags_t views(dt_lib_module_t *self)
{
    return DT_VIEW_LIGHTTABLE;
}

uint32_t container(dt_lib_module_t *self)
{
    return DT_UI_CONTAINER_PANEL_RIGHT_CENTER;
}

static void _duplicate_virgin(dt_action_t *action)
{
    dt_control_duplicate_images(TRUE);
}

static void button_clicked(GtkWidget *widget, gpointer user_data)
{
    const int i = GPOINTER_TO_INT(user_data);
    if (i == 0)
        dt_control_remove_images();
    else if (i == 1)
        dt_control_delete_images();
    // else if(i == 2) dt_control_write_sidecar_files();
    else if (i == 3)
        dt_control_duplicate_images(FALSE);
    else if (i == 4)
        dt_control_flip_images(1);
    else if (i == 5)
        dt_control_flip_images(0);
    else if (i == 6)
        dt_control_flip_images(2);
    else if (i == 7)
        dt_control_merge_hdr();
    else if (i == 8)
        dt_control_move_images();
    else if (i == 9)
        dt_control_copy_images();
    else if (i == 10)
        dt_control_group_images();
    else if (i == 11)
        dt_control_ungroup_images();
    else if (i == 12)
        dt_control_set_local_copy_images();
    else if (i == 13)
        dt_control_reset_local_copy_images();
    else if (i == 14)
        dt_control_refresh_exif();
}

void gui_update(dt_lib_module_t *self)
{
    dt_lib_image_t *d = self->data;
    const int nbimgs = dt_act_on_get_images_nb(FALSE, FALSE);

    const gboolean act_on_any = (nbimgs > 0);
    const gboolean act_on_one = (nbimgs == 1);
    const gboolean act_on_mult = (nbimgs > 1);
    const uint32_t selected_cnt = dt_collection_get_selected_count();
    const gboolean can_paste = dt_control_can_paste_metadata();

    gtk_widget_set_sensitive(GTK_WIDGET(d->remove_button), act_on_any);
    gtk_widget_set_sensitive(GTK_WIDGET(d->delete_button), act_on_any);

    gtk_widget_set_sensitive(GTK_WIDGET(d->move_button), act_on_any);
    gtk_widget_set_sensitive(GTK_WIDGET(d->copy_button), act_on_any);

    gtk_widget_set_sensitive(GTK_WIDGET(d->create_hdr_button), act_on_any);
    gtk_widget_set_sensitive(GTK_WIDGET(d->duplicate_button), act_on_any);

    gtk_widget_set_sensitive(GTK_WIDGET(d->rotate_ccw_button), act_on_any);
    gtk_widget_set_sensitive(GTK_WIDGET(d->rotate_cw_button), act_on_any);
    gtk_widget_set_sensitive(GTK_WIDGET(d->reset_button), act_on_any);

    gtk_widget_set_sensitive(GTK_WIDGET(d->cache_button), act_on_any);
    gtk_widget_set_sensitive(GTK_WIDGET(d->uncache_button), act_on_any);

    gtk_widget_set_sensitive(GTK_WIDGET(d->group_button), selected_cnt > 1);

    gtk_widget_set_sensitive(GTK_WIDGET(d->copy_metadata_button), act_on_one);
    gtk_widget_set_sensitive(GTK_WIDGET(d->paste_metadata_button), can_paste);
    gtk_widget_set_sensitive(GTK_WIDGET(d->clear_metadata_button), act_on_any);

    gtk_widget_set_sensitive(GTK_WIDGET(d->refresh_button), act_on_any);
    if (act_on_mult)
    {
        gtk_widget_set_sensitive(GTK_WIDGET(d->ungroup_button), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(d->set_monochrome_button), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(d->set_color_button), TRUE);
    }
    else if (!act_on_any)
    {
        // no images to act on!
        gtk_widget_set_sensitive(GTK_WIDGET(d->ungroup_button), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(d->set_monochrome_button), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(d->set_color_button), FALSE);
    }
    else
    {
        // exact one image to act on
        const dt_imgid_t imgid = dt_act_on_get_main_image();
        if (dt_is_valid_imgid(imgid))
        {
            dt_image_t *img = dt_image_cache_get(imgid, 'r');
            const gboolean is_bw = (dt_image_monochrome_flags(img) != 0);
            const int img_group_id = img->group_id;
            dt_image_cache_read_release(img);
            gtk_widget_set_sensitive(GTK_WIDGET(d->set_monochrome_button), !is_bw);
            gtk_widget_set_sensitive(GTK_WIDGET(d->set_color_button), is_bw);
            sqlite3_stmt *stmt;
            DT_DEBUG_SQLITE3_PREPARE_V2(dt_database_get(darktable.db),
                                        "SELECT COUNT(id)"
                                        " FROM main.images"
                                        " WHERE group_id = ?1 AND id != ?2",
                                        -1, &stmt, NULL);
            DT_DEBUG_SQLITE3_BIND_INT(stmt, 1, img_group_id);
            DT_DEBUG_SQLITE3_BIND_INT(stmt, 2, imgid);
            if (stmt != NULL && sqlite3_step(stmt) == SQLITE_ROW)
            {
                const int images_in_grp = sqlite3_column_int(stmt, 0);
                gtk_widget_set_sensitive(GTK_WIDGET(d->ungroup_button), images_in_grp > 0);
            }
            else
                gtk_widget_set_sensitive(GTK_WIDGET(d->ungroup_button), FALSE);
            if (stmt)
                sqlite3_finalize(stmt);
        }
        else
        {
            gtk_widget_set_sensitive(GTK_WIDGET(d->set_monochrome_button), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(d->set_color_button), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(d->ungroup_button), FALSE);
        }
    }
}

static void _image_selection_changed_callback(gpointer instance, dt_lib_module_t *self)
{
    dt_lib_gui_queue_update(self);
}

static void _collection_updated_callback(gpointer instance, dt_collection_change_t query_change,
                                         dt_collection_properties_t changed_property, gpointer imgs,
                                         const int next, dt_lib_module_t *self)
{
    dt_lib_gui_queue_update(self);
}

static void _mouse_over_image_callback(gpointer instance, dt_lib_module_t *self)
{
    dt_lib_gui_queue_update(self);
}

static void _image_preference_changed(gpointer instance, dt_lib_module_t *self)
{
    dt_lib_image_t *d = self->data;
    gboolean trash = dt_conf_get_bool("send_to_trash");
    gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(d->delete_button))),
                       trash ? _("delete (trash)") : _("delete"));
    gtk_widget_set_tooltip_text(d->delete_button,
                                trash ? _("physically delete from disk (using trash if possible)") :
                                        _("physically delete from disk immediately"));
}

int position(const dt_lib_module_t *self)
{
    return 700;
}

static void _copy_metadata_callback(GtkWidget *widget, dt_lib_module_t *self)
{
    (void)widget;
    dt_control_copy_metadata_source();
    dt_lib_gui_queue_update(self);
}

static void _paste_metadata_callback(GtkWidget *widget, dt_lib_module_t *self)
{
    (void)widget;
    (void)self;
    dt_control_paste_metadata();
}

static void _clear_metadata_callback(GtkWidget *widget, dt_lib_module_t *self)
{
    (void)widget;
    (void)self;
    dt_control_clear_metadata();
}

static void _set_monochrome_callback(GtkWidget *widget, dt_lib_module_t *self)
{
    dt_control_monochrome_images(2);
}

static void _set_color_callback(GtkWidget *widget, dt_lib_module_t *self)
{
    dt_control_monochrome_images(0);
}

static void _rating_flag_callback(GtkWidget *widget, dt_lib_module_t *self)
{
    dt_lib_image_t *d = self->data;
    const gboolean flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(d->rating_flag));
    dt_conf_set_bool("plugins/lighttable/copy_metadata/rating", flag);
}

static void _colors_flag_callback(GtkWidget *widget, dt_lib_module_t *self)
{
    dt_lib_image_t *d = self->data;
    const gboolean flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(d->colors_flag));
    dt_conf_set_bool("plugins/lighttable/copy_metadata/colors", flag);
}

static void _metadata_flag_callback(GtkWidget *widget, dt_lib_module_t *self)
{
    dt_lib_image_t *d = self->data;
    const gboolean flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(d->metadata_flag));
    dt_conf_set_bool("plugins/lighttable/copy_metadata/metadata", flag);
}

static void _tags_flag_callback(GtkWidget *widget, dt_lib_module_t *self)
{
    dt_lib_image_t *d = self->data;
    const gboolean flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(d->tags_flag));
    dt_conf_set_bool("plugins/lighttable/copy_metadata/tags", flag);
}

static void _pastemode_combobox_changed(GtkWidget *widget, gpointer user_data)
{
    const int mode = dt_bauhaus_combobox_get(widget);
    dt_conf_set_int("plugins/lighttable/copy_metadata/pastemode", mode);
}

#define ellipsize_button(button)                                                                   \
    gtk_label_set_ellipsize(GTK_LABEL(gtk_bin_get_child(GTK_BIN(button))), PANGO_ELLIPSIZE_END);

void gui_init(dt_lib_module_t *self)
{
    dt_lib_image_t *d = malloc(sizeof(dt_lib_image_t));
    self->data = (void *)d;

    static struct dt_action_def_t notebook_def = {};
    self->widget = GTK_WIDGET(dt_ui_notebook_new(&notebook_def));
    dt_action_define(DT_ACTION(self), NULL, N_("page"), GTK_WIDGET(self->widget), &notebook_def);
    dt_gui_add_help_link(self->widget, "image");

    GtkWidget *page1 = dt_ui_notebook_page(GTK_NOTEBOOK(self->widget), N_("images"), NULL);
    GtkWidget *page2 = dt_ui_notebook_page(GTK_NOTEBOOK(self->widget), N_("metadata"), NULL);

    // images operations
    d->page1 = gtk_grid_new();

    GtkGrid *grid = GTK_GRID(d->page1);
    gtk_container_add(GTK_CONTAINER(page1), d->page1);
    gtk_grid_set_column_homogeneous(grid, TRUE);
    int line = 0;

    d->remove_button = dt_action_button_new(
        self, N_("remove"), button_clicked, GINT_TO_POINTER(0),
        _("remove images from the image library, without deleting"), GDK_KEY_Delete, 0);
    gtk_grid_attach(grid, d->remove_button, 0, line, 2, 1);

    // delete button label and tooltip will be updated based on trash pref
    d->delete_button =
        dt_action_button_new(self, N_("delete"), button_clicked, GINT_TO_POINTER(1), NULL, 0, 0);
    gtk_grid_attach(grid, d->delete_button, 2, line++, 2, 1);

    d->move_button = dt_action_button_new(self, N_("move..."), button_clicked, GINT_TO_POINTER(8),
                                          _("move to other folder"), 0, 0);
    gtk_grid_attach(grid, d->move_button, 0, line, 2, 1);

    d->copy_button = dt_action_button_new(self, N_("copy..."), button_clicked, GINT_TO_POINTER(9),
                                          _("copy to other folder"), 0, 0);
    gtk_grid_attach(grid, d->copy_button, 2, line++, 2, 1);

    d->create_hdr_button =
        dt_action_button_new(self, N_("create HDR"), button_clicked, GINT_TO_POINTER(7),
                             _("create a high dynamic range image from selected shots"), 0, 0);
    gtk_grid_attach(grid, d->create_hdr_button, 0, line, 2, 1);

    d->duplicate_button =
        dt_action_button_new(self, N_("duplicate"), button_clicked, GINT_TO_POINTER(3),
                             _("add a duplicate to the image library, including its history stack"),
                             GDK_KEY_d, GDK_CONTROL_MASK);
    gtk_grid_attach(grid, d->duplicate_button, 2, line++, 2, 1);

    d->rotate_ccw_button = dtgtk_button_new(dtgtk_cairo_paint_refresh, CPF_NONE, NULL);
    ;
    gtk_widget_set_name(d->rotate_ccw_button, "non-flat");
    gtk_widget_set_tooltip_text(d->rotate_ccw_button, _("rotate selected images 90 degrees CCW"));
    gtk_grid_attach(grid, d->rotate_ccw_button, 0, line, 1, 1);
    g_signal_connect(G_OBJECT(d->rotate_ccw_button), "clicked", G_CALLBACK(button_clicked),
                     GINT_TO_POINTER(4));
    dt_action_define(DT_ACTION(self), NULL, N_("rotate selected images 90 degrees CCW"),
                     d->rotate_ccw_button, &dt_action_def_button);

    d->rotate_cw_button = dtgtk_button_new(dtgtk_cairo_paint_refresh, 1 | CPF_NONE, NULL);
    gtk_widget_set_name(d->rotate_cw_button, "non-flat");
    gtk_widget_set_tooltip_text(d->rotate_cw_button, _("rotate selected images 90 degrees CW"));
    gtk_grid_attach(grid, d->rotate_cw_button, 1, line, 1, 1);
    g_signal_connect(G_OBJECT(d->rotate_cw_button), "clicked", G_CALLBACK(button_clicked),
                     GINT_TO_POINTER(5));
    dt_action_define(DT_ACTION(self), NULL, N_("rotate selected images 90 degrees CW"),
                     d->rotate_cw_button, &dt_action_def_button);

    d->reset_button =
        dt_action_button_new(self, N_("reset rotation"), button_clicked, GINT_TO_POINTER(6),
                             _("reset rotation to EXIF data"), 0, 0);
    gtk_grid_attach(grid, d->reset_button, 2, line++, 2, 1);

    d->cache_button = dt_action_button_new(self, N_("copy locally"), button_clicked,
                                           GINT_TO_POINTER(12), _("copy the image locally"), 0, 0);
    gtk_grid_attach(grid, d->cache_button, 0, line, 2, 1);

    d->uncache_button =
        dt_action_button_new(self, N_("resync local copy"), button_clicked, GINT_TO_POINTER(13),
                             _("synchronize the image's XMP and remove the local copy"), 0, 0);
    gtk_grid_attach(grid, d->uncache_button, 2, line++, 2, 1);

    d->group_button = dt_action_button_new(
        self, NC_("selected images action", "group"), button_clicked, GINT_TO_POINTER(10),
        _("add selected images to expanded group or create a new one"), GDK_KEY_g,
        GDK_CONTROL_MASK);
    gtk_grid_attach(grid, d->group_button, 0, line, 2, 1);

    d->ungroup_button = dt_action_button_new(
        self, N_("ungroup"), button_clicked, GINT_TO_POINTER(11),
        _("remove selected images from the group"), GDK_KEY_g, GDK_CONTROL_MASK | GDK_SHIFT_MASK);
    gtk_grid_attach(grid, d->ungroup_button, 2, line++, 2, 1);

    // metadata operations
    grid = GTK_GRID(gtk_grid_new());
    gtk_container_add(GTK_CONTAINER(page2), GTK_WIDGET(grid));
    gtk_grid_set_column_homogeneous(grid, TRUE);

    dt_lib_module_t *meta = (dt_lib_module_t *)dt_action_section(DT_ACTION(self), N_("metadata"));
    line = -1;
#define META_FLAG_BUTTON(label, item, left, tooltip)                                               \
    {                                                                                              \
        GtkWidget *flag = gtk_check_button_new_with_label(_(label));                               \
        d->item##_flag = flag;                                                                     \
        gtk_widget_set_tooltip_text(flag, tooltip);                                                \
        ellipsize_button(flag);                                                                    \
        gtk_grid_attach(grid, flag, left, !left ? ++line : line, 3, 1);                            \
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(flag),                                      \
                                     dt_conf_get_bool("plugins/lighttable/copy_metadata/" #item)); \
        dt_action_define(DT_ACTION(meta), N_("flags"), label, flag, &dt_action_def_toggle);        \
        g_signal_connect(G_OBJECT(flag), "clicked", G_CALLBACK(_##item##_flag_callback), self);    \
    }

    META_FLAG_BUTTON(N_("ratings"), rating, 0, _("select ratings metadata"));
    META_FLAG_BUTTON(N_("colors"), colors, 3, _("select colors metadata"));
    META_FLAG_BUTTON(N_("tags"), tags, 0, _("select tags metadata"));
    META_FLAG_BUTTON(N_("metadata"), metadata, 0,
                     _("select darktable metadata (from metadata editor module)"));

    d->copy_metadata_button =
        dt_action_button_new(meta, N_("copy"), _copy_metadata_callback, self,
                             _("set the selected image as source of metadata"), 0, 0);
    gtk_grid_attach(grid, d->copy_metadata_button, 0, ++line, 2, 1);
    g_signal_connect(G_OBJECT(d->copy_metadata_button), "clicked",
                     G_CALLBACK(_copy_metadata_callback), self);

    d->paste_metadata_button =
        dt_action_button_new(meta, N_("paste"), _paste_metadata_callback, self,
                             _("paste selected metadata on selected images"), 0, 0);
    gtk_grid_attach(grid, d->paste_metadata_button, 2, line, 2, 1);

    d->clear_metadata_button =
        dt_action_button_new(meta, N_("clear"), _clear_metadata_callback, self,
                             _("clear selected metadata on selected images"), 0, 0);
    gtk_grid_attach(grid, d->clear_metadata_button, 4, line++, 2, 1);

    GtkWidget *pastemode = NULL;
    DT_BAUHAUS_COMBOBOX_NEW_FULL(pastemode, meta, NULL, N_("mode"),
                                 _("how to handle existing metadata"),
                                 dt_conf_get_int("plugins/lighttable/copy_metadata/pastemode"),
                                 _pastemode_combobox_changed, self, N_("merge"), N_("overwrite"));
    gtk_grid_attach(grid, pastemode, 0, line++, 6, 1);

    d->refresh_button =
        dt_action_button_new(meta, N_("refresh EXIF"), button_clicked, GINT_TO_POINTER(14),
                             _("update all image information to match changes to file\n"
                               "warning: resets star ratings unless you select\n"
                               "'ignore EXIF rating' in the 'import' module"),
                             0, 0);
    gtk_grid_attach(grid, d->refresh_button, 0, line++, 6, 1);

    d->set_monochrome_button = dt_action_button_new(
        meta, N_("monochrome"), _set_monochrome_callback, self,
        _("set selection as monochrome images and activate monochrome workflow"), 0, 0);
    gtk_grid_attach(grid, d->set_monochrome_button, 0, line, 3, 1);

    d->set_color_button = dt_action_button_new(meta, N_("color"), _set_color_callback, self,
                                               _("set selection as color images"), 0, 0);
    gtk_grid_attach(grid, d->set_color_button, 3, line++, 3, 1);

    DT_CONTROL_SIGNAL_HANDLE(DT_SIGNAL_PREFERENCES_CHANGE, _image_preference_changed);
    DT_CONTROL_SIGNAL_HANDLE(DT_SIGNAL_SELECTION_CHANGED, _image_selection_changed_callback);
    DT_CONTROL_SIGNAL_HANDLE(DT_SIGNAL_MOUSE_OVER_IMAGE_CHANGE, _mouse_over_image_callback);
    DT_CONTROL_SIGNAL_HANDLE(DT_SIGNAL_COLLECTION_CHANGED, _collection_updated_callback);

    dt_action_register(DT_ACTION(self), N_("duplicate virgin"), _duplicate_virgin, GDK_KEY_d,
                       GDK_CONTROL_MASK | GDK_SHIFT_MASK);

    _image_preference_changed(NULL, self); // update delete button label/tooltip
}
#undef ellipsize_button

void gui_reset(dt_lib_module_t *self)
{
    dt_lib_gui_queue_update(self);
}

void gui_cleanup(dt_lib_module_t *self)
{
    free(self->data);
    self->data = NULL;
}

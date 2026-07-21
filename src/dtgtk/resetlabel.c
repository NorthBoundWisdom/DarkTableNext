/*
    This file is part of darktable,
    Copyright (C) 2010-2020 darktable developers.

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

#include "dtgtk/resetlabel.h"
#include "gui/gtk.h"

G_DEFINE_TYPE(GtkDarktableResetLabel, dtgtk_reset_label, GTK_TYPE_BOX);

static void dtgtk_reset_label_class_init(GtkDarktableResetLabelClass *klass)
{
}

static void dtgtk_reset_label_init(GtkDarktableResetLabel *label)
{
}

static void _reset_label_callback(GtkGestureSingle *gesture, int n_press, double x, double y,
                                  gpointer user_data)
{
    GtkDarktableResetLabel *label = DTGTK_RESET_LABEL(user_data);
    if (n_press == 2)
    {
        memcpy(((char *)label->module->params) + label->offset,
               ((char *)label->module->default_params) + label->offset, label->size);
        dt_iop_gui_update(label->module);
        dt_dev_add_history_item(darktable.develop, label->module, FALSE);
    }
    (void)gesture;
    (void)x;
    (void)y;
}

// public functions
GtkWidget *dtgtk_reset_label_new(const gchar *text, dt_iop_module_t *module, void *param,
                                 int param_size)
{
    GtkDarktableResetLabel *label;
    label = g_object_new(dtgtk_reset_label_get_type(), "orientation", GTK_ORIENTATION_HORIZONTAL,
                         NULL);
    label->module = module;
    label->offset = (char *)param - (char *)module->params;
    label->size = param_size;

    if (label->offset < 0 || label->offset + label->size > module->params_size)
    {
        label->offset = (char *)param - (char *)module->default_params;
        if (label->offset < 0 || label->offset + label->size > module->params_size)
            dt_print(DT_DEBUG_ALWAYS, "[dtgtk_reset_label_new] reference outside %s params",
                     module->so->op);
    }

    label->lb = GTK_LABEL(gtk_label_new(text));
    gtk_widget_set_halign(GTK_WIDGET(label->lb), GTK_ALIGN_START);
    gtk_label_set_ellipsize(GTK_LABEL(label->lb), PANGO_ELLIPSIZE_END);
    gtk_widget_set_tooltip_text(GTK_WIDGET(label), _("double-click to reset"));
    dt_gui_box_add(label, GTK_WIDGET(label->lb));
    dt_gui_connect_click_all(label, _reset_label_callback, NULL, label);

    return (GtkWidget *)label;
}

void dtgtk_reset_label_set_text(GtkDarktableResetLabel *label, const gchar *str)
{
    gtk_label_set_text(label->lb, str);
}

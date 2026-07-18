/*
    This file is part of darktable.

    The former Objective-C++ implementation provided Cocoa-specific UI
    integration.  DarkTableNext currently uses GTK directly, so this C-only
    layer keeps the common application paths available without requiring an
    Objective-C++ compiler or Cocoa runtime.
*/

#include "osx.h"

#include <gio/gio.h>

float dt_osx_get_ppd()
{
    return 1.0f;
}

void dt_osx_disallow_fullscreen(GtkWidget *widget)
{
    (void)widget;
}

gboolean dt_osx_file_trash(const char *filename, GError **error)
{
    GFile *file = g_file_new_for_path(filename);
    const gboolean result = g_file_trash(file, NULL, error);
    g_object_unref(file);
    return result;
}

char *dt_osx_get_bundle_res_path()
{
    return NULL;
}

void dt_osx_prepare_environment()
{
}

void dt_osx_focus_window()
{
}

gboolean dt_osx_open_url(const char *url)
{
    GError *error = NULL;
    const gboolean result = g_app_info_launch_default_for_uri(url, NULL, &error);
    g_clear_error(&error);
    return result;
}

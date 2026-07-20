/*
    This file is part of darktable.

    The former Objective-C++ implementation provided Cocoa-specific UI
    integration.  DarkTableNext currently uses GTK directly, so this C-only
    layer keeps the common application paths available without requiring an
    Objective-C++ compiler or Cocoa runtime.
*/

#include "osx.h"

#include <gio/gio.h>
#include <string.h>

#include "whereami.h"

float dt_osx_get_ppd()
{
    GdkDisplay *display = gdk_display_get_default();
    if (!display)
        return 1.0f;

    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    if (!monitor && gdk_display_get_n_monitors(display) > 0)
        monitor = gdk_display_get_monitor(display, 0);

    if (!monitor)
        return 1.0f;

    const int scale = gdk_monitor_get_scale_factor(monitor);
    return scale > 0 ? (float)scale : 1.0f;
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
    const int executable_length = wai_getExecutablePath(NULL, 0, NULL);
    if (executable_length <= 0)
        return NULL;

    char *executable_path = g_malloc(executable_length + 1);
    if (wai_getExecutablePath(executable_path, executable_length, NULL) != executable_length)
    {
        g_free(executable_path);
        return NULL;
    }
    executable_path[executable_length] = '\0';

    const char *const contents = strstr(executable_path, ".app/Contents/MacOS/");
    if (!contents)
    {
        g_free(executable_path);
        return NULL;
    }

    char *bundle_path = g_strndup(executable_path, contents - executable_path + 4);
    char *result = g_build_filename(bundle_path, "Contents", "Resources", NULL);
    g_free(bundle_path);
    g_free(executable_path);

    if (!g_file_test(result, G_FILE_TEST_IS_DIR))
        g_clear_pointer(&result, g_free);

    return result;
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

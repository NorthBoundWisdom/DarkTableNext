/*
 This code is taken from http://git.gnome.org/browse/gobject-introspection/tree/giscanner/grealpath.h .
 According to http://git.gnome.org/browse/gobject-introspection/tree/COPYING it's licensed under the LGPLv2+.
*/

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

/** g_realpath: macOS helper until GLib exposes an equivalent API. */

static inline gchar *g_realpath(const char *path)
{
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
    char buffer[PATH_MAX] = {0};

    char *res = realpath(path, buffer);

    if (res)
    {
        return g_strdup(buffer);
    }
    else
    {
        fprintf(stderr, "path lookup '%s' fails with: '%s'\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/*
 *    This file is part of darktable,
 *    Copyright (C) 2017-2021 darktable developers.
 *
 *    darktable is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    darktable is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <gmodule.h>

#include "config.h"
#include "common/darktable.h"
#include "common/file_location.h"
#include "common/module.h"

typedef struct dt_module_prefetch_t
{
    gchar *plugindir;
    guint modules;
    gsize bytes;
} dt_module_prefetch_t;

static void _prefetch_directory(dt_module_prefetch_t *prefetch, const char *subdir)
{
    gchar *directory = g_build_filename(prefetch->plugindir, subdir, NULL);
    GDir *dir = g_dir_open(directory, 0, NULL);
    if (!dir)
    {
        g_free(directory);
        return;
    }

    const gchar *name = NULL;
    while ((name = g_dir_read_name(dir)))
    {
        if (!g_str_has_prefix(name, SHARED_MODULE_PREFIX) ||
            !g_str_has_suffix(name, SHARED_MODULE_SUFFIX))
            continue;

        gchar *path = g_build_filename(directory, name, NULL);
        gchar *contents = NULL;
        gsize length = 0;
        if (g_file_get_contents(path, &contents, &length, NULL))
        {
            prefetch->modules++;
            prefetch->bytes += length;
        }
        g_free(contents);
        g_free(path);
    }

    g_dir_close(dir);
    g_free(directory);
}

static gpointer _prefetch_modules(gpointer user_data)
{
    dt_module_prefetch_t *prefetch = user_data;
    const gint64 started = g_get_monotonic_time();
    static const char *module_directories[] = {
        "plugins", "plugins/imageio/format", "plugins/imageio/storage",
        "plugins/lighttable", "views", NULL};

    for (const char **subdir = module_directories; *subdir; subdir++)
        _prefetch_directory(prefetch, *subdir);

    dt_print(DT_DEBUG_PERF, "[module prefetch] read %u modules (%.1f MiB) in %.3f seconds",
             prefetch->modules, prefetch->bytes / (1024.0 * 1024.0),
             (g_get_monotonic_time() - started) / (double)G_USEC_PER_SEC);
    g_free(prefetch->plugindir);
    g_free(prefetch);
    return NULL;
}

GThread *dt_module_prefetch_modules(void)
{
    char plugindir[PATH_MAX] = {0};
    dt_loc_get_plugindir(plugindir, sizeof(plugindir));

    dt_module_prefetch_t *prefetch = g_new0(dt_module_prefetch_t, 1);
    prefetch->plugindir = g_strdup(plugindir);
    return g_thread_new("module-prefetch", _prefetch_modules, prefetch);
}

GList *dt_module_load_modules(const char *subdir, const size_t module_size,
                              int (*load_module_so)(void *module, const char *libname,
                                                    const char *plugin_name),
                              void (*init_module)(void *module),
                              gint (*sort_modules)(gconstpointer a, gconstpointer b))
{
    GList *plugin_list = NULL;
    char plugindir[PATH_MAX] = {0};
    const gchar *dir_name;
    dt_loc_get_plugindir(plugindir, sizeof(plugindir));
    g_strlcat(plugindir, subdir, sizeof(plugindir));
    GDir *dir = g_dir_open(plugindir, 0, NULL);
    if (!dir)
        return NULL;
    const int name_offset = strlen(SHARED_MODULE_PREFIX),
              name_end = strlen(SHARED_MODULE_PREFIX) + strlen(SHARED_MODULE_SUFFIX);
    while ((dir_name = g_dir_read_name(dir)))
    {
        // get lib*.so
        if (!g_str_has_prefix(dir_name, SHARED_MODULE_PREFIX))
            continue;
        if (!g_str_has_suffix(dir_name, SHARED_MODULE_SUFFIX))
            continue;
        char *plugin_name = g_strndup(dir_name + name_offset, strlen(dir_name) - name_end);
        void *module = calloc(1, module_size);
        gchar *libname = g_module_build_path(plugindir, plugin_name);
        const int res = load_module_so(module, libname, plugin_name);
        g_free(libname);
        g_free(plugin_name);
        if (res)
        {
            free(module);
            continue;
        }
        plugin_list = g_list_prepend(plugin_list, module);

        if (init_module)
            init_module(module);
    }
    g_dir_close(dir);

    if (sort_modules)
        plugin_list = g_list_sort(plugin_list, sort_modules);
    else
        plugin_list =
            g_list_reverse(plugin_list); // list was built in reverse order, so un-reverse it

    return plugin_list;
}

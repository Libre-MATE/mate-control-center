#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "libslab-utils.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

MateDesktopItem *libslab_mate_desktop_item_new_from_unknown_id(
    const gchar *id) {
  MateDesktopItem *item;
  gchar *basename;

  GError *error = NULL;

  if (!id) return NULL;

  item = mate_desktop_item_new_from_uri(id, 0, &error);

  if (!error)
    return item;
  else {
    g_error_free(error);
    error = NULL;
  }

  item = mate_desktop_item_new_from_file(id, 0, &error);

  if (!error)
    return item;
  else {
    g_error_free(error);
    error = NULL;
  }

  item = mate_desktop_item_new_from_basename(id, 0, &error);

  if (!error)
    return item;
  else {
    g_error_free(error);
    error = NULL;
  }

  basename = g_strrstr(id, "/");

  if (basename) {
    basename++;

    item = mate_desktop_item_new_from_basename(basename, 0, &error);

    if (!error)
      return item;
    else {
      g_error_free(error);
      error = NULL;
    }
  }

  return NULL;
}

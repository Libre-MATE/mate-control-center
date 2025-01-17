/* -*- mode: c; style: linux -*- */

/* keyboard-properties.c
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Jonathan Blandford
 *
 * Written by: Bradford Hovinen <hovinen@ximian.com>
 *             Rachel Hestilow <hestilow@ximian.com>
 *	       Jonathan Blandford <jrb@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gio/gio.h>
#include <stdlib.h>

#include "activate-settings-daemon.h"
#include "capplet-util.h"
#include "mate-keyboard-properties-a11y.h"
#include "mate-keyboard-properties-xkb.h"

#define KEYBOARD_SCHEMA "org.mate.peripherals-keyboard"
#define INTERFACE_SCHEMA "org.mate.interface"
#define TYPING_BREAK_SCHEMA "org.mate.typing-break"

#define GET_RANGE(s) GTK_RANGE(gtk_builder_get_object(dialog, s))

enum { RESPONSE_APPLY = 1, RESPONSE_CLOSE };

static GSettings *keyboard_settings = NULL;
static GSettings *interface_settings = NULL;
static GSettings *typing_break_settings = NULL;

static void dialog_response(GtkWidget *widget, gint response_id, guint data) {
  if (response_id == GTK_RESPONSE_HELP)
    capplet_help(GTK_WINDOW(widget), "goscustperiph-2");
  else
    gtk_main_quit();
}

static void setup_dialog(GtkBuilder *dialog) {
  gchar *monitor;

  g_settings_bind(keyboard_settings, "repeat",
                  gtk_builder_get_object(dialog, "repeat_toggle"), "active",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(keyboard_settings, "repeat",
                  gtk_builder_get_object(dialog, "repeat_table"), "sensitive",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(keyboard_settings, "delay",
                  gtk_range_get_adjustment(GET_RANGE("repeat_delay_scale")),
                  "value", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(keyboard_settings, "rate",
                  gtk_range_get_adjustment(GET_RANGE("repeat_speed_scale")),
                  "value", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(interface_settings, "cursor-blink",
                  gtk_builder_get_object(dialog, "cursor_toggle"), "active",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(interface_settings, "cursor-blink",
                  gtk_builder_get_object(dialog, "cursor_hbox"), "sensitive",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(
      interface_settings, "cursor-blink-time",
      gtk_range_get_adjustment(GET_RANGE("cursor_blink_time_scale")), "value",
      G_SETTINGS_BIND_DEFAULT);

  /* Ergonomics */
  monitor = g_find_program_in_path("mate-typing-monitor");
  if (monitor != NULL) {
    g_free(monitor);

    g_settings_bind(typing_break_settings, "enabled",
                    gtk_builder_get_object(dialog, "break_enabled_toggle"),
                    "active", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(typing_break_settings, "enabled",
                    gtk_builder_get_object(dialog, "break_details_table"),
                    "sensitive", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(typing_break_settings, "type-time",
                    gtk_builder_get_object(dialog, "break_enabled_spin"),
                    "value", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(typing_break_settings, "break-time",
                    gtk_builder_get_object(dialog, "break_interval_spin"),
                    "value", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(typing_break_settings, "allow-postpone",
                    gtk_builder_get_object(dialog, "break_postponement_toggle"),
                    "active", G_SETTINGS_BIND_DEFAULT);

  } else {
    /* don't show the typing break tab if the daemon is not available */
    GtkNotebook *nb =
        GTK_NOTEBOOK(gtk_builder_get_object(dialog, "keyboard_notebook"));
    gint tb_page = gtk_notebook_page_num(nb, WID("break_enabled_toggle"));
    gtk_notebook_remove_page(nb, tb_page);
  }

  g_signal_connect(gtk_builder_get_object(dialog, "keyboard_dialog"),
                   "response", (GCallback)dialog_response, NULL);

  setup_xkb_tabs(dialog);
  setup_a11y_tabs(dialog);
}

int main(int argc, char **argv) {
  GtkBuilder *dialog;
  GOptionContext *context;

  static gboolean apply_only = FALSE;
  static gboolean switch_to_typing_break_page = FALSE;
  static gboolean switch_to_a11y_page = FALSE;

  static GOptionEntry cap_options[] = {
      {"apply", 0, 0, G_OPTION_ARG_NONE, &apply_only,
       N_("Just apply settings and quit (compatibility only; now handled by "
          "daemon)"),
       NULL},
      {"init-session-settings", 0, 0, G_OPTION_ARG_NONE, &apply_only,
       N_("Just apply settings and quit (compatibility only; now handled by "
          "daemon)"),
       NULL},
      {"typing-break", 0, 0, G_OPTION_ARG_NONE, &switch_to_typing_break_page,
       N_("Start the page with the typing break settings showing"), NULL},
      {"a11y", 0, 0, G_OPTION_ARG_NONE, &switch_to_a11y_page,
       N_("Start the page with the accessibility settings showing"), NULL},
      {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL}};

  context = g_option_context_new(_("- MATE Keyboard Preferences"));
  g_option_context_add_main_entries(context, cap_options, GETTEXT_PACKAGE);

  capplet_init(context, &argc, &argv);
  g_option_context_free(context);

  activate_settings_daemon();

  keyboard_settings = g_settings_new(KEYBOARD_SCHEMA);
  interface_settings = g_settings_new(INTERFACE_SCHEMA);
  typing_break_settings = g_settings_new(TYPING_BREAK_SCHEMA);

  dialog = gtk_builder_new_from_resource(
      "/org/mate/mcc/keyboard/mate-keyboard-properties-dialog.ui");

  setup_dialog(dialog);

  GtkNotebook *nb =
      GTK_NOTEBOOK(gtk_builder_get_object(dialog, "keyboard_notebook"));
  gtk_widget_add_events(GTK_WIDGET(nb), GDK_SCROLL_MASK);
  g_signal_connect(nb, "scroll-event",
                   G_CALLBACK(capplet_notebook_scroll_event_cb), NULL);

  if (switch_to_typing_break_page) {
    gtk_notebook_set_current_page(nb, 4);
  } else if (switch_to_a11y_page) {
    gtk_notebook_set_current_page(nb, 2);
  }

  capplet_set_icon(WID("keyboard_dialog"), "input-keyboard");
  gtk_widget_show(WID("keyboard_dialog"));
  gtk_main();

  finalize_a11y_tabs();
  g_object_unref(keyboard_settings);
  g_object_unref(interface_settings);
  g_object_unref(typing_break_settings);

  return 0;
}

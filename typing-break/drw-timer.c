/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2009 Nathaniel Smith <njs@pobox.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "drw-timer.h"

#include <glib.h>

struct _DrwTimer {
  gint64 start_time;
};

DrwTimer *drw_timer_new(void) {
  DrwTimer *timer = g_new0(DrwTimer, 1);
  drw_timer_start(timer);
  return timer;
}

void drw_timer_start(DrwTimer *timer) { timer->start_time = g_get_real_time(); }

gint drw_timer_elapsed(DrwTimer *timer) {
  return (g_get_real_time() - timer->start_time) / G_USEC_PER_SEC;
}

void drw_timer_destroy(DrwTimer *timer) { g_free(timer); }

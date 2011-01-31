/* Copyright 2007-2011 Matthew Barnes
 *
 * This file is part of GNOME Video Arcade.
 *
 * GNOME Video Arcade is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * GNOME Video Arcade is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gva-time.h"

#include <string.h>
#include <time.h>

static gpointer
time_copy (gpointer boxed)
{
        gpointer boxed_copy;

        boxed_copy = g_slice_alloc (sizeof (time_t));
        memcpy (boxed_copy, boxed, sizeof (time_t));
        return boxed_copy;
}

static void
time_free (gpointer boxed)
{
        g_slice_free1 (sizeof (time_t), boxed);
}

GType
gva_time_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
                type = g_boxed_type_register_static (
                        "GvaTime", time_copy, time_free);

        return type;
}

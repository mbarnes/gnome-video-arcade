/* Copyright 2007 Matthew Barnes
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

#include "gva-mame-common.h"

GvaMameAsyncData *
gva_mame_async_data_new (GvaMameCallback callback,
                         gpointer user_data)
{
        GvaMameAsyncData *data;

        data = g_slice_new (GvaMameAsyncData);
        data->callback = callback;
        data->user_data = user_data;
        data->lineno = 0;

        return data;
}

void
gva_mame_async_data_free (GvaMameAsyncData *data)
{
        g_slice_free (GvaMameAsyncData, data);
}

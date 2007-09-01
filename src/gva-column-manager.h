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

/**
 * SECTION: gva-column-manager
 * @short_description: Manages the columns of a #GtkTreeView
 *
 * This widget provides a user interface for managing the order and visiblity
 * of columns in a #GtkTreeView.
 **/

#ifndef GVA_COLUMN_MANAGER_H
#define GVA_COLUMN_MANAGER_H

#include "gva-common.h"

/* Standard GObject macros */
#define GVA_TYPE_COLUMN_MANAGER \
        (gva_column_manager_get_type ())
#define GVA_COLUMN_MANAGER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_COLUMN_MANAGER, GvaColumnManager))
#define GVA_COLUMN_MANAGER_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_COLUMN_MANAGER, GvaColumnManagerClass))
#define GVA_IS_COLUMN_MANAGER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_COLUMN_MANAGER))
#define GVA_IS_COLUMN_MANAGER_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_COLUMN_MANAGER))
#define GVA_COLUMN_MANAGER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_COLUMN_MANAGER, GvaColumnManagerClass))

G_BEGIN_DECLS

typedef struct _GvaColumnManager GvaColumnManager;
typedef struct _GvaColumnManagerClass GvaColumnManagerClass;
typedef struct _GvaColumnManagerPrivate GvaColumnManagerPrivate;

struct _GvaColumnManager
{
        GtkHBox parent;

        GtkWidget *vbox;

        GvaColumnManagerPrivate *priv;
};

struct _GvaColumnManagerClass
{
        GtkHBoxClass parent_class;
};

GType           gva_column_manager_get_type     (void);
GtkWidget *     gva_column_manager_new          (GtkTreeView *managed_view);
GtkTreeView *   gva_column_manager_get_managed_view
                                                (GvaColumnManager *manager);
void            gva_column_manager_set_managed_view     
                                                (GvaColumnManager *manager,
                                                 GtkTreeView *managed_view);

G_END_DECLS

#endif /* GVA_COLUMN_MANAGER_H */

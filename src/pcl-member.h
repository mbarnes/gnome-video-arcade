/* PCL - Predicate Constraint Language
 * Copyright (C) 2006 The Boeing Company
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef PCL_MEMBER_H
#define PCL_MEMBER_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

/* NOTE: The `address' parameter and `offset' field types were chosen to
 *       match the types used in the G_STRUCT_OFFSET macro in GLib. */

typedef struct _PclMemberDef PclMemberDef;

/**
 * PclMemberFlags:
 * @PCL_MEMBER_FLAG_READONLY:
 *      The member cannot be assigned to.
 *
 * Used to check or determine characteristics of a #PclMemberDef struct.
 */
typedef enum {
        PCL_MEMBER_FLAG_READONLY        = 1 << 0
} PclMemberFlags;

struct _PclMemberDef {
        gchar *name;
        GType type;
        glong offset;
        PclMemberFlags flags;
        gchar *doc;
};

PclObject *     pcl_member_get                  (guint8 *address,
                                                 PclMemberDef *memdef);
gboolean        pcl_member_set                  (guint8 *address,
                                                 PclMemberDef *memdef,
                                                 PclObject *value);

G_END_DECLS

#endif /* PCL_MEMBER_H */

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

#ifndef PCL_STRING_H
#define PCL_STRING_H

#include "pcl-config.h"

G_BEGIN_DECLS

typedef struct _PclString PclString;
typedef struct _PclStringClass PclStringClass;

struct _PclString {
        PclObject parent;
        GString *string;
        guint hash;
};

struct _PclStringClass {
        PclObjectClass parent_class;
};

GType           pcl_string_get_type             (void);
PclObject *     pcl_string_get_type_object      (void);
gboolean        pcl_string_resize               (PclObject *self,
                                                 glong length);
gchar *         pcl_string_as_string            (PclObject *object);
PclObject *     pcl_string_from_string          (const gchar *v_string);
PclObject *     pcl_string_from_string_and_size (const gchar *v_string,
                                                 gssize size);
PclObject *     pcl_string_from_format          (const gchar *format, ...);
PclObject *     pcl_string_from_format_va       (const gchar *format,
                                                 va_list va);
PclObject *     pcl_string_intern_from_string   (const gchar *string);
void            pcl_string_intern_in_place      (PclObject **p_object);
void            pcl_string_concat               (PclObject **p_object,
                                                 PclObject *string);
void            pcl_string_concat_and_del       (PclObject **p_object,
                                                 PclObject *string);
PclObject *     pcl_string_join                 (PclObject *separator,
                                                 PclObject *sequence);
PclObject *     pcl_string_format               (PclObject *format,
                                                 PclObject *args);

/* Standard GObject macros */
#define PCL_TYPE_STRING \
        (pcl_string_get_type ())
#define PCL_STRING(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_STRING, PclString))
#define PCL_STRING_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_STRING, PclStringClass))
#define PCL_IS_STRING(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_STRING))
#define PCL_IS_STRING_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_STRING))
#define PCL_STRING_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_STRING, PclStringClass))

/* Fast access macros (use carefully) */
#define PCL_STRING_AS_STRING(obj) \
        (PCL_STRING (obj)->string->str)
#define PCL_STRING_GET_SIZE(obj) \
        ((glong) PCL_STRING (obj)->string->len)

G_END_DECLS

#endif /* PCL_STRING_H */

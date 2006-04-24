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

#ifndef PCL_FILE_H
#define PCL_FILE_H

#include "pcl-config.h"
#include "pcl-iterator.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclFile PclFile;
typedef struct _PclFileClass PclFileClass;

struct _PclFile {
        PclIterator parent;
        FILE *stream;
        PclObject *name;
        PclObject *mode;
        PclObject *encoding;
        gint (*close) (FILE *);
        gboolean binary;
        gboolean soft_space;
        gboolean skip_next_lf;
        gboolean univ_newline;
        gint newline_types;
        gchar *buffer;
        gchar *buffer_end;
        gchar *buffer_ptr;
};

struct _PclFileClass {
        PclIteratorClass parent_class;
};

GType           pcl_file_get_type               (void);
PclObject *     pcl_file_get_type_object        (void);
FILE *          pcl_file_as_file                (PclObject *file);
PclObject *     pcl_file_name                   (PclObject *file);
gint            pcl_file_soft_space             (PclObject *file,
                                                 gboolean new_flag);
PclObject *     pcl_file_from_file              (FILE *stream,
                                                 const gchar *name,
                                                 const gchar *mode,
                                                 gint (*close) (FILE *));
PclObject *     pcl_file_from_string            (const gchar *name,
                                                 const gchar *mode);
PclObject *     pcl_file_get_line               (PclObject *file,
                                                 gint size);
gboolean        pcl_file_set_encoding           (PclObject *file,
                                                 const gchar *encoding);
gboolean        pcl_file_write_object           (PclObject *file,
                                                 PclObject *object,
                                                 gint flags);
gboolean        pcl_file_write_string           (PclObject *file,
                                                 const gchar *string);
gint            pcl_file_as_file_descriptor     (PclObject *object);

gchar *         pcl_universal_newline_fgets     (gchar *buffer,
                                                 gsize n_bytes,
                                                 FILE *stream,
                                                 PclObject *object);
gsize           pcl_universal_newline_fread     (gchar *buffer,
                                                 gsize n_bytes,
                                                 FILE *stream,
                                                 PclObject *object);

/* Standard GObject macros */
#define PCL_TYPE_FILE \
        (pcl_file_get_type ())
#define PCL_FILE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_FILE, PclFile))
#define PCL_FILE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_FILE, PclFileClass))
#define PCL_IS_FILE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_FILE))
#define PCL_IS_FILE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_FILE))
#define PCL_FILE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_FILE, PclFileClass))

G_END_DECLS

#endif /* PCL_FILE_H */

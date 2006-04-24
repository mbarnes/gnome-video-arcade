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

#ifndef PCL_GENERATOR_H
#define PCL_GENERATOR_H

#include "pcl-config.h"
#include "pcl-frame.h"
#include "pcl-iterator.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclGenerator PclGenerator;
typedef struct _PclGeneratorClass PclGeneratorClass;

struct _PclGenerator {
        PclIterator parent;
        PclFrame *frame;
        gboolean running;
};

struct _PclGeneratorClass {
        PclIteratorClass parent_class;
};

GType           pcl_generator_get_type          (void);
PclObject *     pcl_generator_get_type_object   (void);
PclObject *     pcl_generator_new               (PclFrame *frame);

/* Standard GObject macros */
#define PCL_TYPE_GENERATOR \
        (pcl_generator_get_type ())
#define PCL_GENERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_GENERATOR, PclGenerator))
#define PCL_GENERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_GENERATOR, PclGeneratorClass))
#define PCL_IS_GENERATOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_GENERATOR))
#define PCL_IS_GENERATOR_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_GENERATOR, PclGeneratorClass))
#define PCL_GENERATOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_GENERATOR, PclGeneratorClass))

G_END_DECLS

#endif /* PCL_GENERATOR_H */

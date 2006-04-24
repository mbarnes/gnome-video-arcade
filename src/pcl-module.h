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

#ifndef PCL_MODULE_H
#define PCL_MODULE_H

#include "pcl-config.h"
#include "pcl-container.h"
#include "pcl-method.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclModule PclModule;
typedef struct _PclModuleClass PclModuleClass;

struct _PclModule {
        PclContainer parent;
        PclObject *dict;
        GModule *g_module;
};

struct _PclModuleClass {
        PclContainerClass parent_class;
};

GType           pcl_module_get_type             (void);
PclObject *     pcl_module_get_type_object      (void);
PclObject *     pcl_module_new                  (const gchar *name);
PclObject *     pcl_module_get_dict             (PclObject *module);
gchar *         pcl_module_get_name             (PclObject *module);
gchar *         pcl_module_get_filename         (PclObject *module);
gboolean        pcl_module_add_int              (PclObject *module,
                                                 const gchar *name,
                                                 glong value);
gboolean        pcl_module_add_float            (PclObject *module,
                                                 const gchar *name,
                                                 gdouble value);
gboolean        pcl_module_add_string           (PclObject *module,
                                                 const gchar *name,
                                                 const gchar *value);
gboolean        pcl_module_add_object           (PclObject *module,
                                                 const gchar *name,
                                                 PclObject *object);

/* Support Functions */
PclObject *     pcl_module_init                 (const gchar *name,
                                                 PclMethodDef *methods,
                                                 const gchar *doc);
void            _pcl_module_clear                (PclObject *module);

/* Standard GObject macros */
#define PCL_TYPE_MODULE \
        (pcl_module_get_type ())
#define PCL_MODULE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), PCL_TYPE_MODULE, PclModule))
#define PCL_MODULE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), PCL_TYPE_MODULE, PclModuleClass))
#define PCL_IS_MODULE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), PCL_TYPE_MODULE))
#define PCL_IS_MODULE_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), PCL_TYPE_MODULE))
#define PCL_MODULE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), PCL_TYPE_MODULE, PclModuleClass))

G_END_DECLS

#endif /* PCL_MODULE_H */

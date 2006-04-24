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

#ifndef PCL_IMPORT_H
#define PCL_IMPORT_H

#include "pcl-config.h"
#include "pcl-object.h"

G_BEGIN_DECLS

typedef struct _PclFileDescription PclFileDescription;
typedef struct _PclFrozenModule PclFrozenModule;

typedef enum {
        PCL_IMPORT_ERROR,
        PCL_IMPORT_SOURCE,
        PCL_IMPORT_COMPILED,
        PCL_IMPORT_EXTENSION,
        PCL_IMPORT_BUILTIN,
        PCL_IMPORT_PACKAGE,
        PCL_IMPORT_FROZEN,
        PCL_IMPORT_HOOK
} PclImportType;

struct _PclFileDescription {
        gchar *suffix;
        gchar *mode;
        PclImportType type;
};

struct _PclFrozenModule {
        gchar *name;
        guchar *code;
        gsize size;
};

PclObject *pcl_import_module (const gchar *name);
PclObject *pcl_import_module_ex (const gchar *name,
                                 PclObject *globals,
                                 PclObject *locals,
                                 PclObject *fromlist);
PclObject *pcl_import (PclObject *module_name);
PclObject *pcl_import_reload_module (PclObject *module);
PclObject *pcl_import_add_module (const gchar *name);
PclObject *pcl_import_exec_code_module (const gchar *name,
                                        PclObject *code);
PclObject *pcl_import_exec_code_module_ex (const gchar *name,
                                           PclObject *code,
                                           const gchar *pathname);
PclObject *pcl_import_get_module_dict (void);
void       pcl_import_cleanup (void);

extern PclFrozenModule *PCL_IMPORT_FROZEN_MODULES;

G_END_DECLS

#endif /* PCL_IMPORT_H */

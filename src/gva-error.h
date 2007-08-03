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
 * SECTION: gva-error
 * @short_description: Error Management
 **/

#ifndef GVA_ERROR_H
#define GVA_ERROR_H

#include "gva-common.h"

/**
 * GVA_ERROR:
 *
 * Error domain for general operations in GNOME Video Arcade.  Errors
 * in this domain will be from the #GvaError enumeration.  See #GError
 * for information on error domains.
 **/
#define GVA_ERROR               gva_error_quark ()

/**
 * GVA_SQLITE_ERROR:
 *
 * Error domain for SQLite operations in GNOME Video Arcade.  Errors
 * in this domain will be from SQLite's own
 * <ulink url="http://www.sqlite.org/capi3ref.html&num;SQLITE_ERROR">
 * result codes</ulink>.  See #GError for information on error domains.
 **/
#define GVA_SQLITE_ERROR        gva_sqlite_error_quark ()

G_BEGIN_DECLS

/**
 * GvaError:
 *
 * Error codes for general operations in GNOME Video Arcade.
 * 
 * GVA_ERROR_CONFIG:    Configuration error.
 * GVA_ERROR_MAME:      Error from a MAME process.
 * GVA_ERROR_QUERY:     SQL query error.
 * GVA_ERROR_SYSTEM:    System-level error.
 **/  
typedef enum
{
        GVA_ERROR_CONFIG,
        GVA_ERROR_MAME,
        GVA_ERROR_QUERY,
        GVA_ERROR_SYSTEM

} GvaError;

GQuark          gva_error_quark                 (void);
GQuark          gva_sqlite_error_quark          (void);
void            gva_error_handle                (GError **error);

G_END_DECLS

#endif /* GVA_ERROR_H */

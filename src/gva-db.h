/* Copyright 2007-2015 Matthew Barnes
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
 * SECTION: gva-db
 * @short_description: Database Management
 *
 * These functions help manage the game database.
 **/

#ifndef GVA_DB_H
#define GVA_DB_H

#include "gva-common.h"
#include "gva-process.h"

G_BEGIN_DECLS

gboolean        gva_db_init                     (GError **error);
GvaProcess *    gva_db_build                    (GError **error);
gboolean        gva_db_reset                    (GError **error);
gboolean        gva_db_execute                  (const gchar *sql,
                                                 GError **error);
gboolean        gva_db_get_table                (const gchar *sql,
                                                 gchar ***result,
                                                 gint *rows,
                                                 gint *columns,
                                                 GError **error);
gboolean        gva_db_transaction_begin        (GError **error);
gboolean        gva_db_transaction_commit       (GError **error);
gboolean        gva_db_transaction_rollback     (GError **error);
gboolean        gva_db_prepare                  (const gchar *sql,
                                                 sqlite3_stmt **stmt,
                                                 GError **error);
gboolean        gva_db_get_build                (gchar **build,
                                                 GError **error);
gboolean        gva_db_get_complete             (gboolean *complete,
                                                 GError **error);
gboolean        gva_db_mark_complete            (GError **error);
const gchar *   gva_db_get_filename             (void);
gboolean        gva_db_is_older_than            (const gchar *filename);
gboolean        gva_db_needs_rebuilt            (void);
void            gva_db_set_error                (GError **error,
                                                 gint code,
                                                 const gchar *message);

G_END_DECLS

#endif /* GVA_DB_H */

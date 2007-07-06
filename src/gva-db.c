#include "gva-db.h"

#include "gva-error.h"

#define SQL_CREATE_TABLE_MAME \
        "CREATE TABLE IF NOT EXISTS mame (build)"

#define SQL_CREATE_TABLE_GAME \
        "CREATE TABLE IF NOT EXISTS game (" \
                "name PRIMARY KEY, " \
                "sourcefile, " \
                "runnable DEFAULT yes, " \
                "cloneof, " \
                "romof, " \
                "sampleof, " \
                "description NOT NULL, " \
                "year, " \
                "manufacturer NOT NULL, " \
                "history, " \
                "video_screen NOT NULL, " \
                "video_orientation, " \
                "video_width, " \
                "video_height, " \
                "video_aspectx, " \
                "video_aspecty, " \
                "video_refresh, " \
                "sound_channels, " \
                "input_service DEFAULT no, " \
                "input_tilt DEFAULT no, " \
                "input_players, " \
                "input_control, " \
                "input_buttons, " \
                "input_coins, " \
                "driver_status, " \
                "driver_emulation, " \
                "driver_color, " \
                "driver_sound, " \
                "driver_graphic, " \
                "driver_cocktail, " \
                "driver_protection, " \
                "driver_savestate, " \
                "driver_palettesize)"

#define SQL_CREATE_TABLE_ROMSET \
        "CREATE TEMPORARY TABLE romset (name PRIMARY KEY, status)"

#define SQL_CREATE_TABLE_SAMPLESET \
        "CREATE TEMPORARY TABLE sampleset (name PRIMARY KEY, status)"

static sqlite3 *db = NULL;

static gboolean
db_create_tables (GError **error)
{
        return gva_db_execute (SQL_CREATE_TABLE_MAME, error)
                && gva_db_execute (SQL_CREATE_TABLE_GAME, error)
                && gva_db_execute (SQL_CREATE_TABLE_ROMSET, error)
                && gva_db_execute (SQL_CREATE_TABLE_SAMPLESET, error);
}

gboolean
gva_db_init (GError **error)
{
        const gchar *filename;

        g_return_val_if_fail (db == NULL, FALSE);

        filename = gva_db_get_filename ();

        if (sqlite3_open (filename, &db) != SQLITE_OK)
        {
                gva_db_set_error (error, 0, NULL);
                sqlite3_close (db);
                db = NULL;

                return FALSE;
        }

        return db_create_tables (error);
}

gboolean
gva_db_execute (const gchar *sql,
                GError **error)
{
        gint errcode;
        char *errmsg;

        g_return_val_if_fail (db != NULL, FALSE);
        g_return_val_if_fail (sql != NULL, FALSE);

        errcode = sqlite3_exec (db, sql, NULL, NULL, &errmsg);

        if (errcode != SQLITE_OK)
        {
                gva_db_set_error (error, errcode, errmsg);
                sqlite3_free (errmsg);
        }

        return (errcode == SQLITE_OK);
}

gboolean
gva_db_prepare (const gchar *sql,
                sqlite3_stmt **stmt,
                GError **error)
{
        gint errcode;

        g_return_val_if_fail (db != NULL, FALSE);
        g_return_val_if_fail (sql != NULL, FALSE);
        g_return_val_if_fail (stmt != NULL, FALSE);

        errcode = sqlite3_prepare_v2 (db, sql, -1, stmt, NULL);

        if (errcode != SQLITE_OK)
                gva_db_set_error (error, 0, NULL);

        return (errcode == SQLITE_OK);
}

static gint
db_get_build_cb (gpointer user_data,
                 gint n_columns,
                 gchar **column_values,
                 gchar **column_names)
{
        gchar **build = user_data;

        if (*build == NULL)
        {
                g_assert (n_columns > 0);
                *build = g_strdup (column_values[0]);
        }

        return 0;
}

gboolean
gva_db_get_build (gchar **build,
                  GError **error)
{
        const gchar *sql = "SELECT build FROM mame";
        gint errcode;
        char *errmsg;

        g_return_val_if_fail (db != NULL, FALSE);
        g_return_val_if_fail (build != NULL, FALSE);

        *build = NULL;  /* in case we don't get a result */
        errcode = sqlite3_exec (db, sql, db_get_build_cb, build, &errmsg);

        if (errcode != SQLITE_OK)
        {
                gva_db_set_error (error, errcode, errmsg);
                sqlite3_free (errmsg);
        }

        return (errcode == SQLITE_OK);
}

const gchar *
gva_db_get_filename (void)
{
        static gchar *filename = NULL;

        if (G_UNLIKELY (filename == NULL))
                filename = g_build_filename (
                        g_get_user_data_dir (), PACKAGE ".db", NULL);

        return filename;
}

void
gva_db_set_error (GError **error,
                  gint code,
                  const gchar *message)
{
        if (message == NULL)
        {
                code = sqlite3_errcode (db);
                message = sqlite3_errmsg (db);
        }

        g_set_error (error, GVA_SQLITE_ERROR, code, message);
}

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

#include "gva-db.h"

#include <string.h>

#include "gva-error.h"
#include "gva-util.h"
#include "gva-xmame.h"

#define ASSERT_OK(code) \
        if ((code) != SQLITE_OK) g_error ("%s", sqlite3_errmsg (db));

/* Based on MAME's DTD */
#define MAX_ELEMENT_DEPTH 4

#define SQL_CREATE_TABLE_MAME \
        "CREATE TABLE IF NOT EXISTS mame (build);"

#define SQL_CREATE_TABLE_GAME \
        "CREATE TABLE IF NOT EXISTS game (" \
                "name PRIMARY KEY, " \
                "sourcefile, " \
                "runnable DEFAULT 'yes', " \
                "cloneof, " \
                "romof, " \
                "romset, " \
                "sampleof, " \
                "sampleset, " \
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
                "input_service DEFAULT 'no', " \
                "input_tilt DEFAULT 'no', " \
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
                "driver_palettesize);"

#define SQL_INSERT_GAME \
        "INSERT INTO game VALUES (" \
                "@name, " \
                "@sourcefile, " \
                "@runnable, " \
                "@cloneof, " \
                "@romof, " \
                "@romset, " \
                "@sampleof, " \
                "@sampleset, " \
                "@description, " \
                "@year, " \
                "@manufacturer, " \
                "@history, " \
                "@video_screen, " \
                "@video_orientation, " \
                "@video_width, " \
                "@video_height, " \
                "@video_aspectx, " \
                "@video_aspecty, " \
                "@video_refresh, " \
                "@sound_channels, " \
                "@input_service, " \
                "@input_tilt, " \
                "@input_players, " \
                "@input_control, " \
                "@input_buttons, " \
                "@input_coins, " \
                "@driver_status, " \
                "@driver_emulation, " \
                "@driver_color, " \
                "@driver_sound, " \
                "@driver_graphic, " \
                "@driver_cocktail, " \
                "@driver_protection, " \
                "@driver_savestate, " \
                "@driver_palettesize);"

#define SQL_DELETE_NOT_FOUND \
        "DELETE FROM game WHERE romset == \"not found\";"

typedef struct _ParserData ParserData;

struct _ParserData
{
        GMarkupParseContext *context;
        GvaProcess *process;
        sqlite3_stmt *stmt;

        GHashTable *romsets;
        GHashTable *samplesets;
        GvaProcess *verify_romsets;
        GvaProcess *verify_samplesets;

        const gchar *element_stack[MAX_ELEMENT_DEPTH];
        guint element_stack_depth;
        guint progress;
};

/* Canonical names of XML elements and attributes */
static struct
{
        const gchar *aspectx;
        const gchar *aspecty;
        const gchar *build;
        const gchar *buttons;
        const gchar *channels;
        const gchar *cloneof;
        const gchar *cocktail;
        const gchar *coins;
        const gchar *color;
        const gchar *control;
        const gchar *description;
        const gchar *driver;
        const gchar *emulation;
        const gchar *game;
        const gchar *graphic;
        const gchar *height;
        const gchar *history;
        const gchar *input;
        const gchar *mame;
        const gchar *manufacturer;
        const gchar *name;
        const gchar *orientation;
        const gchar *palettesize;
        const gchar *players;
        const gchar *protection;
        const gchar *refresh;
        const gchar *romof;
        const gchar *runnable;
        const gchar *sampleof;
        const gchar *savestate;
        const gchar *screen;
        const gchar *service;
        const gchar *sound;
        const gchar *sourcefile;
        const gchar *status;
        const gchar *tilt;
        const gchar *video;
        const gchar *width;
        const gchar *year;

} intern;

static sqlite3 *db = NULL;

static void
db_parser_bind_text (ParserData *data,
                     const gchar *param,
                     const gchar *value)
{
        gint errcode;
        GError *error = NULL;

        errcode = sqlite3_bind_text (
                data->stmt, sqlite3_bind_parameter_index (data->stmt, param),
                g_locale_to_utf8 (value, -1, NULL, NULL, &error), -1, g_free);

        /* Handle conversion errors. */
        gva_error_handle (&error);

        if (errcode != SQLITE_OK)
        {
                gva_db_set_error (&error, 0, NULL);
                gva_error_handle (&error);
        }
}

static void
db_parser_start_element_driver (ParserData *data,
                                const gchar **attribute_name,
                                const gchar **attribute_value,
                                GError **error)
{
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.status)
                        param = "@driver_status";
                else if (attribute_name[ii] == intern.emulation)
                        param = "@driver_emulation";
                else if (attribute_name[ii] == intern.color)
                        param = "@driver_color";
                else if (attribute_name[ii] == intern.sound)
                        param = "@driver_sound";
                else if (attribute_name[ii] == intern.graphic)
                        param = "@driver_graphic";
                else if (attribute_name[ii] == intern.cocktail)
                        param = "@driver_cocktail";
                else if (attribute_name[ii] == intern.protection)
                        param = "@driver_protection";
                else if (attribute_name[ii] == intern.savestate)
                        param = "@driver_savestate";
                else if (attribute_name[ii] == intern.palettesize)
                        param = "@driver_palettesize";
                else
                        continue;

                db_parser_bind_text (data, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_game (ParserData *data,
                              const gchar **attribute_name,
                              const gchar **attribute_value,
                              GError **error)
{
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (data, "@runnable", "yes");

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.name)
                        param = "@name";
                else if (attribute_name[ii] == intern.sourcefile)
                        param = "@sourcefile";
                else if (attribute_name[ii] == intern.runnable)
                        param = "@runnable";
                else if (attribute_name[ii] == intern.cloneof)
                        param = "@cloneof";
                else if (attribute_name[ii] == intern.romof)
                        param = "@romof";
                else if (attribute_name[ii] == intern.sampleof)
                        param = "@sampleof";
                else
                        continue;

                db_parser_bind_text (data, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_input (ParserData *data,
                               const gchar **attribute_name,
                               const gchar **attribute_value,
                               GError **error)
{
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (data, "@input_service", "no");
        db_parser_bind_text (data, "@input_tilt", "no");

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.service)
                        param = "@input_service";
                else if (attribute_name[ii] == intern.tilt)
                        param = "@input_tilt";
                else if (attribute_name[ii] == intern.players)
                        param = "@input_players";
                else if (attribute_name[ii] == intern.control)
                        param = "@input_control";
                else if (attribute_name[ii] == intern.buttons)
                        param = "@input_buttons";
                else if (attribute_name[ii] == intern.coins)
                        param = "@input_coins";
                else
                        continue;

                db_parser_bind_text (data, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_mame (ParserData *data,
                              const gchar **attribute_name,
                              const gchar **attribute_value,
                              GError **error)
{
        const gchar *build = NULL;
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
                if (attribute_name[ii] == intern.build)
                        build = attribute_value[ii];

        if (build != NULL)
        {
                char *sql;

                sql = sqlite3_mprintf (
                        "INSERT INTO mame VALUES (%Q)", build);
                gva_db_execute (sql, error);
                sqlite3_free (sql);
        }
}

static void
db_parser_start_element_sound (ParserData *data,
                               const gchar **attribute_name,
                               const gchar **attribute_value,
                               GError **error)
{
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.channels)
                        param = "@sound_channels";
                else
                        continue;

                db_parser_bind_text (data, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_video (ParserData *data,
                               const gchar **attribute_name,
                               const gchar **attribute_value,
                               GError **error)
{
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.screen)
                        param = "@video_screen";
                else if (attribute_name[ii] == intern.orientation)
                        param = "@video_orientation";
                else if (attribute_name[ii] == intern.width)
                        param = "@video_width";
                else if (attribute_name[ii] == intern.height)
                        param = "@video_height";
                else if (attribute_name[ii] == intern.aspectx)
                        param = "@video_aspectx";
                else if (attribute_name[ii] == intern.aspecty)
                        param = "@video_aspecty";
                else if (attribute_name[ii] == intern.refresh)
                        param = "@video_refresh";
                else
                        continue;

                db_parser_bind_text (data, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element (GMarkupParseContext *context,
                         const gchar *element_name,
                         const gchar **attribute_name,
                         const gchar **attribute_value,
                         gpointer user_data,
                         GError **error)
{
        ParserData *data = user_data;
        const gchar **interned_name;
        guint length, ii;

        element_name = g_intern_string (element_name);
        g_assert (data->element_stack_depth < MAX_ELEMENT_DEPTH);
        data->element_stack[data->element_stack_depth++] = element_name;

        /* Build an array of interned attribute names.
         * Note: g_intern_string (NULL) returns NULL */
        length = g_strv_length ((gchar **) attribute_name) + 1;
        interned_name = g_newa (const gchar *, length);
        for (ii = 0; ii < length; ii++)
                interned_name[ii] = g_intern_string (attribute_name[ii]);
        attribute_name = interned_name;

        if (element_name == intern.driver)
                db_parser_start_element_driver (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.game)
                db_parser_start_element_game (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.input)
                db_parser_start_element_input (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.mame)
                db_parser_start_element_mame (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.sound)
                db_parser_start_element_sound (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.video)
                db_parser_start_element_video (
                        data, attribute_name, attribute_value, error);
}

static void
db_parser_end_element_game (ParserData *data,
                            GError **error)
{
        if (sqlite3_step (data->stmt) != SQLITE_DONE)
        {
                gva_db_set_error (error, 0, NULL);
                return;
        }

        ASSERT_OK (sqlite3_reset (data->stmt));
        ASSERT_OK (sqlite3_clear_bindings (data->stmt));

        gva_process_set_progress (data->process, ++data->progress);
}

static void
db_parser_end_element (GMarkupParseContext *context,
                       const gchar *element_name,
                       gpointer user_data,
                       GError **error)
{
        ParserData *data = user_data;

        g_assert (data->element_stack_depth > 0);
        element_name = data->element_stack[--data->element_stack_depth];

        if (element_name == intern.game)
                db_parser_end_element_game (data, error);
}

static void
db_parser_text (GMarkupParseContext *context,
                const gchar *text,
                gsize text_len,
                gpointer user_data,
                GError **error)
{
        ParserData *data = user_data;
        const gchar *element_name;

        g_assert (data->element_stack_depth > 0);
        element_name = data->element_stack[data->element_stack_depth - 1];

        if (element_name == intern.description)
                db_parser_bind_text (data, "@description", text);

        else if (element_name == intern.history)
                db_parser_bind_text (data, "@history", text);

        else if (element_name == intern.manufacturer)
                db_parser_bind_text (data, "@manufacturer", text);

        else if (element_name == intern.year)
                db_parser_bind_text (data, "@year", text);
}

static void
db_verify_update_foreach (const gchar *status,
                          GString *names,
                          const gchar *column)
{
        gchar *sql;
        GError *error = NULL;

        sql = g_strdup_printf (
                "UPDATE game SET %s=\"%s\" WHERE name IN (%s)",
                column, status, names->str);
        gva_db_execute (sql, &error);
        gva_error_handle (&error);
        g_free (sql);
}

static void
db_verify_update_status (GvaProcess *process,
                         GHashTable *hash_table,
                         gchar *column)
{
        if (process == NULL)
                return;

        while (!gva_process_has_exited (process, NULL))
                g_main_context_iteration (NULL, TRUE);

        if (process->error != NULL)
                return;

        g_hash_table_foreach (
                hash_table, (GHFunc) db_verify_update_foreach, column);
}

static void
db_verify_insert_status (const gchar *name,
                         const gchar *status,
                         GHashTable *hash_table)
{
        GString *string;

        string = g_hash_table_lookup (hash_table, status);
        if (string != NULL)
                g_string_append_printf (string, ", \"%s\"", name);
        else
        {
                string = g_string_sized_new (1024);
                g_string_printf (string, "\"%s\"", name);
                g_hash_table_insert (hash_table, g_strdup (status), string);
        }
}

static void
db_verify_string_free (GString *string)
{
        g_string_free (string, TRUE);
}

static GMarkupParser parser =
{
        db_parser_start_element,
        db_parser_end_element,
        db_parser_text,
        NULL,
        NULL
};

static ParserData *
db_parser_data_new (GvaProcess *process)
{
        ParserData *data;
        gint errcode;
        GError *error = NULL;

        data = g_slice_new0 (ParserData);
        data->context = g_markup_parse_context_new (&parser, 0, data, NULL);
        data->process = g_object_ref (process);

        data->romsets = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) db_verify_string_free);

        data->samplesets = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) db_verify_string_free);

        data->verify_romsets = gva_xmame_verify_romsets (
                (GvaXmameCallback) db_verify_insert_status,
                data->romsets, &error);
        gva_error_handle (&error);

        data->verify_samplesets = gva_xmame_verify_samplesets (
                (GvaXmameCallback) db_verify_insert_status,
                data->samplesets, &error);
        gva_error_handle (&error);

        if (!gva_db_prepare (SQL_INSERT_GAME, &data->stmt, &error))
                g_error ("%s", error->message);

        return data;
}

static void
db_parser_data_free (ParserData *data)
{
        g_markup_parse_context_free (data->context);
        g_object_unref (data->process);
        sqlite3_finalize (data->stmt);

        g_hash_table_destroy (data->romsets);
        g_hash_table_destroy (data->samplesets);

        if (data->verify_romsets != NULL)
                g_object_unref (data->verify_romsets);

        if (data->verify_samplesets != NULL)
                g_object_unref (data->verify_samplesets);

        g_slice_free (ParserData, data);
}

static void
db_parser_read (GvaProcess *process,
                ParserData *data)
{
        gchar *line;

        line = gva_process_stdout_read_line (process);

        if (process->error == NULL)
                g_markup_parse_context_parse (
                        data->context, line, -1, &process->error);

        g_free (line);
}

static void
db_parser_exit (GvaProcess *process,
                gint status,
                ParserData *data)
{
        GTimeVal time_elapsed;

        if (process->error == NULL)
                g_markup_parse_context_end_parse (
                        data->context, &process->error);

        db_verify_update_status (
                data->verify_romsets, data->romsets, "romset");
        db_verify_update_status (
                data->verify_samplesets, data->samplesets, "sampleset");
        gva_db_execute (SQL_DELETE_NOT_FOUND, &process->error);

        gva_process_get_time_elapsed (process, &time_elapsed);

        g_message (
                "Database built in %d.%d seconds.",
                time_elapsed.tv_sec, time_elapsed.tv_usec / 100000);

        db_parser_data_free (data);
}

static gboolean
db_create_tables (GError **error)
{
        return gva_db_execute (SQL_CREATE_TABLE_MAME, error)
                && gva_db_execute (SQL_CREATE_TABLE_GAME, error);
}

static void
db_function_isfavorite (sqlite3_context *context,
                        gint n_values,
                        sqlite3_value **values)
{
        const gchar *name;

        g_assert (n_values == 1);

        name = (const gchar *) sqlite3_value_text (values[0]);
        if (gva_favorites_contains (name))
                sqlite3_result_text (context, "yes", -1, SQLITE_STATIC);
        else
                sqlite3_result_text (context, "no", -1, SQLITE_STATIC);
}

gboolean
gva_db_init (GError **error)
{
        const gchar *filename;
        gint errcode;

        g_return_val_if_fail (db == NULL, FALSE);

        filename = gva_db_get_filename ();

        if (sqlite3_open (filename, &db) != SQLITE_OK)
                goto fail;

        errcode = sqlite3_create_function (
                db, "isfavorite", 1, SQLITE_ANY, NULL,
                db_function_isfavorite, NULL, NULL);
        if (errcode != SQLITE_OK)
                goto fail;

        return db_create_tables (error);

fail:
        gva_db_set_error (error, 0, NULL);
        sqlite3_close (db);
        db = NULL;

        return FALSE;
}

GvaProcess *
gva_db_build (GError **error)
{
        const gchar *filename;
        GvaProcess *process;
        ParserData *data;

        g_return_val_if_fail (db != NULL, NULL);

        /* Initialize the list of canonical names. */
        intern.aspectx      = g_intern_static_string ("aspectx");
        intern.aspecty      = g_intern_static_string ("aspecty");
        intern.build        = g_intern_static_string ("build");
        intern.buttons      = g_intern_static_string ("buttons");
        intern.channels     = g_intern_static_string ("channels");
        intern.cloneof      = g_intern_static_string ("cloneof");
        intern.cocktail     = g_intern_static_string ("cocktail");
        intern.coins        = g_intern_static_string ("coins");
        intern.color        = g_intern_static_string ("color");
        intern.control      = g_intern_static_string ("control");
        intern.description  = g_intern_static_string ("description");
        intern.driver       = g_intern_static_string ("driver");
        intern.emulation    = g_intern_static_string ("emulation");
        intern.game         = g_intern_static_string ("game");
        intern.graphic      = g_intern_static_string ("graphic");
        intern.height       = g_intern_static_string ("height");
        intern.history      = g_intern_static_string ("history");
        intern.input        = g_intern_static_string ("input");
        intern.mame         = g_intern_static_string ("mame");
        intern.manufacturer = g_intern_static_string ("manufacturer");
        intern.name         = g_intern_static_string ("name");
        intern.orientation  = g_intern_static_string ("orientation");
        intern.palettesize  = g_intern_static_string ("palettesize");
        intern.players      = g_intern_static_string ("players");
        intern.protection   = g_intern_static_string ("protection");
        intern.refresh      = g_intern_static_string ("refresh");
        intern.romof        = g_intern_static_string ("romof");
        intern.runnable     = g_intern_static_string ("runnable");
        intern.sampleof     = g_intern_static_string ("sampleof");
        intern.savestate    = g_intern_static_string ("savestate");
        intern.screen       = g_intern_static_string ("screen");
        intern.service      = g_intern_static_string ("service");
        intern.sound        = g_intern_static_string ("sound");
        intern.sourcefile   = g_intern_static_string ("sourcefile");
        intern.status       = g_intern_static_string ("status");
        intern.tilt         = g_intern_static_string ("tilt");
        intern.video        = g_intern_static_string ("video");
        intern.width        = g_intern_static_string ("width");
        intern.year         = g_intern_static_string ("year");

        if (!gva_db_reset (error))
                return NULL;

        process = gva_xmame_list_xml (error);
        if (process == NULL)
                return NULL;

        data = db_parser_data_new (process);

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (db_parser_read), data);

        g_signal_connect (
                process, "exited",
                G_CALLBACK (db_parser_exit), data);

        return process;
}

gboolean
gva_db_reset (GError **error)
{
        const gchar *filename;

        g_return_val_if_fail (db != NULL, FALSE);

        filename = gva_db_get_filename ();

        sqlite3_close (db);
        db = NULL;

        if (g_file_test (filename, G_FILE_TEST_EXISTS))
                g_remove (filename);

        return gva_db_init (error);
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

gboolean
gva_db_needs_rebuilt (void)
{
        GConfClient *client;
        gchar *db_build_id = NULL;
        gchar *mame_version = NULL;
        const gchar *reason;
        gboolean rebuild;
        GError *error = NULL;

#define TEST_CASE(expr) \
        if ((rebuild = (expr))) goto exit;

        /* Begin test cases for rebuilding the games database.
         * The macro tests whether the database SHOULD be rebuilt. */

        reason = "the user requested it";
        TEST_CASE (opt_build_database);

        reason = PACKAGE_NAME "'s version changed";
        TEST_CASE (gva_get_last_version () == NULL);
        TEST_CASE (strcmp (gva_get_last_version (), PACKAGE_VERSION) != 0);

        reason = "the database does not have a build ID";
        gva_db_get_build (&db_build_id, &error);
        gva_error_handle (&error);
        TEST_CASE (db_build_id == NULL);

        reason = "the MAME version could not be determined";
        mame_version = gva_xmame_get_version (&error);
        gva_error_handle (&error);
        TEST_CASE (mame_version == NULL);

        reason = "the database build ID does not match the MAME version";
        TEST_CASE (strstr (mame_version, db_build_id) == NULL);

        /* ... add more tests here ... */

#undef TEST_CASE

exit:
        if (rebuild)
                g_message ("Building database because %s.", reason);
        else
                g_message ("Database seems up-to-date; no rebuild necessary.");

        g_free (db_build_id);
        g_free (mame_version);

        return rebuild;
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

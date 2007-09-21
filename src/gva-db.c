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
#include "gva-favorites.h"
#include "gva-mame.h"
#include "gva-util.h"

#define ASSERT_OK(code) \
        if ((code) != SQLITE_OK) g_error ("%s", sqlite3_errmsg (db));

/* Based on MAME's DTD */
#define MAX_ELEMENT_DEPTH 4

#define SQL_CREATE_TABLE_MAME \
        "CREATE TABLE IF NOT EXISTS mame (" \
                "build, " \
                "debug DEFAULT 'no' " \
                "CHECK (debug in ('yes', 'no')));"

#define SQL_CREATE_TABLE_GAME \
        "CREATE TABLE IF NOT EXISTS game (" \
                "name PRIMARY KEY, " \
                "sourcefile, " \
                "runnable DEFAULT 'yes' " \
                "CHECK (runnable in ('yes', 'no')), " \
                "cloneof, " \
                "romof, " \
                "romset " \
                "CHECK (romset in " \
                "('good', 'best available', 'bad')), " \
                "sampleof, " \
                "sampleset " \
                "CHECK (sampleset in " \
                "('good', 'best available', 'bad')), " \
                "description NOT NULL, " \
                "year, " \
                "manufacturer NOT NULL, " \
                "sound_channels, " \
                "input_service DEFAULT 'no' " \
                "CHECK (input_service in ('yes', 'no')), " \
                "input_tilt DEFAULT 'no' " \
                "CHECK (input_tilt in ('yes', 'no')), " \
                "input_players, " \
                "input_buttons, " \
                "input_coins, " \
                "driver_status " \
                "CHECK (driver_status in " \
                "('good', 'imperfect', 'preliminary')), " \
                "driver_emulation " \
                "CHECK (driver_emulation in " \
                "('good', 'imperfect', 'preliminary')), " \
                "driver_color " \
                "CHECK (driver_color in " \
                "('good', 'imperfect', 'preliminary')), " \
                "driver_sound " \
                "CHECK (driver_sound in " \
                "('good', 'imperfect', 'preliminary')), " \
                "driver_graphic " \
                "CHECK (driver_graphic in " \
                "('good', 'imperfect', 'preliminary')), " \
                "driver_cocktail " \
                "CHECK (driver_cocktail in " \
                "('good', 'imperfect', 'preliminary')), " \
                "driver_protection " \
                "CHECK (driver_protection in " \
                "('good', 'imperfect', 'preliminary')), " \
                "driver_savestate " \
                "CHECK (driver_savestate in " \
                "('supported', 'unsupported')), " \
                "driver_palettesize);"

#define SQL_CREATE_TABLE_BIOSSET \
        "CREATE TABLE IF NOT EXISTS biosset (" \
                "game NOT NULL, " \
                "name NOT NULL, " \
                "description NOT NULL, " \
                "default_ DEFAULT 'no' " \
                "CHECK (default_ in ('yes', 'no')));"

#define SQL_CREATE_TABLE_ROM \
        "CREATE TABLE IF NOT EXISTS rom (" \
                "game NOT NULL, " \
                "name NOT NULL, " \
                "bios, " \
                "size NOT NULL, " \
                "crc, " \
                "md5, " \
                "sha1, " \
                "merge, " \
                "region, " \
                "offset, " \
                "status DEFAULT 'good' " \
                "CHECK (status in ('baddump', 'nodump', 'good')), " \
                "dispose DEFAULT 'no' " \
                "CHECK (dispose in ('yes', 'no')));"

#define SQL_CREATE_TABLE_DISK \
        "CREATE TABLE IF NOT EXISTS disk (" \
                "game NOT NULL, " \
                "name NOT NULL, " \
                "md5, " \
                "sha1, " \
                "merge, " \
                "region, " \
                "index_, " \
                "status DEFAULT 'good' " \
                "CHECK (status in ('baddump', 'nodump', 'good')));"

#define SQL_CREATE_TABLE_SAMPLE \
        "CREATE TABLE IF NOT EXISTS sample (" \
                "game NOT NULL, " \
                "name NOT NULL);"

#define SQL_CREATE_TABLE_CHIP \
        "CREATE TABLE IF NOT EXISTS chip (" \
                "game NOT NULL, " \
                "name NOT NULL, " \
                "type NOT NULL " \
                "CHECK (type in ('cpu', 'audio')), " \
                "clock);"

#define SQL_CREATE_TABLE_DISPLAY \
        "CREATE TABLE IF NOT EXISTS display (" \
                "game NOT NULL, " \
                "type NOT NULL " \
                "CHECK (type in ('raster', 'vector')), " \
                "rotate NOT NULL " \
                "CHECK (rotate in ('0', '90', '180', '270')), " \
                "flipx DEFAULT 'no' " \
                "CHECK (flipx in ('yes', 'no')), " \
                "width, " \
                "height, " \
                "refresh NOT NULL);"

#define SQL_CREATE_TABLE_CONTROL \
        "CREATE TABLE IF NOT EXISTS control (" \
                "game NOT NULL, " \
                "type NOT NULL, " \
                "minimum, " \
                "maximum, " \
                "sensitivity, " \
                "keydelta, " \
                "reverse DEFAULT 'no' " \
                "CHECK (reverse in ('yes', 'no')));"

#define SQL_CREATE_TABLE_DIPVALUE \
        "CREATE TABLE IF NOT EXISTS dipvalue (" \
                "game NOT NULL, " \
                "dipswitch NOT NULL, " \
                "name NOT NULL, " \
                "default_ DEFAULT 'no' " \
                "CHECK (default_ in ('yes', 'no')));"

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
                "@sound_channels, " \
                "@input_service, " \
                "@input_tilt, " \
                "@input_players, " \
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

#define SQL_INSERT_BIOSSET \
        "INSERT INTO biosset VALUES (" \
                "@game, " \
                "@name, " \
                "@description, " \
                "@default_);"

#define SQL_INSERT_ROM \
        "INSERT INTO rom VALUES (" \
                "@game, " \
                "@name, " \
                "@bios, " \
                "@size, " \
                "@crc, " \
                "@md5, " \
                "@sha1, " \
                "@merge, " \
                "@region, " \
                "@offset, " \
                "@status, " \
                "@dispose);"

#define SQL_INSERT_DISK \
        "INSERT INTO disk VALUES (" \
                "@game, " \
                "@name, " \
                "@md5, " \
                "@sha1, " \
                "@merge, " \
                "@region, " \
                "@index_, " \
                "@status);"

#define SQL_INSERT_SAMPLE \
        "INSERT INTO sample VALUES (" \
                "@game, " \
                "@name);"

#define SQL_INSERT_CHIP \
        "INSERT INTO chip VALUES (" \
                "@game, " \
                "@name, " \
                "@type, " \
                "@clock);"

#define SQL_INSERT_DISPLAY \
        "INSERT INTO display VALUES (" \
                "@game, " \
                "@type, " \
                "@rotate, " \
                "@flipx, " \
                "@width, " \
                "@height, " \
                "@refresh);"

#define SQL_INSERT_CONTROL \
        "INSERT INTO control VALUES (" \
                "@game, " \
                "@type, " \
                "@minimum, " \
                "@maximum, " \
                "@sensitivity, " \
                "@keydelta, " \
                "@reverse);"

#define SQL_INSERT_DIPVALUE \
        "INSERT INTO dipvalue VALUES (" \
                "@game, " \
                "@dipswitch, " \
                "@name, " \
                "@default_);"

#define SQL_DELETE_NOT_FOUND \
        "DELETE FROM game WHERE romset == \"not found\";"

typedef struct _ParserData ParserData;

struct _ParserData
{
        GMarkupParseContext *context;
        GvaProcess *process;

        sqlite3_stmt *insert_game_stmt;
        sqlite3_stmt *insert_biosset_stmt;
        sqlite3_stmt *insert_rom_stmt;
        sqlite3_stmt *insert_disk_stmt;
        sqlite3_stmt *insert_sample_stmt;
        sqlite3_stmt *insert_chip_stmt;
        sqlite3_stmt *insert_display_stmt;
        sqlite3_stmt *insert_control_stmt;
        sqlite3_stmt *insert_dipvalue_stmt;

        const gchar *element_stack[MAX_ELEMENT_DEPTH];
        guint element_stack_depth;
        gchar *dipswitch;
        gchar *game;
};

/* Canonical names of XML elements and attributes */
static struct
{
        const gchar *aspectx;
        const gchar *aspecty;
        const gchar *bios;
        const gchar *biosset;
        const gchar *build;
        const gchar *buttons;
        const gchar *channels;
        const gchar *chip;
        const gchar *clock;
        const gchar *cloneof;
        const gchar *cocktail;
        const gchar *coins;
        const gchar *color;
        const gchar *control;
        const gchar *crc;
        const gchar *default_;
        const gchar *description;
        const gchar *dipswitch;
        const gchar *dipvalue;
        const gchar *disk;
        const gchar *display;
        const gchar *dispose;
        const gchar *driver;
        const gchar *emulation;
        const gchar *flipx;
        const gchar *game;
        const gchar *graphic;
        const gchar *height;
        const gchar *index_;
        const gchar *input;
        const gchar *keydelta;
        const gchar *mame;
        const gchar *manufacturer;
        const gchar *maximum;
        const gchar *md5;
        const gchar *merge;
        const gchar *minimum;
        const gchar *name;
        const gchar *offset;
        const gchar *orientation;
        const gchar *palettesize;
        const gchar *players;
        const gchar *protection;
        const gchar *refresh;
        const gchar *region;
        const gchar *reverse;
        const gchar *rom;
        const gchar *romof;
        const gchar *rotate;
        const gchar *runnable;
        const gchar *sample;
        const gchar *sampleof;
        const gchar *savestate;
        const gchar *screen;
        const gchar *sensitivity;
        const gchar *service;
        const gchar *sha1;
        const gchar *size;
        const gchar *sound;
        const gchar *sourcefile;
        const gchar *status;
        const gchar *tilt;
        const gchar *type;
        const gchar *width;
        const gchar *year;

} intern;

static sqlite3 *db = NULL;

static void
db_parser_bind_text (sqlite3_stmt *stmt,
                     const gchar *param,
                     const gchar *value)
{
        gint errcode;
        GError *error = NULL;

        errcode = sqlite3_bind_text (
                stmt, sqlite3_bind_parameter_index (stmt, param),
                g_locale_to_utf8 (value, -1, NULL, NULL, &error), -1, g_free);

        /* Handle conversion errors. */
        gva_error_handle (&error);

        if (errcode != SQLITE_OK)
        {
                gva_db_set_error (&error, 0, NULL);
                gva_error_handle (&error);
        }
}

static gboolean
db_parser_exec_stmt (sqlite3_stmt *stmt,
                     GError **error)
{
        if (sqlite3_step (stmt) != SQLITE_DONE)
        {
                gva_db_set_error (error, 0, NULL);
                return FALSE;
        }

        ASSERT_OK (sqlite3_reset (stmt));
        ASSERT_OK (sqlite3_clear_bindings (stmt));

        return TRUE;
}

static void
db_parser_start_element_biosset (ParserData *data,
                                 const gchar **attribute_name,
                                 const gchar **attribute_value,
                                 GError **error)
{
        sqlite3_stmt *stmt = data->insert_biosset_stmt;
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (stmt, "@default_", "no");

        db_parser_bind_text (stmt, "@game", data->game);

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.name)
                        param = "@name";
                else if (attribute_name[ii] == intern.description)
                        param = "@description";
                else if (attribute_name[ii] == intern.default_)
                        param = "@default_";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_chip (ParserData *data,
                              const gchar **attribute_name,
                              const gchar **attribute_value,
                              GError **error)
{
        sqlite3_stmt *stmt = data->insert_chip_stmt;
        gint ii;

        db_parser_bind_text (stmt, "@game", data->game);

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.name)
                        param = "@name";
                else if (attribute_name[ii] == intern.type)
                        param = "@type";
                else if (attribute_name[ii] == intern.clock)
                        param = "@clock";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_control (ParserData *data,
                                 const gchar **attribute_name,
                                 const gchar **attribute_value,
                                 GError **error)
{
        sqlite3_stmt *stmt = data->insert_control_stmt;
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (stmt, "@reverse", "no");

        db_parser_bind_text (stmt, "@game", data->game);

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.type)
                        param = "@type";
                else if (attribute_name[ii] == intern.minimum)
                        param = "@minimum";
                else if (attribute_name[ii] == intern.maximum)
                        param = "@maximum";
                else if (attribute_name[ii] == intern.sensitivity)
                        param = "@sensitivity";
                else if (attribute_name[ii] == intern.keydelta)
                        param = "@keydelta";
                else if (attribute_name[ii] == intern.reverse)
                        param = "@reverse";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_dipswitch (ParserData *data,
                                   const gchar **attribute_name,
                                   const gchar **attribute_value,
                                   GError **error)
{
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
                if (attribute_name[ii] == intern.name)
                        data->dipswitch = g_strdup (attribute_value[ii]);
}

static void
db_parser_start_element_dipvalue (ParserData *data,
                                  const gchar **attribute_name,
                                  const gchar **attribute_value,
                                  GError **error)
{
        sqlite3_stmt *stmt = data->insert_dipvalue_stmt;
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (stmt, "@default_", "no");

        db_parser_bind_text (stmt, "@game", data->game);
        db_parser_bind_text (stmt, "@dipswitch", data->dipswitch);

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.name)
                        param = "@name";
                else if (attribute_name[ii] == intern.default_)
                        param = "@default_";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_disk (ParserData *data,
                              const gchar **attribute_name,
                              const gchar **attribute_value,
                              GError **error)
{
        sqlite3_stmt *stmt = data->insert_disk_stmt;
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (stmt, "@status", "good");

        db_parser_bind_text (stmt, "@game", data->game);

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.name)
                        param = "@name";
                else if (attribute_name[ii] == intern.md5)
                        param = "@md5";
                else if (attribute_name[ii] == intern.sha1)
                        param = "@sha1";
                else if (attribute_name[ii] == intern.merge)
                        param = "@merge";
                else if (attribute_name[ii] == intern.region)
                        param = "@region";
                else if (attribute_name[ii] == intern.index_)
                        param = "@index_";
                else if (attribute_name[ii] == intern.status)
                        param = "@status";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_display (ParserData *data,
                                 const gchar **attribute_name,
                                 const gchar **attribute_value,
                                 GError **error)
{
        sqlite3_stmt *stmt = data->insert_display_stmt;
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (stmt, "@flipx", "no");

        db_parser_bind_text (stmt, "@game", data->game);

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.type)
                        param = "@type";
                else if (attribute_name[ii] == intern.rotate)
                        param = "@rotate";
                else if (attribute_name[ii] == intern.flipx)
                        param = "@flipx";
                else if (attribute_name[ii] == intern.width)
                        param = "@width";
                else if (attribute_name[ii] == intern.height)
                        param = "@height";
                else if (attribute_name[ii] == intern.refresh)
                        param = "@refresh";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_driver (ParserData *data,
                                const gchar **attribute_name,
                                const gchar **attribute_value,
                                GError **error)
{
        sqlite3_stmt *stmt = data->insert_game_stmt;
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

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_game (ParserData *data,
                              const gchar **attribute_name,
                              const gchar **attribute_value,
                              GError **error)
{
        sqlite3_stmt *stmt = data->insert_game_stmt;
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (stmt, "@runnable", "yes");

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.name)
                {
                        param = "@name";
                        data->game = g_strdup (attribute_value[ii]);
                }
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

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_input (ParserData *data,
                               const gchar **attribute_name,
                               const gchar **attribute_value,
                               GError **error)
{
        sqlite3_stmt *stmt = data->insert_game_stmt;
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (stmt, "@input_service", "no");
        db_parser_bind_text (stmt, "@input_tilt", "no");

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.service)
                        param = "@input_service";
                else if (attribute_name[ii] == intern.tilt)
                        param = "@input_tilt";
                else if (attribute_name[ii] == intern.players)
                        param = "@input_players";
                else if (attribute_name[ii] == intern.buttons)
                        param = "@input_buttons";
                else if (attribute_name[ii] == intern.coins)
                        param = "@input_coins";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
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
                        "INSERT INTO mame (build) VALUES (%Q)", build);
                gva_db_execute (sql, error);
                sqlite3_free (sql);
        }
}

static void
db_parser_start_element_rom (ParserData *data,
                             const gchar **attribute_name,
                             const gchar **attribute_value,
                             GError **error)
{
        sqlite3_stmt *stmt = data->insert_rom_stmt;
        gint ii;

        /* Bind default values. */
        db_parser_bind_text (stmt, "@status", "good");
        db_parser_bind_text (stmt, "@dispose", "no");

        db_parser_bind_text (stmt, "@game", data->game);

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.name)
                        param = "@name";
                else if (attribute_name[ii] == intern.bios)
                        param = "@bios";
                else if (attribute_name[ii] == intern.size)
                        param = "@size";
                else if (attribute_name[ii] == intern.crc)
                        param = "@crc";
                else if (attribute_name[ii] == intern.md5)
                        param = "@md5";
                else if (attribute_name[ii] == intern.sha1)
                        param = "@sha1";
                else if (attribute_name[ii] == intern.merge)
                        param = "@merge";
                else if (attribute_name[ii] == intern.region)
                        param = "@region";
                else if (attribute_name[ii] == intern.offset)
                        param = "@offset";
                else if (attribute_name[ii] == intern.status)
                        param = "@status";
                else if (attribute_name[ii] == intern.dispose)
                        param = "@dispose";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_sample (ParserData *data,
                                const gchar **attribute_name,
                                const gchar **attribute_value,
                                GError **error)
{
        sqlite3_stmt *stmt = data->insert_sample_stmt;
        gint ii;

        db_parser_bind_text (stmt, "@game", data->game);

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.name)
                        param = "@name";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
        }
}

static void
db_parser_start_element_sound (ParserData *data,
                               const gchar **attribute_name,
                               const gchar **attribute_value,
                               GError **error)
{
        sqlite3_stmt *stmt = data->insert_game_stmt;
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param;

                if (attribute_name[ii] == intern.channels)
                        param = "@sound_channels";
                else
                        continue;

                db_parser_bind_text (stmt, param, attribute_value[ii]);
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

        /* XXX Copied from below... */

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

        /* Skip unused elements to speed up parsing. */

#if 0
        if (element_name == intern.biosset)
                db_parser_start_element_biosset (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.chip)
                db_parser_start_element_chip (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.control)
                db_parser_start_element_control (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.dipswitch)
                db_parser_start_element_dipswitch (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.dipvalue)
                db_parser_start_element_dipvalue (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.disk)
                db_parser_start_element_disk (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.display)
                db_parser_start_element_display (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.driver)
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

        else if (element_name == intern.rom)
                db_parser_start_element_rom (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.sample)
                db_parser_start_element_sample (
                        data, attribute_name, attribute_value, error);

        else if (element_name == intern.sound)
                db_parser_start_element_sound (
                        data, attribute_name, attribute_value, error);
#endif
}

static void
db_parser_end_element_dipswitch (ParserData *data,
                                 GError **error)
{
        g_free (data->dipswitch);
        data->dipswitch = NULL;
}

static void
db_parser_end_element_game (ParserData *data,
                            GError **error)
{
        if (!db_parser_exec_stmt (data->insert_game_stmt, error))
                return;

        gva_process_inc_progress (data->process);

        g_free (data->game);
        data->game = NULL;
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

        /* XXX Copied from below... */

        if (element_name == intern.game)
                db_parser_end_element_game (data, error);

        /* Skip unused elements to speed up parsing. */

#if 0
        if (element_name == intern.biosset)
                db_parser_exec_stmt (data->insert_biosset_stmt, error);

        else if (element_name == intern.chip)
                db_parser_exec_stmt (data->insert_chip_stmt, error);

        else if (element_name == intern.control)
                db_parser_exec_stmt (data->insert_control_stmt, error);

        else if (element_name == intern.dipswitch)
                db_parser_end_element_dipswitch (data, error);

        else if (element_name == intern.dipvalue)
                db_parser_exec_stmt (data->insert_dipvalue_stmt, error);

        else if (element_name == intern.disk)
                db_parser_exec_stmt (data->insert_disk_stmt, error);

        else if (element_name == intern.display)
                db_parser_exec_stmt (data->insert_display_stmt, error);

        else if (element_name == intern.game)
                db_parser_end_element_game (data, error);

        else if (element_name == intern.rom)
                db_parser_exec_stmt (data->insert_rom_stmt, error);

        else if (element_name == intern.sample)
                db_parser_exec_stmt (data->insert_sample_stmt, error);
#endif
}

static void
db_parser_text (GMarkupParseContext *context,
                const gchar *text,
                gsize text_len,
                gpointer user_data,
                GError **error)
{
        ParserData *data = user_data;
        sqlite3_stmt *stmt = data->insert_game_stmt;
        const gchar *element_name;

        g_assert (data->element_stack_depth > 0);
        element_name = data->element_stack[data->element_stack_depth - 1];

        if (element_name == intern.description)
                db_parser_bind_text (stmt, "@description", text);

        else if (element_name == intern.manufacturer)
                db_parser_bind_text (stmt, "@manufacturer", text);

        else if (element_name == intern.year)
                db_parser_bind_text (stmt, "@year", text);
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
        GError *error = NULL;

        data = g_slice_new0 (ParserData);
        data->context = g_markup_parse_context_new (&parser, 0, data, NULL);
        data->process = g_object_ref (process);

        if (!gva_db_prepare (SQL_INSERT_GAME, &data->insert_game_stmt, &error))
                g_error ("%s", error->message);

        if (!gva_db_prepare (SQL_INSERT_BIOSSET, &data->insert_biosset_stmt, &error))
                g_error ("%s", error->message);

        if (!gva_db_prepare (SQL_INSERT_ROM, &data->insert_rom_stmt, &error))
                g_error ("%s", error->message);

        if (!gva_db_prepare (SQL_INSERT_DISK, &data->insert_disk_stmt, &error))
                g_error ("%s", error->message);

        if (!gva_db_prepare (SQL_INSERT_SAMPLE, &data->insert_sample_stmt, &error))
                g_error ("%s", error->message);

        if (!gva_db_prepare (SQL_INSERT_CHIP, &data->insert_chip_stmt, &error))
                g_error ("%s", error->message);

        if (!gva_db_prepare (SQL_INSERT_DISPLAY, &data->insert_display_stmt, &error))
                g_error ("%s", error->message);

        if (!gva_db_prepare (SQL_INSERT_CONTROL, &data->insert_control_stmt, &error))
                g_error ("%s", error->message);

        if (!gva_db_prepare (SQL_INSERT_DIPVALUE, &data->insert_dipvalue_stmt, &error))
                g_error ("%s", error->message);

        return data;
}

static void
db_parser_data_free (ParserData *data)
{
        g_markup_parse_context_free (data->context);
        g_object_unref (data->process);

        sqlite3_finalize (data->insert_game_stmt);
        sqlite3_finalize (data->insert_biosset_stmt);
        sqlite3_finalize (data->insert_rom_stmt);
        sqlite3_finalize (data->insert_disk_stmt);
        sqlite3_finalize (data->insert_sample_stmt);
        sqlite3_finalize (data->insert_chip_stmt);
        sqlite3_finalize (data->insert_display_stmt);
        sqlite3_finalize (data->insert_control_stmt);
        sqlite3_finalize (data->insert_dipvalue_stmt);

        g_free (data->dipswitch);
        g_free (data->game);

        g_slice_free (ParserData, data);
}

static void
db_parser_read (GvaProcess *process,
                ParserData *data)
{
        gchar *line;

        if (process->error != NULL)
        {
                gva_process_kill (process);
                return;
        }

        line = gva_process_stdout_read_line (process);

        g_markup_parse_context_parse (
                data->context, line, -1, &process->error);

        g_free (line);
}

static void
db_parser_exit (GvaProcess *process,
                gint status,
                ParserData *data)
{
        GError *error = NULL;

        if (process->error == NULL)
        {
                GTimeVal time_elapsed;

                g_markup_parse_context_end_parse (
                        data->context, &process->error);

                gva_db_transaction_commit (&error);
                gva_error_handle (&error);

                gva_process_get_time_elapsed (process, &time_elapsed);

                g_message (
                        "Database built in %ld.%ld seconds.",
                        time_elapsed.tv_sec, time_elapsed.tv_usec / 100000);
        }
        else
        {
                gva_db_transaction_rollback (&error);
                gva_error_handle (&error);
        }

        db_parser_data_free (data);
}

static void
db_verify_read (const gchar *name,
                const gchar *status,
                GHashTable *hash_table)
{
        GString *string;

        /* Build a quoted, comma-separated list of games. */
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

/* Helper for db_verify_exit() */
static void
db_verify_foreach (const gchar *status,
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
db_verify_exit (GvaProcess *process,
                gint status,
                GHashTable *results)
{
        const gchar *column;
        GError *error = NULL;

        if (process->error != NULL)
                return;

        column = g_object_get_data (G_OBJECT (process), "column");
        g_assert (column != NULL);

        gva_db_transaction_begin (&error);
        gva_error_handle (&error);

        g_hash_table_foreach (
                results, (GHFunc) db_verify_foreach, (gpointer) column);

        /* This part only really applies to romsets, but I suppose there's
         * no harm in executing it twice. */
        gva_db_execute (SQL_DELETE_NOT_FOUND, &error);
        gva_error_handle (&error);

        gva_db_transaction_commit (&error);
        gva_error_handle (&error);

        g_hash_table_destroy (results);
}

static void
db_verify_string_free (GString *string)
{
        g_string_free (string, TRUE);
}

static gboolean
db_create_tables (GError **error)
{
        return gva_db_execute (SQL_CREATE_TABLE_MAME, error)
                && gva_db_execute (SQL_CREATE_TABLE_GAME, error)
                && gva_db_execute (SQL_CREATE_TABLE_BIOSSET, error)
                && gva_db_execute (SQL_CREATE_TABLE_ROM, error)
                && gva_db_execute (SQL_CREATE_TABLE_DISK, error)
                && gva_db_execute (SQL_CREATE_TABLE_SAMPLE, error)
                && gva_db_execute (SQL_CREATE_TABLE_CHIP, error)
                && gva_db_execute (SQL_CREATE_TABLE_DISPLAY, error)
                && gva_db_execute (SQL_CREATE_TABLE_CONTROL, error)
                && gva_db_execute (SQL_CREATE_TABLE_DIPVALUE, error);
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
        GvaProcess *process;
        ParserData *data;

        g_return_val_if_fail (db != NULL, NULL);

        /* Initialize the list of canonical names. */
        intern.aspectx      = g_intern_static_string ("aspectx");
        intern.aspecty      = g_intern_static_string ("aspecty");
        intern.bios         = g_intern_static_string ("bios");
        intern.biosset      = g_intern_static_string ("biosset");
        intern.build        = g_intern_static_string ("build");
        intern.buttons      = g_intern_static_string ("buttons");
        intern.channels     = g_intern_static_string ("channels");
        intern.chip         = g_intern_static_string ("chip");
        intern.clock        = g_intern_static_string ("clock");
        intern.cloneof      = g_intern_static_string ("cloneof");
        intern.cocktail     = g_intern_static_string ("cocktail");
        intern.coins        = g_intern_static_string ("coins");
        intern.color        = g_intern_static_string ("color");
        intern.control      = g_intern_static_string ("control");
        intern.crc          = g_intern_static_string ("crc");
        intern.default_     = g_intern_static_string ("default_");
        intern.description  = g_intern_static_string ("description");
        intern.dipswitch    = g_intern_static_string ("dipswitch");
        intern.dipvalue     = g_intern_static_string ("dipvalue");
        intern.disk         = g_intern_static_string ("disk");
        intern.display      = g_intern_static_string ("display");
        intern.dispose      = g_intern_static_string ("dispose");
        intern.driver       = g_intern_static_string ("driver");
        intern.emulation    = g_intern_static_string ("emulation");
        intern.flipx        = g_intern_static_string ("flipx");
        intern.game         = g_intern_static_string ("game");
        intern.graphic      = g_intern_static_string ("graphic");
        intern.height       = g_intern_static_string ("height");
        intern.index_       = g_intern_static_string ("index_");
        intern.input        = g_intern_static_string ("input");
        intern.keydelta     = g_intern_static_string ("keydelta");
        intern.mame         = g_intern_static_string ("mame");
        intern.manufacturer = g_intern_static_string ("manufacturer");
        intern.maximum      = g_intern_static_string ("maximum");
        intern.md5          = g_intern_static_string ("md5");
        intern.merge        = g_intern_static_string ("merge");
        intern.minimum      = g_intern_static_string ("minimum");
        intern.name         = g_intern_static_string ("name");
        intern.offset       = g_intern_static_string ("offset");
        intern.orientation  = g_intern_static_string ("orientation");
        intern.palettesize  = g_intern_static_string ("palettesize");
        intern.players      = g_intern_static_string ("players");
        intern.protection   = g_intern_static_string ("protection");
        intern.refresh      = g_intern_static_string ("refresh");
        intern.region       = g_intern_static_string ("region");
        intern.reverse      = g_intern_static_string ("reverse");
        intern.rom          = g_intern_static_string ("rom");
        intern.romof        = g_intern_static_string ("romof");
        intern.rotate       = g_intern_static_string ("rotate");
        intern.runnable     = g_intern_static_string ("runnable");
        intern.sample       = g_intern_static_string ("sample");
        intern.sampleof     = g_intern_static_string ("sampleof");
        intern.savestate    = g_intern_static_string ("savestate");
        intern.screen       = g_intern_static_string ("screen");
        intern.sensitivity  = g_intern_static_string ("sensitivity");
        intern.service      = g_intern_static_string ("service");
        intern.sha1         = g_intern_static_string ("sha1");
        intern.size         = g_intern_static_string ("size");
        intern.sound        = g_intern_static_string ("sound");
        intern.sourcefile   = g_intern_static_string ("sourcefile");
        intern.status       = g_intern_static_string ("status");
        intern.tilt         = g_intern_static_string ("tilt");
        intern.type         = g_intern_static_string ("type");
        intern.width        = g_intern_static_string ("width");
        intern.year         = g_intern_static_string ("year");

        if (!gva_db_reset (error))
                return NULL;

        process = gva_mame_list_xml (error);
        if (process == NULL)
                return NULL;

        if (!gva_db_transaction_begin (error))
        {
                g_object_unref (process);
                return NULL;
        }

        data = db_parser_data_new (process);

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (db_parser_read), data);

        g_signal_connect (
                process, "exited",
                G_CALLBACK (db_parser_exit), data);

        return process;
}

GvaProcess *
gva_db_verify_romsets (GError **error)
{
        GvaProcess *process;
        GHashTable *results;

        results = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) db_verify_string_free);

        process = gva_mame_verify_roms (
                (GvaMameCallback) db_verify_read, results, error);

        if (process == NULL)
        {
                g_hash_table_destroy (results);
                return NULL;
        }

        g_object_set_data (G_OBJECT (process), "column", "romset");

        g_signal_connect (
                process, "exited",
                G_CALLBACK (db_verify_exit), results);

        return process;
}

GvaProcess *
gva_db_verify_samplesets (GError **error)
{
        GvaProcess *process;
        GHashTable *results;

        results = g_hash_table_new_full (
                g_str_hash, g_str_equal,
                (GDestroyNotify) g_free,
                (GDestroyNotify) db_verify_string_free);

        process = gva_mame_verify_samples (
                (GvaMameCallback) db_verify_read, results, error);

        if (process == NULL)
        {
                g_hash_table_destroy (results);
                return NULL;
        }

        g_object_set_data (G_OBJECT (process), "column", "sampleset");

        g_signal_connect (
                process, "exited",
                G_CALLBACK (db_verify_exit), results);

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
gva_db_transaction_begin (GError **error)
{
        return gva_db_execute ("BEGIN TRANSACTION", error);
}

gboolean
gva_db_transaction_commit (GError **error)
{
        return gva_db_execute ("COMMIT TRANSACTION", error);
}

gboolean
gva_db_transaction_rollback (GError **error)
{
        return gva_db_execute ("ROLLBACK TRANSACTION", error);
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
                        gva_get_user_data_dir (), "games.db", NULL);

        return filename;
}

gboolean
gva_db_needs_rebuilt (void)
{
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
        mame_version = gva_mame_get_version (&error);
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

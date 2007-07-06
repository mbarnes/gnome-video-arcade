#include "gva-parser.h" 

#include <string.h>

#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-xmame.h"

/* Based on MAME's DTD */
#define MAX_ELEMENT_DEPTH 4

#define SQL_INSERT_GAME \
        "INSERT INTO game VALUES (" \
        "@name, @sourcefile, @runnable, @cloneof, @romof, @sampleof, " \
        "@description, @year, @manufacturer, @history, @video_screen, " \
        "@video_orientation, @video_width, @video_height, @video_aspectx, " \
        "@video_aspecty, @video_refresh, @sound_channels, @input_service, " \
        "@input_tilt, @input_players, @input_control, @input_buttons, " \
        "@input_coins, @driver_status, @driver_emulation, @driver_color, " \
        "@driver_sound, @driver_graphic, @driver_cocktail, " \
        "@driver_protection, @driver_savestate, @driver_palettesize);"

typedef struct
{
        GMarkupParseContext *context;
        sqlite3_stmt *insert_game_stmt;

        const gchar *element_stack[MAX_ELEMENT_DEPTH];
        guint element_stack_depth;

} ParserData;

/* Canonical names of elements */
static struct
{
        const gchar *description;
        const gchar *driver;
        const gchar *game;
        const gchar *history;
        const gchar *input;
        const gchar *mame;
        const gchar *manufacturer;
        const gchar *sound;
        const gchar *video;
        const gchar *year;

} intern;

static gboolean
parser_bind_text (ParserData *data,
                  const gchar *param,
                  const gchar *value,
                  GError **error)
{
        gint errcode;
        gint index;

        index = sqlite3_bind_parameter_index (
                data->insert_game_stmt, param);
        if (index == 0)
                g_error ("%s: No such host parameter", param);

        errcode = sqlite3_bind_text (
                data->insert_game_stmt, index,
                sqlite3_mprintf ("%Q", value), -1,
                sqlite3_free);
        if (errcode != SQLITE_OK)
                gva_db_set_error (error, 0, NULL);

        return (errcode == SQLITE_OK);
}

static void
parser_start_element_driver (GMarkupParseContext *context,
                             const gchar **attribute_name,
                             const gchar **attribute_value,
                             ParserData *data,
                             GError **error)
{
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param = NULL;
                const gchar *value = attribute_value[ii];

                if (strcmp (attribute_name[ii], "status") == 0)
                        param = "@driver_status";
                else if (strcmp (attribute_name[ii], "emulation") == 0)
                        param = "@driver_emulation";
                else if (strcmp (attribute_name[ii], "color") == 0)
                        param = "@driver_color";
                else if (strcmp (attribute_name[ii], "sound") == 0)
                        param = "@driver_sound";
                else if (strcmp (attribute_name[ii], "graphic") == 0)
                        param = "@driver_graphic";
                else if (strcmp (attribute_name[ii], "cocktail") == 0)
                        param = "@driver_cocktail";
                else if (strcmp (attribute_name[ii], "protection") == 0)
                        param = "@driver_protection";
                else if (strcmp (attribute_name[ii], "savestate") == 0)
                        param = "@driver_savestate";
                else if (strcmp (attribute_name[ii], "palettesize") == 0)
                        param = "@driver_palettesize";

                if (param != NULL)
                        parser_bind_text (data, param, value, error);
        }
}

static void
parser_start_element_game (GMarkupParseContext *context,
                           const gchar **attribute_name,
                           const gchar **attribute_value,
                           ParserData *data,
                           GError **error)
{
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param = NULL;
                const gchar *value = attribute_value[ii];

                if (strcmp (attribute_name[ii], "name") == 0)
                        param = "@name";
                else if (strcmp (attribute_name[ii], "sourcefile") == 0)
                        param = "@sourcefile";
                else if (strcmp (attribute_name[ii], "runnable") == 0)
                        param = "@runnable";
                else if (strcmp (attribute_name[ii], "cloneof") == 0)
                        param = "@cloneof";
                else if (strcmp (attribute_name[ii], "romof") == 0)
                        param = "@romof";
                else if (strcmp (attribute_name[ii], "sampleof") == 0)
                        param = "@sampleof";

                if (param != NULL)
                        parser_bind_text (data, param, value, error);
        }
}

static void
parser_start_element_input (GMarkupParseContext *context,
                            const gchar **attribute_name,
                            const gchar **attribute_value,
                            ParserData *data,
                            GError **error)
{
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param = NULL;
                const gchar *value = attribute_value[ii];

                if (strcmp (attribute_name[ii], "service") == 0)
                        param = "@input_service";
                else if (strcmp (attribute_name[ii], "tilt") == 0)
                        param = "@input_tilt";
                else if (strcmp (attribute_name[ii], "players") == 0)
                        param = "@input_players";
                else if (strcmp (attribute_name[ii], "control") == 0)
                        param = "@input_control";
                else if (strcmp (attribute_name[ii], "buttons") == 0)
                        param = "@input_buttons";
                else if (strcmp (attribute_name[ii], "coins") == 0)
                        param = "@input_coins";

                if (param != NULL)
                        parser_bind_text (data, param, value, error);
        }
}

static void
parser_start_element_mame (GMarkupParseContext *context,
                           const gchar **attribute_name,
                           const gchar **attribute_value,
                           ParserData *data,
                           GError **error)
{
        const gchar *build = NULL;
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
                if (strcmp (attribute_name[ii], "build") == 0)
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
parser_start_element_sound (GMarkupParseContext *context,
                            const gchar **attribute_name,
                            const gchar **attribute_value,
                            ParserData *data,
                            GError **error)
{
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param = NULL;
                const gchar *value = attribute_value[ii];

                if (strcmp (attribute_name[ii], "channels") == 0)
                        param = "@sound_channels";

                if (param != NULL)
                        parser_bind_text (data, param, value, error);
        }
}

static void
parser_start_element_video (GMarkupParseContext *context,
                            const gchar **attribute_name,
                            const gchar **attribute_value,
                            ParserData *data,
                            GError **error)
{
        gint ii;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                const gchar *param = NULL;
                const gchar *value = attribute_value[ii];

                if (strcmp (attribute_name[ii], "screen") == 0)
                        param = "@video_screen";
                else if (strcmp (attribute_name[ii], "orientation") == 0)
                        param = "@video_orientation";
                else if (strcmp (attribute_name[ii], "width") == 0)
                        param = "@video_width";
                else if (strcmp (attribute_name[ii], "height") == 0)
                        param = "@video_height";
                else if (strcmp (attribute_name[ii], "aspectx") == 0)
                        param = "@video_aspectx";
                else if (strcmp (attribute_name[ii], "aspecty") == 0)
                        param = "@video_aspecty";
                else if (strcmp (attribute_name[ii], "refresh") == 0)
                        param = "@video_refresh";

                if (param != NULL)
                        parser_bind_text (data, param, value, error);
        }
}

static void
parser_start_element (GMarkupParseContext *context,
                      const gchar *element_name,
                      const gchar **attribute_name,
                      const gchar **attribute_value,
                      gpointer user_data,
                      GError **error)
{
        ParserData *data = user_data;
        gint ii;

        element_name = g_intern_string (element_name);
        g_assert (data->element_stack_depth < MAX_ELEMENT_DEPTH);
        data->element_stack[data->element_stack_depth++] = element_name;

        if (element_name == intern.driver)
                parser_start_element_driver (
                        context, attribute_name,
                        attribute_value, data, error);

        else if (element_name == intern.game)
                parser_start_element_game (
                        context, attribute_name,
                        attribute_value, data, error);

        else if (element_name == intern.input)
                parser_start_element_input (
                        context, attribute_name,
                        attribute_value, data, error);

        else if (element_name == intern.mame)
                parser_start_element_mame (
                        context, attribute_name,
                        attribute_value, data, error);

        else if (element_name == intern.sound)
                parser_start_element_sound (
                        context, attribute_name,
                        attribute_value, data, error);

        else if (element_name == intern.video)
                parser_start_element_video (
                        context, attribute_name,
                        attribute_value, data, error);
}

static void
parser_end_element_game (GMarkupParseContext *context,
                         ParserData *data,
                         GError **error)
{
        static gint count = 0;

        g_print ("Processed %d games\r", ++count);

        if (sqlite3_step (data->insert_game_stmt) != SQLITE_DONE)
                gva_db_set_error (error, 0, NULL);

        sqlite3_reset (data->insert_game_stmt);
        sqlite3_clear_bindings (data->insert_game_stmt);
}

static void
parser_end_element (GMarkupParseContext *context,
                    const gchar *element_name,
                    gpointer user_data,
                    GError **error)
{
        ParserData *data = user_data;

        g_assert (data->element_stack_depth > 0);
        element_name = data->element_stack[--data->element_stack_depth];

        if (element_name == intern.game)
                parser_end_element_game (context, data, error);
}

static void
parser_text (GMarkupParseContext *context,
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
                parser_bind_text (data, "@description", text, error);

        else if (element_name == intern.history)
                parser_bind_text (data, "@history", text, error);

        else if (element_name == intern.manufacturer)
                parser_bind_text (data, "@manufacturer", text, error);

        else if (element_name == intern.year)
                parser_bind_text (data, "@year", text, error);
}

static GMarkupParser parser =
{
        parser_start_element,
        parser_end_element,
        parser_text,
        NULL,
        NULL
};

static ParserData *
parser_data_new (void)
{
        ParserData *data;
        GError *error = NULL;

        data = g_slice_new0 (ParserData);
        data->context = g_markup_parse_context_new (&parser, 0, data, NULL);
        gva_db_prepare (SQL_INSERT_GAME, &data->insert_game_stmt, &error);

        if (error != NULL)
                g_error ("%s", error->message);

        return data;
}

static void
parser_data_free (ParserData *data)
{
        g_markup_parse_context_free (data->context);
        sqlite3_finalize (data->insert_game_stmt);
        g_slice_free (ParserData, data);
}

static void
parser_read (GvaProcess *process,
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
parser_exit (GvaProcess *process,
             gint status,
             ParserData *data)
{
        GTimeVal time_elapsed;

        if (process->error == NULL)
                g_markup_parse_context_end_parse (
                        data->context, &process->error);

        gva_process_get_time_elapsed (process, &time_elapsed);

        g_message (
                "XML parsing completed in %d.%d seconds",
                time_elapsed.tv_sec, time_elapsed.tv_usec / 100000);

        parser_data_free (data);
}

GvaProcess *
gva_parse_game_data (GError **error)
{
        GvaProcess *process;
        ParserData *data;

        /* Initialize the list of canonical names. */
        intern.description  = g_intern_static_string ("description");
        intern.driver       = g_intern_static_string ("driver");
        intern.game         = g_intern_static_string ("game");
        intern.history      = g_intern_static_string ("history");
        intern.input        = g_intern_static_string ("input");
        intern.mame         = g_intern_static_string ("mame");
        intern.manufacturer = g_intern_static_string ("manufacturer");
        intern.sound        = g_intern_static_string ("sound");
        intern.video        = g_intern_static_string ("video");
        intern.year         = g_intern_static_string ("year");

        process = gva_xmame_list_xml (error);
        if (process == NULL)
                return NULL;

        data = parser_data_new ();

        g_signal_connect (
                process, "stdout-ready",
                G_CALLBACK (parser_read), data);

        g_signal_connect (
                process, "exited",
                G_CALLBACK (parser_exit), data);

        return process;
}

#include "gva-parser.h" 

#include <string.h>

#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-xmame.h"

typedef struct
{
        GMarkupParseContext *context;
        GtkTreeModel *model;
        GtkTreeIter iter;
        gboolean iter_set;

} ParserData;

/* Canonical names of elements and attributes */
static struct
{
        const gchar *cloneof;
        const gchar *description;
        const gchar *game;
        const gchar *history;
        const gchar *mame;
        const gchar *manufacturer;
        const gchar *name;
        const gchar *romof;
        const gchar *runnable;
        const gchar *sampleof;
        const gchar *sourcefile;
        const gchar *year;

} intern;

static void
parser_error_missing_required_attribute (GMarkupParseContext *context,
                                         const gchar *attribute_name,
                                         GError **error)
{
        const gchar *element_name;
        gint line_number;

        element_name = g_markup_parse_context_get_element (context);
        g_markup_parse_context_get_position (context, &line_number, NULL);

        g_set_error (
                error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                _("<%s> element at line %d missing required attribute \"%s\""),
                element_name, line_number, attribute_name);
}

static const gchar **
parser_intern_attribute_names (const gchar **attribute_name)
{
        const gchar **interned;
        guint length, ii;

        length = g_strv_length ((gchar **) attribute_name);
        interned = g_new0 (const gchar *, length + 1);
        for (ii = 0; ii < length; ii++)
                interned[ii] = g_intern_string (attribute_name[ii]);

        return interned;
}

static void
parser_start_element_game (GMarkupParseContext *context,
                           const gchar **attribute_name,
                           const gchar **attribute_value,
                           ParserData *data,
                           GError **error)
{
        GtkTreePath *path;
        gint ii;

        const gchar *name = NULL;
        const gchar *sourcefile = NULL;
        const gchar *runnable = "yes";
        const gchar *cloneof = NULL;
        const gchar *romof = NULL;
        const gchar *sampleof = NULL;

        g_assert (!data->iter_set);

        for (ii = 0; attribute_name[ii] != NULL; ii++)
        {
                if (attribute_name[ii] == intern.name)
                        name = attribute_value[ii];
                else if (attribute_name[ii] == intern.sourcefile)
                        sourcefile = attribute_value[ii];
                else if (attribute_name[ii] == intern.runnable)
                        runnable = attribute_value[ii];
                else if (attribute_name[ii] == intern.cloneof)
                        cloneof = attribute_value[ii];
                else if (attribute_name[ii] == intern.romof)
                        romof = attribute_value[ii];
                else if (attribute_name[ii] == intern.sampleof)
                        sampleof = attribute_value[ii];
        }

        if (name == NULL)
        {
                parser_error_missing_required_attribute (
                        context, "name", error);
                return;
        }

        path = gva_game_db_lookup (name);
        if (path == NULL)
                return;
        data->iter_set = gtk_tree_model_get_iter (
                data->model, &data->iter, path);
        g_assert (data->iter_set);
        gtk_tree_path_free (path);

        gtk_list_store_set (
                GTK_LIST_STORE (data->model), &data->iter,
                GVA_GAME_STORE_COLUMN_NAME, name,
                GVA_GAME_STORE_COLUMN_SOURCEFILE, sourcefile,
                GVA_GAME_STORE_COLUMN_RUNNABLE, strcmp (runnable, "yes") == 0,
                GVA_GAME_STORE_COLUMN_CLONEOF, cloneof,
                GVA_GAME_STORE_COLUMN_ROMOF, romof,
                GVA_GAME_STORE_COLUMN_SAMPLEOF, sampleof, -1);
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
        attribute_name = parser_intern_attribute_names (attribute_name);

        /* Check these in decreasing order of likelihood. */

        if (element_name == intern.game)
                parser_start_element_game (
                        context, attribute_name,
                        attribute_value, data, error);

        g_free (attribute_name);
}

static void
parser_end_element_game (GMarkupParseContext *context,
                         ParserData *data,
                         GError **error)
{
        data->iter_set = FALSE;
}

static void
parser_end_element (GMarkupParseContext *context,
                    const gchar *element_name,
                    gpointer user_data,
                    GError **error)
{
        ParserData *data = user_data;

        element_name = g_intern_string (element_name);

        /* Check these in decreasing order of likelihood. */

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

        if (!data->iter_set)
                return;

        element_name = g_markup_parse_context_get_element (context);
        element_name = g_intern_string (element_name);

        if (element_name == intern.description)
                gtk_list_store_set (
                        GTK_LIST_STORE (data->model), &data->iter,
                        GVA_GAME_STORE_COLUMN_DESCRIPTION, text, -1);

        else if (element_name == intern.year)
                gtk_list_store_set (
                        GTK_LIST_STORE (data->model), &data->iter,
                        GVA_GAME_STORE_COLUMN_YEAR, text, -1);

        else if (element_name == intern.manufacturer)
                gtk_list_store_set (
                        GTK_LIST_STORE (data->model), &data->iter,
                        GVA_GAME_STORE_COLUMN_MANUFACTURER, text, -1);

        else if (element_name == intern.history)
                gtk_list_store_set (
                        GTK_LIST_STORE (data->model), &data->iter,
                        GVA_GAME_STORE_COLUMN_HISTORY, text, -1);
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

        data = g_slice_new (ParserData);
        data->context = g_markup_parse_context_new (&parser, 0, data, NULL);
        data->model = gva_game_db_get_model ();

        return data;
}

static void
parser_data_free (ParserData *data)
{
        g_markup_parse_context_free (data->context);
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
        intern.cloneof      = g_intern_static_string ("cloneof");
        intern.description  = g_intern_static_string ("description");
        intern.game         = g_intern_static_string ("game");
        intern.history      = g_intern_static_string ("history");
        intern.mame         = g_intern_static_string ("mame");
        intern.manufacturer = g_intern_static_string ("manufacturer");
        intern.name         = g_intern_static_string ("name");
        intern.romof        = g_intern_static_string ("romof");
        intern.runnable     = g_intern_static_string ("runnable");
        intern.sampleof     = g_intern_static_string ("sampleof");
        intern.sourcefile   = g_intern_static_string ("sourcefile");
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

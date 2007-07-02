#include "gva-parser.h" 

#include <string.h>

#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-xmame.h"

typedef struct
{
        GMarkupParseContext *context;
        GtkTreeModel *model;
        GtkTreePath *path;

} ParserData;

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

/* Convenience Macro */
#define LOOKUP_ATTRIBUTE(name, def) \
        parser_attribute_lookup (name, attribute_name, attribute_value, def)

static const gchar *
parser_attribute_lookup (const gchar *lookup_name,
                         const gchar **attribute_name,
                         const gchar **attribute_value,
                         const gchar *default_value)
{
        gint ii;

        if (attribute_name == NULL || attribute_value == NULL)
                return NULL;

        for (ii = 0; attribute_name[ii] != NULL; ii++)
                if (strcmp (lookup_name, attribute_name[ii]) == 0)
                        return attribute_value[ii];

        return default_value;
}

static void
parser_start_element_game (GMarkupParseContext *context,
                           const gchar **attribute_name,
                           const gchar **attribute_value,
                           ParserData *data,
                           GError **error)
{
        GtkTreeIter iter;
        gboolean valid;

        const gchar *name;
        const gchar *sourcefile;
        const gchar *runnable;
        const gchar *cloneof;
        const gchar *romof;
        const gchar *sampleof;

        g_assert (data->path == NULL);

        name = LOOKUP_ATTRIBUTE ("name", NULL);
        sourcefile = LOOKUP_ATTRIBUTE ("sourcefile", NULL);
        runnable = LOOKUP_ATTRIBUTE ("runnable", "yes");
        romof = LOOKUP_ATTRIBUTE ("romof", NULL);
        sampleof = LOOKUP_ATTRIBUTE ("sampleof", NULL);

        if (name == NULL)
        {
                parser_error_missing_required_attribute (
                        context, "name", error);
                return;
        }

        data->path = gva_game_db_lookup (name);
        if (data->path == NULL)
                return;

        valid = gtk_tree_model_get_iter (data->model, &iter, data->path);
        g_assert (valid);

        gtk_list_store_set (
                GTK_LIST_STORE (data->model), &iter,
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

        /* Check these in decreasing order of likelihood. */

        if (strcmp (element_name, "game") == 0)
                parser_start_element_game (
                        context, attribute_name,
                        attribute_value, data, error);
}

static void
parser_end_element_game (GMarkupParseContext *context,
                         ParserData *data,
                         GError **error)
{
        if (data->path != NULL)
        {
                gtk_tree_path_free (data->path);
                data->path = NULL;
        }
}

static void
parser_end_element (GMarkupParseContext *context,
                    const gchar *element_name,
                    gpointer user_data,
                    GError **error)
{
        ParserData *data = user_data;

        /* Check these in decreasing order of likelihood. */

        if (strcmp (element_name, "game") == 0)
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
        GtkTreeIter iter;
        gboolean valid;

        if (data->path == NULL)
                return;

        element_name = g_markup_parse_context_get_element (context);
        valid = gtk_tree_model_get_iter (data->model, &iter, data->path);
        g_assert (valid);

        if (strcmp (element_name, "description") == 0)
                gtk_list_store_set (
                        GTK_LIST_STORE (data->model), &iter,
                        GVA_GAME_STORE_COLUMN_DESCRIPTION, text, -1);

        if (strcmp (element_name, "year") == 0)
                gtk_list_store_set (
                        GTK_LIST_STORE (data->model), &iter,
                        GVA_GAME_STORE_COLUMN_YEAR, text, -1);

        if (strcmp (element_name, "manufacturer") == 0)
                gtk_list_store_set (
                        GTK_LIST_STORE (data->model), &iter,
                        GVA_GAME_STORE_COLUMN_MANUFACTURER, text, -1);

        if (strcmp (element_name, "history") == 0)
                gtk_list_store_set (
                        GTK_LIST_STORE (data->model), &iter,
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
        data->path = NULL;

        return data;
}

static void
parser_data_free (ParserData *data)
{
        g_markup_parse_context_free (data->context);
        if (data->path != NULL)
                gtk_tree_path_free (data->path);
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
        if (process->error == NULL)
                g_markup_parse_context_end_parse (
                        data->context, &process->error);

        parser_data_free (data);
}

GvaProcess *
gva_parse_game_data (GError **error)
{
        GvaProcess *process;
        ParserData *data;

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

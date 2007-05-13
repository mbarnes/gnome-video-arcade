#include "gva-models.h"

#include <gconf/gconf-client.h>
#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-ui.h"

#define GCONF_MODEL_KEY         GVA_GCONF_PREFIX "/model"

static gboolean initialized = FALSE;
static GtkTreeModel *models[GVA_NUM_MODELS];
static gint current_model = 0;

static void
models_init (void)
{
        GvaModelType type;
        GtkTreeModel *base_model;

        base_model = gva_game_db_get_model ();

        type = GVA_MODEL_AVAILABLE;
        models[type] = g_object_ref (base_model);

        type = GVA_MODEL_FAVORITES;
        models[type] = gtk_tree_model_filter_new (base_model, NULL);
        gtk_tree_model_filter_set_visible_column (
                GTK_TREE_MODEL_FILTER (models[type]),
                GVA_GAME_STORE_COLUMN_FAVORITE);

        type = GVA_MODEL_RESULTS;
        models[type] = NULL;  /* TODO */

        initialized = TRUE;
}

GvaModelType
gva_models_get_current_model (void)
{
        GConfClient *client;
        GvaModelType current;
        GError *error = NULL;

        if (G_UNLIKELY (!initialized))
                models_init ();

        client = gconf_client_get_default ();
        current = gconf_client_get_int (client, GCONF_MODEL_KEY, &error);
        g_object_unref (client);

        if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }

        return current;
}

void
gva_models_set_current_model (GvaModelType model)
{
        GConfClient *client;
        GError *error = NULL;

        g_return_if_fail (CLAMP (model, 0, GVA_NUM_MODELS) == model);

        if (G_UNLIKELY (!initialized))
                models_init ();

        client = gconf_client_get_default ();
        gconf_client_set_int (client, GCONF_MODEL_KEY, model, &error);
        g_object_unref (client);

        if (error == NULL)
        {
                GtkTreeView *view;

                view = GTK_TREE_VIEW (GVA_WIDGET_MAIN_TREE_VIEW);
                gtk_tree_view_set_model (view, models[model]);
        }
        else
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }
}

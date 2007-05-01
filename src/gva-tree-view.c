#include "gva-tree-view.h"

#include "gva-game-store.h"

static void
tree_view_select_default (GtkTreeView *tree_view)
{
        GtkTreePath *tree_path = NULL;
        GConfClient *client;
        gchar *romname;
        GError *error = NULL;

        client = gconf_client_get_default ();

        romname = gconf_client_get_string (
                client, GVA_GCONF_PREFIX "/selected", &error);

        if (romname != NULL)
        {
                GtkTreeModel *tree_model;
                GtkTreeRowReference *tree_row_reference;

                tree_model = gtk_tree_model_sort_get_model (
                        GTK_TREE_MODEL_SORT (
                        gtk_tree_view_get_model (tree_view)));
                tree_row_reference = gva_game_store_lookup (
                        GVA_GAME_STORE (tree_model), romname);
                if (tree_row_reference != NULL)
                        tree_path = gtk_tree_row_reference_get_path (
                                tree_row_reference);
                else
                        g_warning ("Failed to find ROM '%s'", romname);
                g_free (romname);
        }
        else if (error != NULL)
        {
                g_warning ("%s", error->message);
                g_error_free (error);
        }

        if (tree_path == NULL)
                tree_path = gtk_tree_path_new_first ();

        gtk_tree_selection_select_path (
                gtk_tree_view_get_selection (tree_view), tree_path);
        gtk_tree_view_scroll_to_cell (
                tree_view, tree_path, NULL, TRUE, 0.5, 0.0);

        gtk_tree_path_free (tree_path);

        g_object_unref (client);
}

static void
tree_view_selection_changed_cb (GtkTreeSelection *tree_selection)
{
        GtkTreeModel *tree_model;
        GtkTreeIter tree_iter;
        gboolean selected;

        selected = gtk_tree_selection_get_selected (
                tree_selection, &tree_model, &tree_iter);

        if (selected)
        {
                GConfClient *client;
                gchar *romname;
                GError *error = NULL;

                gtk_tree_model_get (
                        tree_model, &tree_iter,
                        GVA_GAME_STORE_COLUMN_ROMNAME, &romname, -1);

                client = gconf_client_get_default ();
                gconf_client_set_string (
                        client, GVA_GCONF_PREFIX "/selected",
                        romname, &error);
                g_object_unref (client);

                if (error != NULL)
                {
                        g_warning ("%s", error->message);
                        g_error_free (error);
                }

                g_free (romname);
        }
}

void
gva_tree_view_init (GtkTreeView *tree_view, GvaGameStore *game_store)
{
        GtkTreeModel *sort_model;
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;

        g_return_if_fail (tree_view != NULL);
        g_return_if_fail (game_store != NULL);

        sort_model = gtk_tree_model_sort_new_with_model (
                GTK_TREE_MODEL (game_store));
        gtk_tree_view_set_model (tree_view, sort_model);
        g_object_unref (sort_model);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (
                _("Title"), renderer,
                "text", GVA_GAME_STORE_COLUMN_TITLE,
                NULL);
        gtk_tree_view_column_set_sort_column_id (
                column, GVA_GAME_STORE_COLUMN_TITLE);
        gtk_tree_view_append_column (tree_view, column);

        gtk_tree_view_set_search_column (
                tree_view, GVA_GAME_STORE_COLUMN_TITLE);
        gtk_tree_view_set_rules_hint (tree_view, TRUE);

        g_signal_connect (
                gtk_tree_view_get_selection (tree_view), "changed",
                G_CALLBACK (tree_view_selection_changed_cb), NULL);

        tree_view_select_default (tree_view);
}

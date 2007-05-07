#include "gva-play-back.h"

#include <errno.h>
#include <langinfo.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "gva-game-db.h"
#include "gva-game-store.h"
#include "gva-ui.h"
#include "gva-xmame.h"

static void
play_back_selection_changed_cb (GtkTreeSelection *tree_selection)
{
        gint selected_rows;

        selected_rows =
                gtk_tree_selection_count_selected_rows (tree_selection);
        gtk_widget_set_sensitive (
                GVA_WIDGET_PLAY_BACK_BUTTON, (selected_rows == 1));
        gtk_widget_set_sensitive (
                GVA_WIDGET_PLAY_BACK_DELETE_BUTTON, (selected_rows >= 1));
}

static void
play_back_window_init (void)
{
        static gboolean initialized = FALSE;

        GtkWindow *window;
        GtkTreeView *tree_view;
        GtkCellRenderer *cell_renderer;
        GtkTreeViewColumn *tree_view_column;

        if (initialized)
                return;

        window = GTK_WINDOW (GVA_WIDGET_PLAY_BACK_WINDOW);
        tree_view = GTK_TREE_VIEW (GVA_WIDGET_PLAY_BACK_TREE_VIEW);

        gtk_tree_selection_set_mode (
                gtk_tree_view_get_selection (tree_view),
                GTK_SELECTION_MULTIPLE);

        cell_renderer = gtk_cell_renderer_text_new ();
        tree_view_column = gtk_tree_view_column_new_with_attributes (
                _("Played On"), cell_renderer,
                "text", GVA_GAME_STORE_COLUMN_DATE_TIME,
                NULL);
        gtk_tree_view_append_column (tree_view, tree_view_column);

        cell_renderer = gtk_cell_renderer_text_new ();
        tree_view_column = gtk_tree_view_column_new_with_attributes (
                _("Title"), cell_renderer,
                "text", GVA_GAME_STORE_COLUMN_TITLE,
                NULL);
        gtk_tree_view_append_column (tree_view, tree_view_column);

        g_signal_connect (
                window, "delete_event",
                G_CALLBACK (gtk_widget_hide_on_delete), NULL);
        g_signal_connect (
                gtk_tree_view_get_selection (tree_view), "changed",
                G_CALLBACK (play_back_selection_changed_cb), NULL);
        g_signal_connect_swapped (
                GVA_WIDGET_PLAY_BACK_CLOSE_BUTTON, "clicked",
                G_CALLBACK (gtk_widget_hide), window);

        initialized = TRUE;
}

static void
play_back_add_input_file (gchar *inpfile, gchar *romname,
                          GtkTreeModel *tree_model)
{
        GtkTreePath *path;
        GtkTreeIter iter;
        gchar *inpname;
        gchar *title;
        gchar date_time[256];
        struct stat statbuf;

        if (g_stat (inpfile, &statbuf) != 0)
        {
                g_warning ("%s: %s", inpfile, g_strerror (errno));
                return;
        }

        strftime (
                date_time, sizeof (date_time),
                nl_langinfo (D_T_FMT), localtime (&statbuf.st_ctime));

        path = gva_game_db_lookup (romname);
        if (path != NULL)
        {
                gboolean iter_was_set;

                iter_was_set = gtk_tree_model_get_iter (
                        gva_game_db_get_model (), &iter, path);
                g_assert (iter_was_set);

                gtk_tree_model_get (
                        gva_game_db_get_model (), &iter,
                        GVA_GAME_STORE_COLUMN_TITLE, &title, -1);

                gtk_tree_path_free (path);
        }
        else
        {
                g_warning ("%s: Game '%s' not found", inpfile, romname);
                return;
        }

        inpname = g_path_get_basename (inpfile);

        gtk_list_store_append (GTK_LIST_STORE (tree_model), &iter);

        gtk_list_store_set (
                GTK_LIST_STORE (tree_model), &iter,
                GVA_GAME_STORE_COLUMN_INPNAME, inpname,
                GVA_GAME_STORE_COLUMN_ROMNAME, romname,
                GVA_GAME_STORE_COLUMN_TITLE, title,
                GVA_GAME_STORE_COLUMN_DATE_TIME, date_time,
                -1);

        g_free (inpname);
        g_free (title);
}

static GtkTreeModel *
play_back_load_recorded_games (GError **error)
{
        GtkTreeModel *tree_model;
        GHashTable *hash_table;

        hash_table = gva_xmame_get_input_files (error);
        if (hash_table == NULL)
                return NULL;

        tree_model = gva_game_store_new ();

        g_hash_table_foreach (
                hash_table, (GHFunc) play_back_add_input_file, tree_model);

        g_hash_table_destroy (hash_table);

        return tree_model;
}

gboolean
gva_play_back_run_dialog (GError **error)
{
        GtkTreeModel *tree_model;
        GtkTreeModel *sort_model;
        GtkTreeView *tree_view;

        play_back_window_init ();

        tree_model = play_back_load_recorded_games (error);
        if (tree_model == NULL)
                return FALSE;

        sort_model = gtk_tree_model_sort_new_with_model (tree_model);

        tree_view = GTK_TREE_VIEW (GVA_WIDGET_PLAY_BACK_TREE_VIEW);

        gtk_tree_view_set_model (tree_view, sort_model);

        gtk_widget_show (GVA_WIDGET_PLAY_BACK_WINDOW);

        return TRUE;
}

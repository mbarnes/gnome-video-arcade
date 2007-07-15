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

#include "gva-main.h"

#include <stdarg.h>

#include "gva-db.h"
#include "gva-tree-view.h"
#include "gva-ui.h"

static guint menu_tooltip_cid;

static void
main_build_database_exited_cb (GvaProcess *process,
                               gint status,
                               gpointer user_data)
{
        guint context_id = GPOINTER_TO_UINT (user_data);

        gva_main_statusbar_pop (context_id);
        gtk_widget_hide (GVA_WIDGET_MAIN_PROGRESS_BAR);
}

static void
main_build_database_progress_cb (GvaProcess *process,
                                 GParamSpec *pspec,
                                 gpointer user_data)
{
        guint total_supported = GPOINTER_TO_UINT (user_data);
        GtkProgressBar *progress_bar;
        gdouble fraction = 0.0;

        if (total_supported != 0)
        {
                guint progress;

                progress = gva_process_get_progress (process);
                fraction = (gdouble) progress / (gdouble) total_supported;
        }

        progress_bar = GTK_PROGRESS_BAR (GVA_WIDGET_MAIN_PROGRESS_BAR);
        gtk_progress_bar_set_fraction (progress_bar, fraction);
}

static void
main_menu_item_select_cb (GtkItem *item, GtkAction *action)
{
        gchar *tooltip;

        g_object_get (G_OBJECT (action), "tooltip", &tooltip, NULL);
        if (tooltip != NULL)
                gva_main_statusbar_push (menu_tooltip_cid, "%s", tooltip);
        g_free (tooltip);
}

static void
main_menu_item_deselect_cb (GtkItem *item)
{
        gva_main_statusbar_pop (menu_tooltip_cid);
}

static void
main_window_destroy_cb (GtkObject *object)
{
        gtk_action_activate (GVA_ACTION_QUIT);
}

void
gva_main_init (void)
{
        gva_tree_view_init ();

        gtk_box_pack_start (
                GTK_BOX (GVA_WIDGET_MAIN_VBOX),
                gva_ui_get_managed_widget ("/main-menu"),
                FALSE, FALSE, 0);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_AVAILABLE,
                GVA_WIDGET_MAIN_VIEW_BUTTON_0);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_FAVORITES,
                GVA_WIDGET_MAIN_VIEW_BUTTON_1);

        gtk_action_connect_proxy (
                GVA_ACTION_VIEW_RESULTS,
                GVA_WIDGET_MAIN_VIEW_BUTTON_2);

        gtk_action_connect_proxy (
                GVA_ACTION_PROPERTIES,
                GVA_WIDGET_MAIN_PROPERTIES_BUTTON);

        gtk_action_connect_proxy (
                GVA_ACTION_START,
                GVA_WIDGET_MAIN_START_GAME_BUTTON);

        g_signal_connect (
                GVA_WIDGET_MAIN_WINDOW, "destroy",
                G_CALLBACK (main_window_destroy_cb), NULL);

        gtk_action_set_sensitive (GVA_ACTION_PROPERTIES, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_RECORD, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_START, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_VIEW_AVAILABLE, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_VIEW_FAVORITES, FALSE);
        gtk_action_set_sensitive (GVA_ACTION_VIEW_RESULTS, FALSE);

        gtk_action_set_visible (GVA_ACTION_INSERT_FAVORITE, FALSE);
        gtk_action_set_visible (GVA_ACTION_REMOVE_FAVORITE, FALSE);
}

GvaProcess *
gva_main_build_database (GError **error)
{
        GvaProcess *process;
        guint context_id;
        guint total_supported;

        process = gva_db_build (error);
        if (process == NULL)
                return FALSE;

        context_id = gva_main_statusbar_get_context_id (G_STRFUNC);
        total_supported = gva_xmame_get_total_supported (NULL);

        gva_main_statusbar_push (context_id, _("Building game database..."));
        gtk_widget_show (GVA_WIDGET_MAIN_PROGRESS_BAR);

        g_signal_connect (
                process, "exited",
                G_CALLBACK (main_build_database_exited_cb),
                GUINT_TO_POINTER (context_id));

        g_signal_connect (
                process, "notify::progress",
                G_CALLBACK (main_build_database_progress_cb),
                GUINT_TO_POINTER (total_supported));

        return process;
}

void
gva_main_connect_proxy_cb (GtkUIManager *manager,
                           GtkAction *action,
                           GtkWidget *proxy)
{
        /* Show GtkMenuItem tooltips in the statusbar. */
        if (GTK_IS_MENU_ITEM (proxy))
        {
                GtkStatusbar *statusbar;

                statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

                g_signal_connect (
                        proxy, "select",
                        G_CALLBACK (main_menu_item_select_cb), action);
                g_signal_connect (
                        proxy, "deselect",
                        G_CALLBACK (main_menu_item_deselect_cb), NULL);

                menu_tooltip_cid =
                        gva_main_statusbar_get_context_id (G_STRFUNC);
        }
}

guint
gva_main_statusbar_get_context_id (const gchar *context)
{
        GtkStatusbar *statusbar;

        g_return_val_if_fail (context != NULL, 0);

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

        return gtk_statusbar_get_context_id (statusbar, context);
}

guint
gva_main_statusbar_push (guint context_id,
                         const gchar *format,
                         ...)
{
        GtkStatusbar *statusbar;
        guint message_id;
        va_list args;
        gchar *text;

        g_return_val_if_fail (format != NULL, 0);

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

        va_start (args, format);
        text = g_strdup_vprintf (format, args);
        message_id = gtk_statusbar_push (statusbar, context_id, text);
        g_free (text);
        va_end (args);

        return message_id;
}

void
gva_main_statusbar_pop (guint context_id)
{
        GtkStatusbar *statusbar;

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

        gtk_statusbar_pop (statusbar, context_id);
}

void
gva_main_statusbar_remove (guint context_id,
                           guint message_id)
{
        GtkStatusbar *statusbar;

        statusbar = GTK_STATUSBAR (GVA_WIDGET_MAIN_STATUSBAR);

        gtk_statusbar_remove (statusbar, context_id, message_id);
}

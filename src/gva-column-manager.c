/* Copyright 2007-2011 Matthew Barnes
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

#include "gva-column-manager.h"

#define GVA_COLUMN_MANAGER_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE \
        ((obj), GVA_TYPE_COLUMN_MANAGER, GvaColumnManagerPrivate))

enum
{
        PROP_0,
        PROP_MANAGED_VIEW
};

struct _GvaColumnManagerPrivate
{
        GtkTreeView *managed_view;

        GtkTreeRowReference *move_reference;

        GtkTreeView *tree_view;
        GtkWidget *move_up_button;
        GtkWidget *move_down_button;
        GtkWidget *show_button;
        GtkWidget *hide_button;
};

static gpointer parent_class = NULL;

static void
column_manager_move_selected_up (GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv = manager->priv;
        GtkTreeSelection *selection;
        GtkTreeViewColumn *column;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        GList *list;
        gint index;

        selection = gtk_tree_view_get_selection (priv->tree_view);
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        gtk_tree_model_get (model, &iter, 0, &column, -1);
        list = gtk_tree_view_get_columns (priv->managed_view);

        index = g_list_index (list, column);
        g_assert (index >= 0);

        gtk_tree_view_move_column_after (
                priv->managed_view, column, (index >= 2) ?
                g_list_nth_data (list, index - 2) : NULL);

        path = gtk_tree_path_new_from_indices (index - 1, -1);
        gtk_tree_view_set_cursor (priv->tree_view, path, NULL, FALSE);
        gtk_tree_path_free (path);

        g_object_unref (column);
        g_list_free (list);
}

static void
column_manager_move_selected_down (GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv = manager->priv;
        GtkTreeSelection *selection;
        GtkTreeViewColumn *column;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        GList *list;
        gint index;

        selection = gtk_tree_view_get_selection (priv->tree_view);
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        gtk_tree_model_get (model, &iter, 0, &column, -1);
        list = gtk_tree_view_get_columns (priv->managed_view);

        index = g_list_index (list, column);
        g_assert (index >= 0);

        gtk_tree_view_move_column_after (
                priv->managed_view, column,
                g_list_nth_data (list, index + 1));

        path = gtk_tree_path_new_from_indices (index + 1, -1);
        gtk_tree_view_set_cursor (priv->tree_view, path, NULL, FALSE);
        gtk_tree_path_free (path);

        g_object_unref (column);
        g_list_free (list);
}

static void
column_manager_hide_selected (GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv = manager->priv;
        GtkTreeSelection *selection;
        GtkTreeViewColumn *column;
        GtkTreeModel *model;
        GtkTreeIter iter;

        selection = gtk_tree_view_get_selection (priv->tree_view);
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        /* Hide the selected column. */
        gtk_tree_model_get (model, &iter, 0, &column, -1);
        gtk_tree_view_column_set_visible (column, FALSE);
        g_object_unref (column);
}

static void
column_manager_show_selected (GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv = manager->priv;
        GtkTreeSelection *selection;
        GtkTreeViewColumn *column;
        GtkTreeModel *model;
        GtkTreeIter iter;

        selection = gtk_tree_view_get_selection (priv->tree_view);
        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        /* Show the selected column. */
        gtk_tree_model_get (model, &iter, 0, &column, -1);
        gtk_tree_view_column_set_visible (column, TRUE);
        g_object_unref (column);
}

static void
column_manager_render_title (GtkTreeViewColumn *column,
                             GtkCellRenderer *renderer,
                             GtkTreeModel *model,
                             GtkTreeIter *iter)
{
        GtkTreeViewColumn *managed_column;
        const gchar *title;

        gtk_tree_model_get (model, iter, 0, &managed_column, -1);
        title = gtk_tree_view_column_get_title (managed_column);
        g_object_set (renderer, "text", title, NULL);
        g_object_unref (managed_column);
}

static void
column_manager_render_visible (GtkTreeViewColumn *column,
                               GtkCellRenderer *renderer,
                               GtkTreeModel *model,
                               GtkTreeIter *iter)
{
        GtkTreeViewColumn *managed_column;
        gboolean visible;

        gtk_tree_model_get (model, iter, 0, &managed_column, -1);
        visible = gtk_tree_view_column_get_visible (managed_column);
        g_object_set (renderer, "active", visible, NULL);
        g_object_unref (managed_column);
}

static void
column_manager_row_changed_cb (GtkTreeModel *model,
                               GtkTreePath *path,
                               GtkTreeIter *iter,
                               GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv = manager->priv;
        GtkTreeSelection *selection;
        GtkTreeViewColumn *column;
        gboolean visible;

        /* If the changed row is not selected, nothing to do. */
        selection = gtk_tree_view_get_selection (priv->tree_view);
        if (!gtk_tree_selection_iter_is_selected (selection, iter))
                return;

        /* Get the visible state of the selected row. */
        gtk_tree_model_get (model, iter, 0, &column, -1);
        visible = gtk_tree_view_column_get_visible (column);
        g_object_unref (column);

        /* Update show/hide button sensitivity. */
        gtk_widget_set_sensitive (priv->show_button, !visible);
        gtk_widget_set_sensitive (priv->hide_button, visible);
}

static void
column_manager_row_inserted_cb (GtkTreeModel *model,
                                GtkTreePath *path,
                                GtkTreeIter *iter,
                                GvaColumnManager *manager)
{
        /* First phase of drag-and-drop reordering. */

        GvaColumnManagerPrivate *priv = manager->priv;

        g_assert (priv->move_reference == NULL);

        /* The new row is still empty.  Bookmark it and listen for
         * the "row-deleted" signal; it should be populated by then. */
        priv->move_reference = gtk_tree_row_reference_new (model, path);
}

static void
column_manager_row_deleted_cb (GtkTreeModel *model,
                               GtkTreePath *path,
                               GvaColumnManager *manager)
{
        /* Second phase of drag-and-drop reordering. */

        GvaColumnManagerPrivate *priv = manager->priv;
        GtkTreeViewColumn *column;
        GtkTreeView *view;
        GtkTreeIter iter;
        gboolean valid;

        g_assert (priv->move_reference != NULL);

        view = GTK_TREE_VIEW (priv->managed_view);

        /* Use the path that we bookmarked in phase one. */
        path = gtk_tree_row_reference_get_path (priv->move_reference);

        valid = gtk_tree_model_get_iter (model, &iter, path);
        g_assert (valid);
        gtk_tree_model_get (model, &iter, 0, &column, -1);

        if (gtk_tree_path_prev (path))
        {
                GtkTreeViewColumn *base_column;

                valid = gtk_tree_model_get_iter (model, &iter, path);
                g_assert (valid);
                gtk_tree_model_get (model, &iter, 0, &base_column, -1);
                gtk_tree_view_move_column_after (view, column, base_column);
                g_object_unref (base_column);
        }
        else
                gtk_tree_view_move_column_after (view, column, NULL);

        gtk_tree_path_free (path);

        path = gtk_tree_row_reference_get_path (priv->move_reference);
        gtk_tree_view_set_cursor (priv->tree_view, path, NULL, FALSE);
        gtk_tree_path_free (path);

        g_object_unref (column);

        gtk_tree_row_reference_free (priv->move_reference);
        priv->move_reference = NULL;
}

static void
column_manager_selection_changed_cb (GtkTreeSelection *selection,
                                     GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv = manager->priv;
        GtkTreeViewColumn *column;
        GtkTreeModel *model;
        GtkTreeIter iter;
        GtkTreePath *path;
        gboolean visible;
        gint last, index;

        if (!gtk_tree_selection_get_selected (selection, &model, &iter))
                return;

        path = gtk_tree_model_get_path (model, &iter);
        gtk_tree_model_get (model, &iter, 0, &column, -1);
        visible = gtk_tree_view_column_get_visible (column);
        last = gtk_tree_model_iter_n_children (model, NULL) - 1;
        index = gtk_tree_path_get_indices (path)[0];

        /* Update button sensitivity. */
        gtk_widget_set_sensitive (priv->move_up_button, index > 0);
        gtk_widget_set_sensitive (priv->move_down_button, index < last);
        gtk_widget_set_sensitive (priv->show_button, !visible);
        gtk_widget_set_sensitive (priv->hide_button, visible);

        gtk_tree_path_free (path);
        g_object_unref (column);
}

static void
column_manager_toggled_cb (GtkCellRendererToggle *renderer,
                           gchar *path_string,
                           GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv = manager->priv;
        GtkTreePath *path;

        /* Select the toggled row. */
        path = gtk_tree_path_new_from_string (path_string);
        gtk_tree_view_set_cursor (priv->tree_view, path, NULL, FALSE);
        gtk_tree_path_free (path);

        if (gtk_cell_renderer_toggle_get_active (renderer))
                column_manager_hide_selected (manager);
        else
                column_manager_show_selected (manager);
}

static void
column_manager_notify_visible_cb (GtkTreeViewColumn *column,
                                  GParamSpec *pspec,
                                  GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv = manager->priv;
        GtkTreeModel *model;
        GtkTreePath *path;
        GtkTreeIter iter;
        GList *list;
        gboolean valid;
        gint index;

        /* Find the index for this column. */
        list = gtk_tree_view_get_columns (priv->managed_view);
        index = g_list_index (list, column);
        g_list_free (list);

        /* Convert the index to an iterator. */
        model = gtk_tree_view_get_model (priv->tree_view);
        path = gtk_tree_path_new_from_indices (index, -1);
        valid = gtk_tree_model_get_iter (model, &iter, path);
        g_return_if_fail (valid);

        /* Notify the toggle cell renderer and buttons. */
        gtk_tree_model_row_changed (model, path, &iter);

        gtk_tree_path_free (path);
}

static void
column_manager_update_view (GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv = manager->priv;
        GtkListStore *list_store;
        GList *list;

        list_store = gtk_list_store_new (1, GTK_TYPE_TREE_VIEW_COLUMN);
        list = gtk_tree_view_get_columns (priv->managed_view);

        while (list != NULL)
        {
                GtkTreeViewColumn *column = list->data;
                GtkTreeIter iter;

                g_signal_connect (
                        column, "notify::visible",
                        G_CALLBACK (column_manager_notify_visible_cb),
                        manager);

                gtk_list_store_append (list_store, &iter);
                gtk_list_store_set (list_store, &iter, 0, column, -1);
                list = g_list_delete_link (list, list);
        }

        gtk_tree_view_set_model (
                priv->tree_view, GTK_TREE_MODEL (list_store));

        g_signal_connect (
                list_store, "row-changed",
                G_CALLBACK (column_manager_row_changed_cb), manager);
        g_signal_connect (
                list_store, "row-inserted",
                G_CALLBACK (column_manager_row_inserted_cb), manager);
        g_signal_connect (
                list_store, "row-deleted",
                G_CALLBACK (column_manager_row_deleted_cb), manager);

        g_object_unref (list_store);
}

static GObject *
column_manager_constructor (GType type,
                            guint n_construct_properties,
                            GObjectConstructParam *construct_properties)
{
        GvaColumnManagerPrivate *priv;
        GtkWidget *widget;
        GObject *object;

        /* Chain up to parent's constructor() method. */
        object = G_OBJECT_CLASS (parent_class)->constructor (
                type, n_construct_properties, construct_properties);

        priv = GVA_COLUMN_MANAGER_GET_PRIVATE (object);

        widget = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (
                GTK_SCROLLED_WINDOW (widget),
                GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (
                GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_IN);
        gtk_container_add (
                GTK_CONTAINER (widget), GTK_WIDGET (priv->tree_view));
        gtk_box_pack_start (GTK_BOX (object), widget, TRUE, TRUE, 0);

        widget = GVA_COLUMN_MANAGER (object)->vbox;
        gtk_box_pack_start (GTK_BOX (object), widget, FALSE, FALSE, 0);

        gtk_box_set_spacing (GTK_BOX (object), 6);
        gtk_widget_show_all (GTK_WIDGET (object));

        return object;
}

static void
column_manager_set_property (GObject *object,
                             guint property_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_MANAGED_VIEW:
                        gva_column_manager_set_managed_view (
                                GVA_COLUMN_MANAGER (object),
                                g_value_get_object (value));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
column_manager_get_property (GObject *object,
                             guint property_id,
                             GValue *value,
                             GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_MANAGED_VIEW:
                        g_value_set_object (
                                value, gva_column_manager_get_managed_view (
                                GVA_COLUMN_MANAGER (object)));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
column_manager_dispose (GObject *object)
{
        GvaColumnManager *manager = GVA_COLUMN_MANAGER (object);
        GvaColumnManagerPrivate *priv = manager->priv;

        if (manager->vbox != NULL)
        {
                g_object_unref (manager->vbox);
                manager->vbox = NULL;
        }

        if (priv->managed_view != NULL)
        {
                g_signal_handlers_disconnect_by_func (
                        priv->managed_view,
                        column_manager_update_view, manager);

                g_object_unref (priv->managed_view);
                manager->priv->managed_view = NULL;
        }

        if (priv->tree_view != NULL)
        {
                g_object_unref (priv->tree_view);
                priv->tree_view = NULL;
        }

        if (priv->move_up_button != NULL)
        {
                g_object_unref (priv->move_up_button);
                priv->move_up_button = NULL;
        }

        if (priv->move_down_button != NULL)
        {
                g_object_unref (priv->move_down_button);
                priv->move_down_button = NULL;
        }

        if (priv->show_button != NULL)
        {
                g_object_unref (priv->show_button);
                priv->show_button = NULL;
        }

        if (priv->hide_button != NULL)
        {
                g_object_unref (priv->hide_button);
                priv->hide_button = NULL;
        }

        /* Chain up to parent's dispose() method. */
        G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
column_manager_finalize (GObject *object)
{
        GvaColumnManagerPrivate *priv;

        priv = GVA_COLUMN_MANAGER_GET_PRIVATE (object);

        gtk_tree_row_reference_free (priv->move_reference);

        /* Chain up to parent's finalize() method. */
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
column_manager_class_init (GvaColumnManagerClass *class)
{
        GObjectClass *object_class;

        parent_class = g_type_class_peek_parent (class);
        g_type_class_add_private (class, sizeof (GvaColumnManagerPrivate));

        object_class = G_OBJECT_CLASS (class);
        object_class->constructor = column_manager_constructor;
        object_class->set_property = column_manager_set_property;
        object_class->get_property = column_manager_get_property;
        object_class->dispose = column_manager_dispose;
        object_class->finalize = column_manager_finalize;

        g_object_class_install_property (
                object_class,
                PROP_MANAGED_VIEW,
                g_param_spec_object (
                        "managed-view",
                        _("Managed View"),
                        _("The GtkTreeView being managed"),
                        GTK_TYPE_TREE_VIEW,
                        G_PARAM_READWRITE));
}

static void
column_manager_init (GvaColumnManager *manager)
{
        GvaColumnManagerPrivate *priv;
        GtkTreeViewColumn *column;
        GtkCellRenderer *renderer;
        GtkWidget *widget;
        GtkWidget *vbox;

        priv = GVA_COLUMN_MANAGER_GET_PRIVATE (manager);
        manager->priv = priv;

        /* Initialize the tree view. */

        priv->tree_view = g_object_ref_sink (gtk_tree_view_new ());
        gtk_tree_view_set_headers_visible (priv->tree_view, FALSE);
        gtk_tree_view_set_reorderable (priv->tree_view, TRUE);

        g_signal_connect (
                gtk_tree_view_get_selection (priv->tree_view), "changed",
                G_CALLBACK (column_manager_selection_changed_cb), manager);

        column = gtk_tree_view_column_new ();
        renderer = gtk_cell_renderer_toggle_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);
        gtk_tree_view_append_column (priv->tree_view, column);

        gtk_tree_view_column_set_cell_data_func (
                column, renderer, (GtkTreeCellDataFunc)
                column_manager_render_visible, NULL, NULL);

        g_signal_connect (
                renderer, "toggled",
                G_CALLBACK (column_manager_toggled_cb), manager);

        column = gtk_tree_view_column_new ();
        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (column, renderer, TRUE);
        gtk_tree_view_append_column (priv->tree_view, column);

        gtk_tree_view_column_set_cell_data_func (
                column, renderer, (GtkTreeCellDataFunc)
                column_manager_render_title, NULL, NULL);

        /* Initialize the vertical button box. */

        vbox = gtk_vbutton_box_new ();
        gtk_box_set_spacing (GTK_BOX (vbox), 6);
        gtk_button_box_set_layout (GTK_BUTTON_BOX (vbox), GTK_BUTTONBOX_START);
        manager->vbox = g_object_ref_sink (vbox);

        widget = gtk_button_new_with_mnemonic (_("Move _Up"));
        gtk_button_set_alignment (GTK_BUTTON (widget), 0.0, 0.5);
        gtk_button_set_image (
                GTK_BUTTON (widget), gtk_image_new_from_stock (
                GTK_STOCK_GO_UP, GTK_ICON_SIZE_BUTTON));
        gtk_widget_set_sensitive (widget, FALSE);
        gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
        priv->move_up_button = g_object_ref (widget);

        g_signal_connect_swapped (
                priv->move_up_button, "clicked",
                G_CALLBACK (column_manager_move_selected_up), manager);

        widget = gtk_button_new_with_mnemonic (_("Move _Down"));
        gtk_button_set_alignment (GTK_BUTTON (widget), 0.0, 0.5);
        gtk_button_set_image (
                GTK_BUTTON (widget), gtk_image_new_from_stock (
                GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_BUTTON));
        gtk_widget_set_sensitive (widget, FALSE);
        gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
        priv->move_down_button = g_object_ref (widget);

        g_signal_connect_swapped (
                priv->move_down_button, "clicked",
                G_CALLBACK (column_manager_move_selected_down), manager);

        widget = gtk_button_new_with_mnemonic (_("_Show"));
        gtk_widget_set_sensitive (widget, FALSE);
        gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
        priv->show_button = g_object_ref (widget);

        g_signal_connect_swapped (
                priv->show_button, "clicked",
                G_CALLBACK (column_manager_show_selected), manager);

        widget = gtk_button_new_with_mnemonic (_("_Hide"));
        gtk_widget_set_sensitive (widget, FALSE);
        gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
        priv->hide_button = g_object_ref (widget);

        g_signal_connect_swapped (
                priv->hide_button, "clicked",
                G_CALLBACK (column_manager_hide_selected), manager);
}

GType
gva_column_manager_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info =
                {
                        sizeof (GvaColumnManagerClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) column_manager_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (GvaColumnManager),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) column_manager_init,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        GTK_TYPE_HBOX, "GvaColumnManager", &type_info, 0);
        }

        return type;
}

/**
 * gva_column_manager_new:
 * @managed_view: a #GtkTreeView to manage
 *
 * Creates a new #GtkColumnManager.
 *
 * Returns: a new #GtkColumnManager
 **/
GtkWidget *
gva_column_manager_new (GtkTreeView *managed_view)
{
        g_return_val_if_fail (GTK_IS_TREE_VIEW (managed_view), NULL);

        return g_object_new (
                GVA_TYPE_COLUMN_MANAGER, "managed-view", managed_view, NULL);
}

/**
 * gva_column_manager_get_managed_view:
 * @manager: a #GvaColumnManager
 *
 * Returns the #GtkTreeView being managed.
 *
 * Returns: the #GtkTreeView being managed
 **/
GtkTreeView *
gva_column_manager_get_managed_view (GvaColumnManager *manager)
{
        g_return_val_if_fail (GVA_IS_COLUMN_MANAGER (manager), NULL);

        return manager->priv->managed_view;
}

/**
 * gva_column_manager_set_managed_view:
 * @manager: a #GvaColumnManager
 * @managed_view: a #GtkTreeView to manage
 *
 * Sets a new #GtkTreeView to manage.  The widget will be updated
 * appropriately.
 **/
void
gva_column_manager_set_managed_view (GvaColumnManager *manager,
                                     GtkTreeView *managed_view)
{
        g_return_if_fail (GVA_IS_COLUMN_MANAGER (manager));

        if (managed_view != NULL)
                g_return_if_fail (GTK_IS_TREE_VIEW (managed_view));

        if (manager->priv->managed_view != NULL)
        {
                g_signal_handlers_disconnect_by_func (
                        manager->priv->managed_view,
                        column_manager_update_view, manager);

                g_object_unref (manager->priv->managed_view);
                manager->priv->managed_view = NULL;
        }

        if (managed_view != NULL)
        {
                g_signal_connect_swapped (
                        managed_view, "columns-changed",
                        G_CALLBACK (column_manager_update_view), manager);

                manager->priv->managed_view = g_object_ref (managed_view);

                column_manager_update_view (manager);
        }
}

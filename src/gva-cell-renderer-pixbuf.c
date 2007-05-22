#include "gva-cell-renderer-pixbuf.h"

enum {
        CLICKED,
        LAST_SIGNAL
};

static gpointer parent_class = NULL;
static guint signals[LAST_SIGNAL] = { 0 };

static gboolean
cell_renderer_pixbuf_activate (GtkCellRenderer *cell,
                               GdkEvent *event,
                               GtkWidget *widget,
                               const gchar *path,
                               GdkRectangle *background_area,
                               GdkRectangle *cell_area,
                               GtkCellRendererState flags)
{
        GtkTreePath *tree_path;

        tree_path = gtk_tree_path_new_from_string (path);
        g_assert (tree_path != NULL);
        g_signal_emit (cell, signals[CLICKED], 0, tree_path);
        gtk_tree_path_free (tree_path);

        return TRUE;
}

static void
cell_renderer_pixbuf_class_init (GvaCellRendererPixbufClass *class)
{
        GtkCellRendererClass *cell_renderer_class;

        parent_class = g_type_class_peek_parent (class);

        cell_renderer_class = GTK_CELL_RENDERER_CLASS (class);
        cell_renderer_class->activate = cell_renderer_pixbuf_activate;

        signals[CLICKED] = g_signal_new (
                "clicked",
                G_OBJECT_CLASS_TYPE (class),
                G_SIGNAL_RUN_LAST,
                G_STRUCT_OFFSET (GvaCellRendererPixbufClass, clicked),
                NULL, NULL,
                g_cclosure_marshal_VOID__BOXED,
                G_TYPE_NONE, 1,
                GTK_TYPE_TREE_PATH);
}

static void
cell_renderer_pixbuf_init (GvaCellRendererPixbuf *cell_renderer_pixbuf)
{
        GTK_CELL_RENDERER (cell_renderer_pixbuf)->mode =
                GTK_CELL_RENDERER_MODE_ACTIVATABLE;
}

GType
gva_cell_renderer_pixbuf_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (GvaCellRendererPixbufClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) cell_renderer_pixbuf_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (GvaCellRendererPixbuf),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) cell_renderer_pixbuf_init,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        GTK_TYPE_CELL_RENDERER_PIXBUF,
                        "GvaCellRendererPixbuf", &type_info, 0);
        }

        return type;
}

GtkCellRenderer *
gva_cell_renderer_pixbuf_new (void)
{
        return g_object_new (GVA_TYPE_CELL_RENDERER_PIXBUF, NULL);
}

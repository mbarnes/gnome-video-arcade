#ifndef GVA_CELL_RENDERER_PIXBUF_H
#define GVA_CELL_RENDERER_PIXBUF_H

#include "gva-common.h"

/* Standard GObject macros */
#define GVA_TYPE_CELL_RENDERER_PIXBUF \
        (gva_cell_renderer_pixbuf_get_type ())
#define GVA_CELL_RENDERER_PIXBUF(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_CELL_RENDERER_PIXBUF, GvaCellRendererPixbuf))
#define GVA_CELL_RENDERER_PIXBUF_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_CELL_RENDERER_PIXBUF, GvaCellRendererPixbufClass))
#define GVA_IS_CELL_RENDERER_PIXBUF(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_CELL_RENDERER_PIXBUF))
#define GVA_IS_CELL_RENDERER_PIXBUF_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_CELL_RENDERER_PIXBUF))
#define GVA_CELL_RENDERER_PIXBUF_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_CELL_RENDERER_PIXBUF, GvaCellRendererPixbufClass))

G_BEGIN_DECLS

typedef struct _GvaCellRendererPixbuf GvaCellRendererPixbuf;
typedef struct _GvaCellRendererPixbufClass GvaCellRendererPixbufClass;

struct _GvaCellRendererPixbuf
{
        GtkCellRendererPixbuf parent;
};

struct _GvaCellRendererPixbufClass
{
        GtkCellRendererPixbufClass parent_class;

        void (*clicked) (GvaCellRendererPixbuf *cell_renderer_pixbuf,
                         GtkTreePath *tree_path);
};

GType             gva_cell_renderer_pixbuf_get_type   (void);
GtkCellRenderer * gva_cell_renderer_pixbuf_new        (void);

G_END_DECLS

#endif /* GVA_CELL_RENDERER_PIXBUF_H */

#ifndef GVA_TIME_H
#define GVA_TIME_H

#include "gva-common.h"

/* Create a portable GType for time_t.
 *
 * IEEE 1003.1 does not specify the size of time_t, nor even whether it's
 * an integer or floating-point type.  So represent the value as a subtype
 * of G_TYPE_BOXED.
 */

#define GVA_TYPE_TIME   (gva_time_get_type ())

G_BEGIN_DECLS

GType           gva_time_get_type               (void);
void            gva_time_set_tree_sortable_func (GtkTreeSortable *sortable,
                                                 gint sort_column_id);

G_END_DECLS

#endif /* GVA_TIME_H */

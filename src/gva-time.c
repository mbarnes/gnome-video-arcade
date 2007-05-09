#include "gva-time.h"

#include <string.h>
#include <time.h>

static gpointer
time_copy (gpointer boxed)
{
        gpointer boxed_copy;

        boxed_copy = g_slice_alloc (sizeof (time_t));
        memcpy (boxed_copy, boxed, sizeof (time_t));
        return boxed_copy;
}

static void
time_free (gpointer boxed)
{
        g_slice_free1 (sizeof (time_t), boxed);
}

GType
gva_time_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
                type = g_boxed_type_register_static (
                        "GvaTime", time_copy, time_free);

        return type;
}

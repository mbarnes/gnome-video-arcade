#include "gva-error.h"

GQuark
gva_error_quark (void)
{
        static GQuark quark = 0;

        if (G_UNLIKELY (quark == 0))
                quark = g_quark_from_static_string ("gva-error-quark");

        return quark;
}

void
gva_error_handle (GError **error)
{
        g_return_if_fail (error != NULL);

        if (*error != NULL)
        {
                g_warning ("%s", (*error)->message);
                g_clear_error (error);
        }
}

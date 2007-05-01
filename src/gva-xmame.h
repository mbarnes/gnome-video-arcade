#ifndef GVA_XMAME_H
#define GVA_XMAME_H

#include "gva-common.h"

#define GVA_ERROR               gva_error_quark ()

G_BEGIN_DECLS

typedef enum {
        GVA_ERROR_XMAME
} GvaError;

GQuark          gva_error_quark                 (void);
gchar *         gva_xmame_get_executable        (GError **error);
gboolean        gva_xmame_scan_for_error        (const gchar *xmame_output,
                                                 GError **error);
gboolean        gva_xmame_command               (const gchar *arguments,
                                                 gchar **standard_output,
                                                 gchar **standard_error,
                                                 GError **error);
gchar *         gva_xmame_get_version           (GError **error);
gchar *         gva_xmame_get_config_value      (const gchar *config_key,
                                                 GError **error);
GHashTable *    gva_xmame_list_full             (GError **error);
gboolean        gva_xmame_run_game              (const gchar *romname,
                                                 GError **error);
gboolean        gva_xmame_record_game           (const gchar *romname,
                                                 const gchar *inpname,
                                                 GError **error);
gboolean        gva_xmame_playback_game         (const gchar *inpname,
                                                 GError **error);

G_END_DECLS

#endif /* GVA_XMAME_H */

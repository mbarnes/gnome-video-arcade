#ifndef GVA_XMAME_H
#define GVA_XMAME_H

#include "gva-common.h"
#include "gva-process.h"

#define GVA_ERROR               gva_error_quark ()

G_BEGIN_DECLS

typedef enum
{
        GVA_ERROR_SYSTEM,
        GVA_ERROR_XMAME

} GvaError;

typedef void    (*GvaXmameVerifyFunc)           (const gchar *romname,
                                                 const gchar *result,
                                                 gpointer user_data);

GQuark          gva_error_quark                 (void);
gchar *         gva_xmame_get_executable        (GError **error);
GvaProcess *    gva_xmame_async_command         (const gchar *arguments,
                                                 GError **error);
gint            gva_xmame_command               (const gchar *arguments,
                                                 gchar **standard_output,
                                                 gchar **standard_error,
                                                 GError **error);
gint            gva_xmame_wait_for_exit         (GvaProcess *process,
                                                 GError **error);

gchar *         gva_xmame_get_version           (GError **error);
gchar *         gva_xmame_get_config_value      (const gchar *config_key,
                                                 GError **error);
gboolean        gva_xmame_has_config_value      (const gchar *config_key);
GList *         gva_xmame_get_available         (GError **error);
GHashTable *    gva_xmame_get_input_files       (GError **error);
gchar *         gva_xmame_get_rompath           (GError **error);
GHashTable *    gva_xmame_list_full             (GError **error);
GvaProcess *    gva_xmame_verify_rom_sets       (GvaXmameVerifyFunc func,
                                                 gpointer user_data,
                                                 GError **error);
GHashTable *    gva_xmame_verify_sample_sets    (GError **error);
gboolean        gva_xmame_run_game              (const gchar *romname,
                                                 GError **error);
gboolean        gva_xmame_record_game           (const gchar *romname,
                                                 const gchar *inpname,
                                                 GError **error);
gboolean        gva_xmame_playback_game         (const gchar *romname,
                                                 const gchar *inpname,
                                                 GError **error);
gboolean        gva_xmame_clear_state           (const gchar *romname,
                                                 GError **error);
gboolean        gva_xmame_supports_auto_save    (void);
gboolean        gva_xmame_supports_full_screen  (void);

G_END_DECLS

#endif /* GVA_XMAME_H */

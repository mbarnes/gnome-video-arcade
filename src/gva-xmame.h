#ifndef GVA_XMAME_H
#define GVA_XMAME_H

#include "gva-common.h"
#include "gva-process.h"

G_BEGIN_DECLS

typedef void    (*GvaXmameCallback)             (const gchar *romname,
                                                 const gchar *game_data,
                                                 gpointer user_data);

gint            gva_xmame_command               (const gchar *arguments,
                                                 gchar ***stdout_lines,
                                                 gchar ***stderr_lines,
                                                 GError **error);

gchar *         gva_xmame_get_version           (GError **error);
gchar *         gva_xmame_get_config_value      (const gchar *config_key,
                                                 GError **error);
gboolean        gva_xmame_has_config_value      (const gchar *config_key);
GHashTable *    gva_xmame_get_input_files       (GError **error);
GPtrArray *     gva_xmame_get_romset_files      (GError **error);
GvaProcess *    gva_xmame_list_full             (GvaXmameCallback callback,
                                                 gpointer user_data,
                                                 GError **error);
GvaProcess *    gva_xmame_verify_rom_sets       (GvaXmameCallback callback,
                                                 gpointer user_data,
                                                 GError **error);
GvaProcess *    gva_xmame_verify_sample_sets    (GvaXmameCallback callback,
                                                 gpointer user_data,
                                                 GError **error);
GvaProcess *    gva_xmame_run_game              (const gchar *romname,
                                                 GError **error);
GvaProcess *    gva_xmame_record_game           (const gchar *romname,
                                                 const gchar *inpname,
                                                 GError **error);
GvaProcess *    gva_xmame_playback_game         (const gchar *romname,
                                                 const gchar *inpname,
                                                 GError **error);
gboolean        gva_xmame_clear_state           (const gchar *romname,
                                                 GError **error);
gboolean        gva_xmame_supports_auto_save    (void);
gboolean        gva_xmame_supports_full_screen  (void);

G_END_DECLS

#endif /* GVA_XMAME_H */

#ifndef GVA_MAME_PROCESS_H
#define GVA_MAME_PROCESS_H

#include "gva-common.h"
#include "gva-process.h"

/* Standard GObject macros */
#define GVA_TYPE_MAME_PROCESS \
        (gva_mame_process_get_type ())
#define GVA_MAME_PROCESS(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST \
        ((obj), GVA_TYPE_MAME_PROCESS, GvaMameProcess))
#define GVA_MAME_PROCESS_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_CAST \
        ((cls), GVA_TYPE_MAME_PROCESS, GvaMameProcessClass))
#define GVA_IS_MAME_PROCESS(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE \
        ((obj), GVA_TYPE_MAME_PROCESS))
#define GVA_IS_MAME_PROCESS_CLASS(cls) \
        (G_TYPE_CHECK_CLASS_TYPE \
        ((cls), GVA_TYPE_MAME_PROCESS))
#define GVA_MAME_PROCESS_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS \
        ((obj), GVA_TYPE_MAME_PROCESS, GvaMameProcessClass))

G_BEGIN_DECLS

typedef struct _GvaMameProcess GvaMameProcess;
typedef struct _GvaMameProcessClass GvaMameProcessClass;

struct _GvaMameProcess
{
        GvaProcess parent;
};

struct _GvaMameProcessClass
{
        GvaProcessClass parent_class;
};

GType           gva_mame_process_get_type       (void);
gchar *         gva_mame_process_get_executable (GError **error);
GvaProcess *    gva_mame_process_spawn          (const gchar *arguments,
                                                 GError **error);

G_END_DECLS

#endif /* GVA_MAME_PROCESS_H */

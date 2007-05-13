#ifndef GVA_MODELS_H
#define GVA_MODELS_H

#include "gva-common.h"

G_BEGIN_DECLS

typedef enum {
        GVA_MODEL_AVAILABLE,
        GVA_MODEL_FAVORITES,
        GVA_MODEL_RESULTS,
        GVA_NUM_MODELS
} GvaModelType;

GvaModelType    gva_models_get_current_model    (void);
void            gva_models_set_current_model    (GvaModelType model);

G_END_DECLS

#endif /* GVA_MODELS_H */

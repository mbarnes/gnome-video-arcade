/* Copyright 2007-2009 Matthew Barnes
 *
 * This file is part of GNOME Video Arcade.
 *
 * GNOME Video Arcade is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * GNOME Video Arcade is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION: gva-audit
 * @short_description: Audit ROM and Sample Sets
 *
 * These functions audit the available ROM and audio sample sets for
 * correctness and also manipulate the audit results window.
 **/

#ifndef GVA_AUDIT_H
#define GVA_AUDIT_H

#include "gva-common.h"
#include "gva-process.h"

G_BEGIN_DECLS

void            gva_audit_init                  (void);
GvaProcess *    gva_audit_roms                  (GError **error);
GvaProcess *    gva_audit_samples               (GError **error);
void            gva_audit_save_errors           (void);
gboolean        gva_audit_detect_changes        (void);

G_END_DECLS

#endif /* GVA_AUDIT_H */

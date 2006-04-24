/* PCL - Predicate Constraint Language
 * Copyright (C) 2006 The Boeing Company
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "pcl.h"

#define NAME_CHARS \
        "01234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"

static gpointer code_parent_class = NULL;

static gboolean
code_is_valid_name (const guchar *string)
{
        static guchar *name_chars = (guchar *) NAME_CHARS;
        static gchar is_name_char[256];  /* XXX G_MAXUCHAR? */
        const guchar *cp;

        /* first time only */
        if (is_name_char[*name_chars] == 0)
                for (cp = name_chars; *cp != '\0'; cp++)
                        is_name_char[*cp] = 1;

        for (cp = string; *cp != '\0'; cp++)
                if (is_name_char[*cp] == 0)
                        return FALSE;
        return TRUE;
}

static void
code_intern_strings (PclObject *tuple)
{
        glong ii, size;

        size = PCL_TUPLE_GET_SIZE (tuple);
        for (ii = 0; ii < size; ii++)
        {
                PclObject **p_item = (PclObject **)
                        &PCL_TUPLE_GET_ITEM (tuple, ii);
                if (!PCL_IS_STRING (*p_item))
                        g_error ("non-string found in code slot");
                pcl_string_intern_in_place (p_item);
        }
}

static void
code_dispose (GObject *g_object)
{
        PclCode *self = PCL_CODE (g_object);

        PCL_CLEAR (self->instructions);
        PCL_CLEAR (self->lineno_table);
        PCL_CLEAR (self->constants);
        PCL_CLEAR (self->names);
        PCL_CLEAR (self->varnames);
        PCL_CLEAR (self->freevars);
        PCL_CLEAR (self->cellvars);
        PCL_CLEAR (self->filename);
        PCL_CLEAR (self->name);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (code_parent_class)->dispose (g_object);
}

static guint
code_hash (PclObject *object)
{
        PclCode *self = PCL_CODE (object);
        guint hash, h0, h1, h2, h3, h4, h5, h6;

        h0 = pcl_object_hash (self->name);
        if (h0 == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        h1 = pcl_object_hash (self->instructions);
        if (h1 == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        h2 = pcl_object_hash (self->constants);
        if (h2 == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        h3 = pcl_object_hash (self->names);
        if (h3 == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        h4 = pcl_object_hash (self->varnames);
        if (h4 == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        h5 = pcl_object_hash (self->freevars);
        if (h5 == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        h6 = pcl_object_hash (self->cellvars);
        if (h6 == PCL_HASH_INVALID)
                return PCL_HASH_INVALID;
        hash = h0 ^ h1 ^ h2 ^ h3 ^ h4 ^ h5 ^ h6
               ^ self->argument_count
               ^ self->variable_count
               ^ (guint) self->flags;
        return PCL_HASH_VALIDATE (hash);
}

static PclObject *
code_repr (PclObject *object)
{
        PclCode *code = PCL_CODE (object);
        gchar *filename = "???";
        gchar *codename = "???";

        if (PCL_IS_STRING (code->filename))
                filename = pcl_string_as_string (code->filename);

        if (PCL_IS_STRING (code->name))
                codename = pcl_string_as_string (code->name);

        return pcl_string_from_format (
               "<code object %s at %p, file \"%s\", line %d>",
               codename, code, filename, code->first_lineno);
}

static gint
code_compare (PclObject *object1, PclObject *object2)
{
        PclCode *code1, *code2;
        gint cmp;

        /* NOTE: Some members are excluded from comparison:
         *       first_lineno, lineno_table, filename, name */

        if (!PCL_IS_CODE (object1) || !PCL_IS_CODE (object2))
                return 2;

        code1 = PCL_CODE (object1);
        code2 = PCL_CODE (object2);

        cmp = pcl_object_compare (code1->name, code2->name);
        if (cmp) return cmp;
        cmp = code1->argument_count - code2->argument_count;
        if (cmp) return (cmp < 0) ? -1 : 1;
        cmp = code1->variable_count - code2->variable_count;
        if (cmp) return (cmp < 0) ? -1 : 1;
        cmp = code1->flags - code2->flags;
        if (cmp) return (cmp < 0) ? -1 : 1;
        cmp = pcl_object_compare (code1->instructions, code2->instructions);
        if (cmp) return cmp;
        cmp = pcl_object_compare (code1->constants, code2->constants);
        if (cmp) return cmp;
        cmp = pcl_object_compare (code1->names, code2->names);
        if (cmp) return cmp;
        cmp = pcl_object_compare (code1->varnames, code2->varnames);
        if (cmp) return cmp;
        cmp = pcl_object_compare (code1->freevars, code2->freevars);
        if (cmp) return cmp;
        cmp = pcl_object_compare (code1->cellvars, code2->cellvars);
        return cmp;
}

static gboolean
code_traverse (PclContainer *container, PclTraverseFunc func,
               gpointer user_data)
{
        PclCode *self = PCL_CODE (container);

        if (self->instructions != NULL)
                if (!func (self->instructions, user_data))
                        return FALSE;
        if (self->lineno_table != NULL)
                if (!func (self->lineno_table, user_data))
                        return FALSE;
        if (self->constants != NULL)
                if (!func (self->constants, user_data))
                        return FALSE;
        if (self->names != NULL)
                if (!func (self->names, user_data))
                        return FALSE;
        if (self->varnames != NULL)
                if (!func (self->varnames, user_data))
                        return FALSE;
        if (self->freevars != NULL)
                if (!func (self->freevars, user_data))
                        return FALSE;
        if (self->cellvars != NULL)
                if (!func (self->cellvars, user_data))
                        return FALSE;
        if (self->filename != NULL)
                if (!func (self->filename, user_data))
                        return FALSE;
        if (self->name != NULL)
                if (!func (self->name, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (code_parent_class)->
                traverse (container, func, user_data);
}

static void
code_class_init (PclCodeClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        code_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = code_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_code_get_type_object;
        object_class->hash = code_hash;
        object_class->repr = code_repr;
        object_class->compare = code_compare;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = code_dispose;
}

GType
pcl_code_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclCodeClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) code_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclCode),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclCode", &type_info, 0);
        }
        return type;
}

/**
 * pcl_code_get_type_object:
 *
 * Returns the type object for #PclCode.
 *
 * Returns: a borrowed reference to the type object for #PclCode
 */
PclObject *
pcl_code_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_CODE, "code");
                pcl_register_singleton ("<type 'code'>", &object);
        }
        return object;
}

PclCode *
pcl_code_new (guint argument_count, guint variable_count, gulong stack_size,
              PclObject *instructions, PclObject *lineno_table,
              PclObject *constants, PclObject *names, PclObject *varnames,
              PclObject *freevars, PclObject *cellvars, PclObject *filename,
              PclObject *name, gint first_lineno, PclCodeFlags flags)
{
        PclCode *code;
        glong ii, size;

        /* check arguments */
        if (argument_count < 0 || variable_count < 0 || instructions == NULL ||
                !PCL_IS_TUPLE (constants) || !PCL_IS_TUPLE (names) ||
                !PCL_IS_TUPLE (varnames) || !PCL_IS_TUPLE (freevars) ||
                !PCL_IS_TUPLE (cellvars) || !PCL_IS_STRING (filename) ||
                !PCL_IS_STRING (name))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        code_intern_strings (names);
        code_intern_strings (varnames);
        code_intern_strings (freevars);
        code_intern_strings (cellvars);

        /* intern selected string constants */
        size = PCL_TUPLE_GET_SIZE (constants);
        for (ii = 0; ii < size; ii++)
        {
                PclObject **p_item = (PclObject **)
                        &PCL_TUPLE_GET_ITEM (constants, ii);
                if (!PCL_IS_STRING (*p_item))
                        continue;
                if (!code_is_valid_name ((guchar *)
                        pcl_string_as_string (*p_item)))
                        continue;
                pcl_string_intern_in_place (p_item);
        }

        code = pcl_object_new (PCL_TYPE_CODE, NULL);
        if (code != NULL)
        {
                code->argument_count = argument_count;
                code->variable_count = variable_count;
                code->stack_size = stack_size;
                code->flags = flags;
                code->instructions = pcl_object_ref (instructions);
                code->lineno_table = pcl_object_ref (lineno_table);
                code->constants = pcl_object_ref (constants);
                code->names = pcl_object_ref (names);
                code->varnames = pcl_object_ref (varnames);
                code->freevars = pcl_object_ref (freevars);
                code->cellvars = pcl_object_ref (cellvars);
                code->filename = pcl_object_ref (filename);
                code->name = pcl_object_ref (name);
                code->first_lineno = first_lineno;

                if (PCL_TUPLE_GET_SIZE (freevars) == 0 &&
                        PCL_TUPLE_GET_SIZE (cellvars) == 0)
                        code->flags |= PCL_CODE_FLAG_NOFREE;
        }
        return code;
}

gint
pcl_code_addr_to_line (PclCode *code, gint addrq)
{
        glong ii, size = pcl_object_measure (code->lineno_table) / 2;
        guchar *cp = (guchar *) pcl_string_as_string (code->lineno_table);
        gint line = code->first_lineno;
        gint addr = 0;
        for (ii = 0; ii < size; ii++)
        {
                addr += *cp++;
                if (addr > addrq)
                        break;
                line += *cp++;
        }
        return line;
}

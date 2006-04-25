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
#include "glib-ext.h"  /* for g_hash_table_remove_all */

static PclFactory frame_factory;

static gpointer frame_parent_class = NULL;

/* Helper for pcl_frame_fast_to_locals() */
static void
names_to_dict (PclObject *names, glong size, PclObject *dict,
               PclObject **slots, gboolean dereference)
{
        PclObject *key;
        PclObject *value;
        glong ii;

        for (ii = 0; ii < size; ii++)
        {
                key = PCL_TUPLE_GET_ITEM (names, ii);
                value = slots[ii];
                if (dereference)
                        value = PCL_CELL_GET (value);
                if (value == NULL)
                {
                        if (!pcl_object_del_item (dict, key))
                                pcl_error_clear ();
                }
                else
                {
                        if (!pcl_object_set_item (dict, key, value))
                                pcl_error_clear ();
                }
        }
}

/* Helper for pcl_frame_locals_to_fast() */
static void
dict_to_names (PclObject *names, glong size, PclObject *dict,
               PclObject **slots, gboolean dereference, gboolean clear)
{
        PclObject *key;
        PclObject *value;
        glong ii;

        for (ii = 0; ii < size; ii++)
        {
                key = PCL_TUPLE_GET_ITEM (names, ii);
                value = pcl_object_get_item (dict, key);
                if (value == NULL)
                        pcl_error_clear ();
                if (dereference)
                {
                        if (value != NULL || clear)
                        {
                                if (PCL_CELL_GET (slots[ii]) != value)
                                        if (!pcl_cell_set (slots[ii], value))
                                                pcl_error_clear ();
                        }
                }
                else if (value != NULL || clear)
                {
                        if (slots[ii] != value)
                        {
                                if (value != NULL)
                                        pcl_object_ref (value);
                                if (slots[ii] != NULL)
                                        pcl_object_unref (slots[ii]);
                                slots[ii] = value;
                        }
                }
                if (value != NULL)
                        pcl_object_unref (value);
        }
}

static void
frame_dispose (GObject *g_object)
{
        PclFrame *self = PCL_FRAME (g_object);
        PclObject **p_object;

        PCL_CLEAR (self->previous);
        PCL_CLEAR (self->code);
        PCL_CLEAR (self->builtins);
        PCL_CLEAR (self->globals);
        PCL_CLEAR (self->locals);
        PCL_CLEAR (self->exc_type);
        PCL_CLEAR (self->exc_value);
        PCL_CLEAR (self->exc_traceback);
         
        /* slots */
        p_object = self->slots + self->slot_count;
        while (p_object-- != self->slots)
                PCL_CLEAR (*p_object);

        /* stack */
        if (self->tos != NULL)
        {
                while (self->tos-- != self->stack)
                        PCL_CLEAR (*self->tos);
                self->tos = NULL;
        }

        /* reset flags and counters */
        self->block_count = 0;
        self->cut = FALSE;

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (frame_parent_class)->dispose (g_object);
}

static void
frame_finalize (GObject *g_object)
{
        PclFrame *self = PCL_FRAME (g_object);

        g_free (self->stack);
        g_free (self->slots);

        /* Chain up to parent's finalize method. */
        G_OBJECT_CLASS (frame_parent_class)->finalize (g_object);
}

static gboolean
frame_traverse (PclContainer *container, PclTraverseFunc func,
                gpointer user_data)
{
        PclFrame *self = PCL_FRAME (container);
        PclObject **p_object;

        if (self->previous != NULL)
                if (!func (self->previous, user_data))
                        return FALSE;
        if (self->code != NULL)
                if (!func (self->code, user_data))
                        return FALSE;
        if (self->builtins != NULL)
                if (!func (self->builtins, user_data))
                        return FALSE;
        if (self->globals != NULL)
                if (!func (self->globals, user_data))
                        return FALSE;
        if (self->locals != NULL)
                if (!func (self->locals, user_data))
                        return FALSE;
        if (self->exc_type != NULL)
                if (!func (self->exc_type, user_data))
                        return FALSE;
        if (self->exc_value != NULL)
                if (!func (self->exc_value, user_data))
                        return FALSE;
        if (self->exc_traceback != NULL)
                if (!func (self->exc_traceback, user_data))
                        return FALSE;
        if (self->result != NULL)
                if (!func (self->result, user_data))
                        return FALSE;

        /* slots */
        p_object = self->slots + self->slot_count;
        while (p_object-- != self->slots)
                if (*p_object != NULL)
                        if (!func (*p_object, user_data))
                                return FALSE;

        /* stack */
        if (self->tos != NULL)
        {
                p_object = self->tos;
                while (p_object-- != self->stack)
                        if (*p_object != NULL)
                                if (!func (*p_object, user_data))
                                        return FALSE;
        }

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (frame_parent_class)->
                traverse (container, func, user_data);
}

static void
frame_class_init (PclFrameClass *class)
{
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        frame_parent_class = g_type_class_peek_parent (class);

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = frame_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_frame_get_type_object;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = frame_dispose;
        g_object_class->finalize = frame_finalize;
}

GType
pcl_frame_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclFrameClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) frame_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclFrame),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_CONTAINER, "PclFrame", &type_info, 0);
        }
        return type;
}

/**
 * pcl_frame_get_type_object:
 *
 * Returns the type object for #PclFrame.
 *
 * Returns: a borrowed reference to the type object for #PclFrame
 */
PclObject *
pcl_frame_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_FRAME, "frame");
                pcl_register_singleton ("<type 'frame'>", &object);
        }
        return object;
}

PclFrame *
pcl_frame_new (PclThreadState *ts, PclCode *code,
               PclObject *globals, PclObject *locals)
{
        PclFrame *previous = ts->frame;
        PclFrame *frame;
        PclObject *builtins;
        PclObject **p_object;
        guint prev_length;
        guint cell_count;
        guint free_count;
        guint slot_count;
        glong stack_size;

#ifdef G_ENABLE_DEBUG
        if (code == NULL || !PCL_IS_DICT (globals) ||
                (locals != NULL && !PCL_IS_MAPPING (locals)))
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }
#endif

        if (previous == NULL || previous->globals != globals)
        {
                builtins = pcl_dict_get_item_string (globals, "__builtins__");
                if (builtins != NULL)
                {
                        if (PCL_IS_MODULE (builtins))
                                builtins = pcl_module_get_dict (builtins);
                        else if (!PCL_IS_DICT (builtins))
                                builtins = NULL;
                }
                if (builtins == NULL)
                {
                        /* No builtins!  Make up a minimal one. */
                        builtins = pcl_dict_new ();
                        if (builtins == NULL || !pcl_dict_set_item_string (
                                        builtins, "None", PCL_NONE))
                                return NULL;
                }
                else
                        pcl_object_ref (builtins);
        }
        else
        {
                /* If we share the globals, we share the builtins. */
                builtins = pcl_object_ref (previous->builtins);
        }
        g_assert (PCL_IS_DICT (builtins));

        cell_count = PCL_TUPLE_GET_SIZE (code->cellvars);
        free_count = PCL_TUPLE_GET_SIZE (code->freevars);
        slot_count = code->variable_count + cell_count + free_count;
        stack_size = code->stack_size;

        prev_length = frame_factory.length;
        frame = pcl_factory_order (&frame_factory);
        if (frame_factory.length < prev_length)
        {
                frame->slots = g_renew (PclObject *, frame->slots, slot_count);
                frame->stack = g_renew (PclObject *, frame->stack, stack_size);
                memset (frame->slots, 0, slot_count * sizeof (gpointer));
                memset (frame->stack, 0, stack_size * sizeof (gpointer));
        }
        else
        {
                frame->slots = g_new0 (PclObject *, slot_count);
                frame->stack = g_new0 (PclObject *, stack_size);
        }
        if (previous != NULL)
                pcl_object_ref (previous);
        frame->previous = previous;
        frame->code = pcl_object_ref (code);
        frame->builtins = builtins;
        frame->tos = frame->stack;
        frame->ts = ts;
        frame->cell_count = cell_count;
        frame->free_count = free_count;
        frame->slot_count = slot_count;
        frame->stack_size = stack_size;
        frame->lasti = -1;  /* no instruction executed yet */

        if (code->flags & PCL_CODE_FLAG_PREDICATE)
                frame->result = pcl_list_new (0);

        /* Create cell objects in the appropriate slots. */
        p_object = frame->slots + code->variable_count;
        while (cell_count-- > 0)
                *p_object++ = pcl_cell_new (NULL);

        if (code->flags & PCL_CODE_FLAG_CONJUNCTION)
        {
                g_assert (previous != NULL);

                /* XXX Not really in GLib 2.6 */
                g_hash_table_remove_all (ts->cache);

                /* Pack up the previous frame's fast locals, clone 
                 * everything, and unpack the new frame's fast locals. */
                pcl_frame_fast_to_locals (previous);
                frame->globals = pcl_object_clone (previous->globals);
                frame->locals = pcl_object_clone (previous->locals);
                pcl_frame_locals_to_fast (frame, TRUE);

                frame->result = pcl_object_ref (previous->result);
        }
        else
        {
                frame->globals = pcl_object_ref (globals);
                if (locals == NULL)
                        locals = frame->globals;
                frame->locals = pcl_object_ref (locals);
        }

        return frame;
}

void
pcl_frame_block_setup (PclFrame *frame, guint type, guint handler, guint level)
{
        PclTryBlock *block;

        if (frame->block_count >= PCL_MAX_BLOCKS)
                g_error ("%s: Block stack overflow", G_STRFUNC);
        block = &frame->block[frame->block_count++];
        block->type = type;
        block->handler = handler;
        block->level = level;
}

PclTryBlock *
pcl_frame_block_pop (PclFrame *frame)
{
        if (frame->block_count == 0)
                g_error ("%s: Block stack underflow", G_STRFUNC);
        return &frame->block[--frame->block_count];
}

void
pcl_frame_fast_to_locals (PclFrame *frame)
{
        /* Merge fast locals into frame->locals */
        PclObject *locals, *names, **slots;
        PclObject *error_type, *error_value, *error_traceback;
        glong length;

        if (frame == NULL)
                return;

        locals = frame->locals;
        if (locals == NULL)
                locals = frame->locals = pcl_dict_new ();
        names = frame->code->varnames;
        g_assert (PCL_IS_TUPLE (names));
        length = PCL_TUPLE_GET_SIZE (names);
        length = CLAMP (length, 0, frame->code->variable_count);
        slots = frame->slots;

        pcl_error_fetch (&error_type, &error_value, &error_traceback);
        if (frame->code->variable_count > 0)
                names_to_dict (names, length, locals, slots, FALSE);
        if (frame->cell_count > 0 || frame->free_count > 0)
        {
                names = frame->code->cellvars;
                g_assert (PCL_IS_TUPLE (names));
                length = PCL_TUPLE_GET_SIZE (names);
                slots += frame->code->variable_count;
                names_to_dict (names, length, locals, slots, TRUE);

                names = frame->code->freevars;
                g_assert (PCL_IS_TUPLE (names));
                length = PCL_TUPLE_GET_SIZE (names);
                slots += frame->cell_count;
                names_to_dict (names, length, locals, slots, TRUE);
        }
        pcl_error_restore (error_type, error_value, error_traceback);
}

void
pcl_frame_locals_to_fast (PclFrame *frame, gboolean clear)
{
        /* Merge frame->locals into fast locals */
        PclObject *locals, *names, **slots;
        PclObject *error_type, *error_value, *error_traceback;
        glong length;

        if (frame == NULL)
                return;

        locals = frame->locals;
        names = frame->code->varnames;
        if (locals == NULL || !PCL_IS_TUPLE (names))
                return;
        slots = frame->slots;
        length = PCL_TUPLE_GET_SIZE (names);
        length = CLAMP (length, 0, frame->code->variable_count);

        pcl_error_fetch (&error_type, &error_value, &error_traceback);
        if (frame->code->variable_count > 0)
                dict_to_names (names, length, locals, slots, FALSE, clear);
        if (frame->cell_count > 0 || frame->free_count > 0)
        {
                if (!PCL_IS_TUPLE (frame->code->cellvars) &&
                        PCL_IS_TUPLE (frame->code->freevars))
                        return;

                names = frame->code->cellvars;
                g_assert (PCL_IS_TUPLE (names));
                length = PCL_TUPLE_GET_SIZE (names);
                slots += frame->code->variable_count;
                dict_to_names (names, length, locals, slots, TRUE, clear);

                names = frame->code->freevars;
                g_assert (PCL_IS_TUPLE (names));
                length = PCL_TUPLE_GET_SIZE (names);
                slots += frame->cell_count;
                dict_to_names (names, length, locals, slots, TRUE, clear);
        }
        pcl_error_restore (error_type, error_value, error_traceback);
}

void
_pcl_frame_init (void)
{
        frame_factory.type = PCL_TYPE_FRAME;
        frame_factory.limit = 200;
}

void
_pcl_frame_fini (void)
{
        pcl_factory_close (&frame_factory);
}

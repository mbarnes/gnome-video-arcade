/* Copyright 2007-2011 Matthew Barnes
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

/* A brief history of MAME's INP format.
 *
 * ??? - 0.112
 * - Unknown.  No INP headers?
 *
 * 0.113 - 0.115
 * - Simple headers introduced.
 *
 * 0.116 - 0.124
 * - Added support for playback of "extended" INP files that are commonly
 *   found on compete sites. [David Haywood]
 *
 * 0.125 - 0.128
 * - Versioned headers introduced (INP version 2.0).
 * - Dropped support for simple and extended formats.
 * - Rewrote INP recording from scratch, since all old INPs are broken
 *   anyways. Header now includes timestamp, which overrides the default
 *   time base for MAME's system time. ... [Aaron Giles]
 *
 * 0.129 - 0.130 (present)
 * - INP version 3.0.
 * - To fix 02688 (DIP switch settings are not being stored in INP files),
 *   old INP files had to be broken. Since they were already broken, the
 *   corefile module was enhanced to support streaming compression, and the
 *   new INP files are output and processed compressed. [Aaron Giles]
 */

#include "gva-input-file.h"

#include <errno.h>
#include <sys/stat.h>

#include "gva-db.h"
#include "gva-error.h"
#include "gva-mame.h"
#include "gva-util.h"

#define GVA_INPUT_FILE_GET_PRIVATE(obj) \
        (G_TYPE_INSTANCE_GET_PRIVATE \
        ((obj), GVA_TYPE_INPUT_FILE, GvaInputFilePrivate))

typedef struct _GvaInpHeaderSimple GvaInpHeaderSimple;
typedef struct _GvaInpHeaderExtended GvaInpHeaderExtended;
typedef struct _GvaInpHeaderVersioned GvaInpHeaderVersioned;

enum
{
        PROP_0,
        PROP_FILENAME,
        PROP_FORMAT,
        PROP_GAME,
        PROP_ORIGIN,
        PROP_TIMESTAMP
};

struct _GvaInputFilePrivate
{
        gchar *filename;
        gchar *format;
        gchar *game;
        gchar *origin;
        time_t timestamp;
};

/* Simple Header */
struct _GvaInpHeaderSimple
{
        gchar rom_name[9];      /* ROM name */
        gint8 major_version;    /* MAME major version */
        gint8 minor_version;    /* MAME minor version */
        gint8 beta_version;     /* MAME beta version */
        gchar reserved[20];     /* unused padding */
};

/* Extended Header */
struct _GvaInpHeaderExtended
{
        gchar header[7];        /* "XINP\0\0\0" */
        gchar rom_name[9];      /* ROM name */
        gchar origin[32];       /* MAME version string */
        guint32 timestamp;      /* approx start time */
        gchar reserved[32];     /* unused padding */
};

/* Versioned Header */
struct _GvaInpHeaderVersioned
{
        gchar header[8];        /* "MAMEINP\0" */
        gint64 timestamp;       /* approx start time */
        gint16 major_version;   /* INP format major version */
        gint16 minor_version;   /* INP format minor version */
        gchar rom_name[12];     /* ROM name */
        gchar origin[32];       /* MAME version string */
};

static gpointer parent_class = NULL;

static void
input_file_dump_header (GvaInputFile *input_file)
{
        const gchar *filename;
        const gchar *format;
        const gchar *game;
        const gchar *origin;
        gchar *inpname;
        time_t timestamp;

        filename = gva_input_file_get_filename (input_file);
        format = gva_input_file_get_format (input_file);
        game = gva_input_file_get_game (input_file);
        origin = gva_input_file_get_origin (input_file);
        timestamp = gva_input_file_get_timestamp (input_file);

        inpname = g_strdelimit (g_path_get_basename (filename), ".", '\0');

        g_log (G_LOG_DOMAIN, GVA_DEBUG_INP, "Input file: %s", inpname);
        g_log (G_LOG_DOMAIN, GVA_DEBUG_INP, "Game: %s", game);
        g_log (G_LOG_DOMAIN, GVA_DEBUG_INP, "%s", format);
        g_log (G_LOG_DOMAIN, GVA_DEBUG_INP, "Created %s", ctime (&timestamp));
        g_log (G_LOG_DOMAIN, GVA_DEBUG_INP, "Recorded using %s", origin);
        g_log (G_LOG_DOMAIN, GVA_DEBUG_INP, "--");

        g_free (inpname);
}

static gboolean
input_file_read_simple (GvaInputFile *input_file,
                        GMappedFile *mapped_file)
{
        GvaInpHeaderSimple *header;
        const gchar *contents;
        const gchar *filename;
        struct stat st;
        GString *buffer;
        gsize length;

        filename = gva_input_file_get_filename (input_file);

        contents = g_mapped_file_get_contents (mapped_file);
        length = g_mapped_file_get_length (mapped_file);

        if (length < sizeof (GvaInpHeaderSimple))
                return FALSE;

        header = (GvaInpHeaderSimple *) contents;

        input_file->priv->game = g_strndup (header->rom_name, 9);

        buffer = g_string_sized_new (64);
        g_string_append_printf (
                buffer, "MAME %d.%d",
                header->major_version,
                header->minor_version);
        if (header->beta_version > 0)
                g_string_append_printf (
                        buffer, "b%d", header->beta_version);
        input_file->priv->origin = g_string_free (buffer, FALSE);

        if (g_stat (filename, &st) == 0)
                input_file->priv->timestamp = st.st_mtime;

        input_file->priv->format = g_strdup ("INP simple format");

        if (gva_get_debug_flags () & GVA_DEBUG_INP)
                input_file_dump_header (input_file);

        return TRUE;
}

static gboolean
input_file_read_extended (GvaInputFile *input_file,
                          GMappedFile *mapped_file)
{
        GvaInpHeaderExtended *header;
        const gchar *contents;
        gsize length;

        contents = g_mapped_file_get_contents (mapped_file);
        length = g_mapped_file_get_length (mapped_file);

        if (length < sizeof (GvaInpHeaderExtended))
                return FALSE;

        if (memcmp (contents, "XINP\0\0\0", 7) != 0)
                return FALSE;

        header = (GvaInpHeaderExtended *) contents;

        input_file->priv->game = g_strndup (header->rom_name, 9);
        input_file->priv->origin = g_strndup (header->origin, 32);
        input_file->priv->timestamp = GINT32_FROM_LE (header->timestamp);

        input_file->priv->format = g_strdup ("INP extended format");

        if (gva_get_debug_flags () & GVA_DEBUG_INP)
                input_file_dump_header (input_file);

        return TRUE;
}

static gboolean
input_file_read_versioned (GvaInputFile *input_file,
                           GMappedFile *mapped_file)
{
        GvaInpHeaderVersioned *header;
        const gchar *contents;
        gsize length;

        contents = g_mapped_file_get_contents (mapped_file);
        length = g_mapped_file_get_length (mapped_file);

        if (length < sizeof (GvaInpHeaderVersioned))
                return FALSE;

        if (memcmp (contents, "MAMEINP\0", 8) != 0)
                return FALSE;

        header = (GvaInpHeaderVersioned *) contents;

        input_file->priv->game = g_strndup (header->rom_name, 12);
        input_file->priv->origin = g_strndup (header->origin, 32);
        input_file->priv->timestamp = GINT64_FROM_LE (header->timestamp);

        input_file->priv->format = g_strdup_printf (
                "INP version %d.%d",
                GINT16_FROM_LE (header->major_version),
                GINT16_FROM_LE (header->minor_version));

        if (gva_get_debug_flags () & GVA_DEBUG_INP)
                input_file_dump_header (input_file);

        return TRUE;
}

static void
input_file_set_filename (GvaInputFile *input_file,
                         const gchar *filename)
{
        g_return_if_fail (input_file->priv->filename == NULL);

        input_file->priv->filename = g_strdup (filename);
}

static void
input_file_set_property (GObject *object,
                         guint property_id,
                         const GValue *value,
                         GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_FILENAME:
                        input_file_set_filename (
                                GVA_INPUT_FILE (object),
                                g_value_get_string (value));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
input_file_get_property (GObject *object,
                         guint property_id,
                         GValue *value,
                         GParamSpec *pspec)
{
        switch (property_id)
        {
                case PROP_FILENAME:
                        g_value_set_string (
                                value, gva_input_file_get_filename (
                                GVA_INPUT_FILE (object)));
                        return;

                case PROP_FORMAT:
                        g_value_set_string (
                                value, gva_input_file_get_format (
                                GVA_INPUT_FILE (object)));
                        return;

                case PROP_GAME:
                        g_value_set_string (
                                value, gva_input_file_get_game (
                                GVA_INPUT_FILE (object)));
                        return;

                case PROP_ORIGIN:
                        g_value_set_string (
                                value, gva_input_file_get_origin (
                                GVA_INPUT_FILE (object)));
                        return;

                case PROP_TIMESTAMP:
                        g_value_set_int64 (
                                value, gva_input_file_get_timestamp (
                                GVA_INPUT_FILE (object)));
                        return;
        }

        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
input_file_finalize (GObject *object)
{
        GvaInputFilePrivate *priv;

        priv = GVA_INPUT_FILE_GET_PRIVATE (object);

        g_free (priv->filename);
        g_free (priv->format);
        g_free (priv->game);
        g_free (priv->origin);

        /* Chain up to parent's finalize() method. */
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
input_file_class_init (GvaInputFileClass *class)
{
        GObjectClass *object_class;

        parent_class = g_type_class_peek_parent (class);
        g_type_class_add_private (class, sizeof (GvaInputFilePrivate));

        object_class = G_OBJECT_CLASS (class);
        object_class->set_property = input_file_set_property;
        object_class->get_property = input_file_get_property;
        object_class->finalize = input_file_finalize;

        /**
         * GvaInputFile:filename:
         *
         * The name of the input file.
         **/
        g_object_class_install_property (
                object_class,
                PROP_FILENAME,
                g_param_spec_string (
                        "filename",
                        NULL,
                        NULL,
                        NULL,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT_ONLY));

        /**
         * GvaInputFile:format:
         *
         * The format of the input file.
         **/
        g_object_class_install_property (
                object_class,
                PROP_FORMAT,
                g_param_spec_string (
                        "format",
                        NULL,
                        NULL,
                        NULL,
                        G_PARAM_READABLE));

        /**
         * GvaInputFile:game:
         *
         * The corresponding ROM name for the input file.
         **/
        g_object_class_install_property (
                object_class,
                PROP_GAME,
                g_param_spec_string (
                        "game",
                        NULL,
                        NULL,
                        NULL,
                        G_PARAM_READABLE));

        /**
         * GvaInputFile:origin:
         *
         * The version of MAME that recorded the input file.
         **/
        g_object_class_install_property (
                object_class,
                PROP_ORIGIN,
                g_param_spec_string (
                        "origin",
                        NULL,
                        NULL,
                        NULL,
                        G_PARAM_READABLE));

        /**
         * GvaInputFile:timestamp:
         *
         * The creation timestamp for the input file.
         **/
        g_object_class_install_property (
                object_class,
                PROP_TIMESTAMP,
                g_param_spec_int64 (
                        "timestamp",
                        NULL,
                        NULL,
                        G_MININT64,
                        G_MAXINT64,
                        0,
                        G_PARAM_READABLE));
}

static void
input_file_init (GvaInputFile *input_file)
{
        input_file->priv = GVA_INPUT_FILE_GET_PRIVATE (input_file);
}

GType
gva_input_file_get_type (void)
{
        static GType type = 0;

        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info =
                {
                        sizeof (GvaInputFileClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) input_file_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (GvaInputFile),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) input_file_init,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        G_TYPE_OBJECT, "GvaInputFile", &type_info, 0);
        }

        return type;
}

/**
 * gva_input_file_new:
 * @filename: path to a MAME INP file
 *
 * Creates a new #GvaInputFile for the given @filename.
 *
 * Returns: a new #GvaInputFile
 **/
GvaInputFile *
gva_input_file_new (const gchar *filename)
{
        g_return_val_if_fail (filename != NULL, NULL);

        return g_object_new (GVA_TYPE_INPUT_FILE, "filename", filename, NULL);
}

/**
 * gva_input_file_read:
 * @input_file: a #GvaInputFile
 * @error: return location for a #GError, or %NULL
 *
 * Reads header information from a MAME INP file.  The information is
 * made available through the various accessor functions.  If an error
 * occurs while reading, the function returns %FALSE and sets @error.
 *
 * Returns: %TRUE on success, %FALSE if an error occurred
 **/
gboolean
gva_input_file_read (GvaInputFile *input_file,
                     GError **error)
{
        GMappedFile *mapped_file = NULL;
        const gchar *filename;
        gboolean success = TRUE;

        g_return_val_if_fail (GVA_IS_INPUT_FILE (input_file), FALSE);

        if (input_file->priv->format != NULL)
                goto exit;

        filename = gva_input_file_get_filename (input_file);

        mapped_file = g_mapped_file_new (filename, FALSE, error);
        if (mapped_file == NULL)
                return FALSE;

        input_file->priv->filename = g_strdup (filename);

        /* Check for a versioned INP header. */
        if (input_file_read_versioned (input_file, mapped_file))
                goto exit;

        /* Check for an extended INP header. */
        if (input_file_read_extended (input_file, mapped_file))
                goto exit;

        /* Check for a simple INP header. */
        if (input_file_read_simple (input_file, mapped_file))
                goto exit;

        g_set_error (
                error, GVA_ERROR, GVA_ERROR_FORMAT,
                _("Invalid or unsupported INP file format"));

        success = FALSE;

exit:
        if (success)
        {
                g_object_freeze_notify (G_OBJECT (input_file));
                g_object_notify (G_OBJECT (input_file), "format");
                g_object_notify (G_OBJECT (input_file), "game");
                g_object_notify (G_OBJECT (input_file), "origin");
                g_object_notify (G_OBJECT (input_file), "timestamp");
                g_object_thaw_notify (G_OBJECT (input_file));
        }

        if (mapped_file != NULL)
                g_mapped_file_free (mapped_file);

        return success;
}

/**
 * gva_input_file_play_back:
 * @input_file: a #GvaInputFile
 * @error: return location for a #GError, or %NULL
 *
 * Begins play back of @input_file and returns the resulting #GvaProcess.
 * If an error occurs, it returns %NULL and sets @error.
 *
 * Returns: a new #GvaProcess, or %NULL
 **/
GvaProcess *
gva_input_file_play_back (GvaInputFile *input_file,
                          GError **error)
{
        GvaProcess *process;
        const gchar *filename;
        const gchar *game;
        gchar *inpname;

        g_return_val_if_fail (GVA_IS_INPUT_FILE (input_file), NULL);

        if (!gva_input_file_read (input_file, error))
                return NULL;

        filename = gva_input_file_get_filename (input_file);
        game = gva_input_file_get_game (input_file);

        inpname = g_strdelimit (g_path_get_basename (filename), ".", '\0');
        process = gva_mame_playback_game (game, inpname, error);
        g_free (inpname);

        return process;
}

/**
 * gva_input_file_get_filename:
 * @input_file: a #GvaInputFile
 *
 * Returns the filename for @input_file.
 *
 * Returns: filename for @input_file
 **/
const gchar *
gva_input_file_get_filename (GvaInputFile *input_file)
{
        g_return_val_if_fail (GVA_IS_INPUT_FILE (input_file), NULL);

        return input_file->priv->filename;
}

/**
 * gva_input_file_get_format:
 * @input_file: a #GvaInputFile
 *
 * Returns a description of the INP header format.  Possible formats
 * are "INP simple format", "INP extended format", and "INP version x.y".
 *
 * Returns: description of the header format
 **/
const gchar *
gva_input_file_get_format (GvaInputFile *input_file)
{
        g_return_val_if_fail (GVA_IS_INPUT_FILE (input_file), NULL);

        if (input_file->priv->format == NULL)
                return NULL;

        return input_file->priv->format;
}

/**
 * gva_input_file_get_game:
 * @input_file: a #GvaInputFile
 *
 * Returns the corresponding ROM name for @input_file.
 *
 * Returns: corresponding ROM name for @input_file
 **/
const gchar *
gva_input_file_get_game (GvaInputFile *input_file)
{
        g_return_val_if_fail (GVA_IS_INPUT_FILE (input_file), NULL);

        return input_file->priv->game;
}

/**
 * gva_input_file_get_origin:
 * @input_file: a #GvaInputFile
 *
 * Returns a description of the MAME program that recorded the INP file.
 *
 * Returns: description of the MAME program that recorded the INP file
 **/
const gchar *
gva_input_file_get_origin (GvaInputFile *input_file)
{
        g_return_val_if_fail (GVA_IS_INPUT_FILE (input_file), NULL);

        return input_file->priv->origin;
}

/**
 * gva_input_file_get_timestamp:
 * @input_file: a #GvaInputFile
 *
 * Returns an approximation of when the game recording began.
 *
 * Returns: approximation of when the game recording began
 **/
time_t
gva_input_file_get_timestamp (GvaInputFile *input_file)
{
        g_return_val_if_fail (GVA_IS_INPUT_FILE (input_file), 0);

        return input_file->priv->timestamp;
}

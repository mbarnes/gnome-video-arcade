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
#include <glib/gstdio.h>  /* for g_fopen() */

#define FILE_READAHEAD_BUFFER_SIZE      8192  /* 8 KB */

#define FILE_SMALL_CHUNK                (MAX (BUFSIZ, 8192))
#define FILE_BIG_CHUNK                  (512 * 1024)

/* Newline Types */
#define FILE_NEWLINE_CR         (1 << 0)        /* \r newline seen */
#define FILE_NEWLINE_LF         (1 << 1)        /* \n newline seen */
#define FILE_NEWLINE_CRLF       (1 << 2)        /* \r\n newline seen */

/* XXX long string emits compiler warning */
static gchar file_doc[] =
"file(name[, mode[, buffering]]) -> file object\n\
\n\
Open a file.  The mode can be 'r', 'w' or 'a' for reading (default),\n\
writing or appending.  The file will be created if it doesn't exist\n\
when opened for writing or appending; it will be truncated when\n\
opened for writing.  Add a 'b' to the mode for binary files.\n\
Add a '+' to the mode to allow simultaneous reading and writing.\n\
If the buffering argument is given, 0 means unbuffered, 1 means line\n\
buffered, and larger numbers specify the buffer size.\n\
Add a 'U' to mode to open the file for input with universal newline\n\
support.  Any line ending in the input file will be seen as a '\\n'\n\
in PCL.  Also, a file so opened gains the attribute 'newlines';\n\
the value for this attribute is one of None (no newline read yet),\n\
'\\r', '\\n', '\\r\\n' or a tuple containing all the newline types seen.\n\
\n\
'U' cannot be combined with 'w' or '+' mode.\n\
\n\
Note: open() is an alias for file().";

static gpointer file_parent_class = NULL;

static PclObject *
file_closed_error (void)
{
        pcl_error_set_string (
                pcl_exception_value_error (),
                "I/O operation on closed file");
        return NULL;
}

static PclObject *
file_io_error (FILE *stream)
{
        pcl_error_set_from_errno (
                pcl_exception_io_error ());
        clearerr (stream);
        return NULL;
}

static PclObject *
file_get_line (PclFile *self, gsize len)
{
        PclObject *result;
        GString *buffer;
        gchar *cp, *end;
        gsize increment;
        gsize used_len;
        gint c = 0;

        buffer = g_string_new (NULL);
        g_string_set_size (buffer, len > 0 ? len : 100);
        cp = buffer->str;
        end = cp + buffer->len;

        while (TRUE)
        {
                if (self->univ_newline)
                {
                        while (cp != end)
                        {
                                c = fgetc (self->stream);
                                if (c == EOF)
                                        break;
                                if (self->skip_next_lf)
                                {
                                        self->skip_next_lf = FALSE;
                                        if (c == '\n')
                                        {
                                                self->newline_types |=
                                                        FILE_NEWLINE_CRLF;
                                                c = fgetc (self->stream);
                                                if (c == EOF)
                                                        break;
                                        }
                                        else
                                                self->newline_types |=
                                                        FILE_NEWLINE_CR;
                                }
                                if (c == '\r')
                                {
                                        self->skip_next_lf = 1;
                                        c = '\n';
                                }
                                else if (c == '\n')
                                        self->newline_types |= FILE_NEWLINE_LF;
                                *cp++ = (gchar) c;
                                if (c == '\n')
                                        break;
                        }
                        if (c == EOF && self->skip_next_lf)
                                self->newline_types |= FILE_NEWLINE_CR;
                }
                else
                {
                        while (cp != end)
                        {
                                c = fgetc (self->stream);
                                if (c == EOF)
                                        break;
                                *cp++ = (gchar) c;
                                if (c == '\n')
                                        break;
                        }
                }

                if (c == '\n')
                        break;
                if (c == EOF)
                {
                        if (ferror (self->stream))
                        {
                                g_string_free (buffer, TRUE);
                                return file_io_error (self->stream);
                        }
                        clearerr (self->stream);
                        if (!pcl_error_check_signals ())
                        {
                                g_string_free (buffer, TRUE);
                                return NULL;
                        }
                        break;
                }

                g_assert (cp == end);
                if (len > 0)
                        break;
                used_len = buffer->len;
                increment = buffer->len >> 2;
                g_string_set_size (buffer, buffer->len + increment);
                cp = buffer->str + used_len;
                end = buffer->str + buffer->len;
        }

        used_len = cp - buffer->str;
        result = pcl_string_from_string_and_size (buffer->str, used_len);
        g_string_free (buffer, TRUE);
        return result;
}

static gsize
file_new_buffer_size (PclFile *self, gsize current_size)
{
        struct stat st;

        if (fstat (fileno (self->stream), &st) == 0)
        {
                off_t position, end;

                end = st.st_size;
                position = lseek (fileno (self->stream), 0L, SEEK_CUR);
                if (position >= 0)
                        position = ftell (self->stream);
                if (position < 0)
                        clearerr (self->stream);
                if (end > position && position >= 0)
                        return current_size + end - position + 1;
                /* Add 1 so if the file were to grow we'd notice. */
        }

        if (current_size > FILE_SMALL_CHUNK)
        {
                /* Keep doubling until we reach FILE_BIG_CHUNK, then keep
                 * adding FILE_BIG_CHUNK. */
                if (current_size <= FILE_BIG_CHUNK)
                        return current_size + current_size;
                else
                        return current_size + FILE_BIG_CHUNK;
        }
        return current_size + FILE_SMALL_CHUNK;
}

static gboolean
file_readahead (PclFile *self, gsize buffer_size)
{
        /* Make sure that file has a readahead buffer with at least one byte
         * (unless at EOF) and no more than 'buffer_size'. */

        gsize chunk_size;

        if (self->buffer != NULL)
        {
                if ((self->buffer_end - self->buffer_ptr) >= 1)
                        return TRUE;
                else
                {
                        g_free (self->buffer);
                        self->buffer = NULL;
                }
        }
        self->buffer = g_new (gchar, buffer_size);
        errno = 0;
        chunk_size = pcl_universal_newline_fread (
                self->buffer, buffer_size, self->stream, PCL_OBJECT (self));
        if (chunk_size == 0)
        {
                if (ferror (self->stream))
                {
                        file_io_error (self->stream);
                        g_free (self->buffer);
                        self->buffer = NULL;
                        return FALSE;
                }
        }
        self->buffer_ptr = self->buffer;
        self->buffer_end = self->buffer + chunk_size;
        return TRUE;
}

static PclObject *
file_readahead_get_line_skip (PclFile *self, gsize skip, gsize buffer_size)
{
        /* The returned string will start with 'skip' uninitialized
         * bytes followed by the remainder of the line.  Don't be
         * horrified by the recursive call: maximum recursion depth
         * is limited by logarithmic buffer growth to about 50 even
         * when reading a 1GB line. */

        PclObject *result;
        gchar *buffer_ptr;
        gchar *buffer;
        gsize length;

        if (self->buffer == NULL)
                if (!file_readahead (self, buffer_size))
                        return NULL;
        length = self->buffer_end - self->buffer_ptr;
        if (length == 0)
                return pcl_string_from_string_and_size (NULL, skip);
        buffer_ptr = memchr (self->buffer_ptr, '\n', length);
        if (buffer_ptr != NULL)
        {
                buffer_ptr++;  /* count the '\n' */
                length = buffer_ptr - self->buffer_ptr;
                result = pcl_string_from_string_and_size (NULL, skip + length);
                if (result == NULL)
                        return NULL;
                memcpy (PCL_STRING_AS_STRING (result) + skip,
                        self->buffer_ptr, length);
                self->buffer_ptr = buffer_ptr;
                if (buffer_ptr == self->buffer_end)
                {
                        g_free (self->buffer);
                        self->buffer = NULL;
                }
        }
        else
        {
                buffer_ptr = self->buffer_ptr;
                buffer = self->buffer;
                self->buffer = NULL;  /* force new readahead buffer */
                result = file_readahead_get_line_skip (
                        self, skip + length, buffer_size + (buffer_size >> 2));
                if (result == NULL)
                {
                        g_free (buffer);
                        return NULL;
                }
                memcpy (PCL_STRING_AS_STRING (result) + skip,
                        buffer_ptr, length);
                g_free (buffer);
        }
        return result;
}

static void
file_dispose (GObject *g_object)
{
        PclFile *self = PCL_FILE (g_object);

        PCL_CLEAR (self->name);
        PCL_CLEAR (self->mode);
        PCL_CLEAR (self->encoding);

        /* Chain up to parent's dispose method. */
        G_OBJECT_CLASS (file_parent_class)->dispose (g_object);
}

static void
file_finalize (GObject *g_object)
{
        PclFile *self = PCL_FILE (g_object);

        if (self->stream != NULL && self->close != NULL)
                self->close (self->stream);
        g_free (self->buffer);

        /* Chain up to parent's finalize method. */
        G_OBJECT_CLASS (file_parent_class)->finalize (g_object);
}

static PclObject *
file_new_instance (PclObject *type, PclObject *args, PclObject *kwds)
{
        const gchar *kwlist[] = { "name", "mode", "buffering", NULL };
        gchar *name = NULL;
        gchar *mode = "r";
        gint buffer_size = -1;
        PclObject *file;

        if (!pcl_arg_parse_tuple_and_keywords (args, kwds, "s|si:file",
                                kwlist, &name, &mode, &buffer_size))
                return NULL;
        file = pcl_file_from_string (name, mode);
        if (file == NULL)
                return NULL;
        /* XXX not doing anything with buffer_size */
        return file;
}

static PclObject *
file_repr (PclObject *object)
{
        PclFile *self = PCL_FILE (object);

        return pcl_string_from_format ("<%s file '%s', mode '%s' at %p>",
               (self->stream == NULL) ? "closed" : "open",
               pcl_string_as_string (self->name),
               pcl_string_as_string (self->mode),
               (gpointer) object);
}

static gboolean
file_traverse (PclContainer *container, PclTraverseFunc func,
               gpointer user_data)
{
        PclFile *self = PCL_FILE (container);

        if (self->name != NULL)
                if (!func (self->name, user_data))
                        return FALSE;
        if (self->mode != NULL)
                if (!func (self->mode, user_data))
                        return FALSE;
        if (self->encoding != NULL)
                if (!func (self->encoding, user_data))
                        return FALSE;

        /* Chain up to parent's traverse method. */
        return PCL_CONTAINER_CLASS (file_parent_class)->
                traverse (container, func, user_data);
}

static PclObject *
file_next (PclIterator *iterator)
{
        PclFile *self = PCL_FILE (iterator);
        PclObject *line;

        if (self->stream == NULL)
                return file_closed_error ();
        line = file_readahead_get_line_skip (
                self, 0, FILE_READAHEAD_BUFFER_SIZE);
        if (line == NULL)
                return NULL;
        if (PCL_STRING_GET_SIZE (line) == 0)
        {
                pcl_object_unref (line);
                return NULL;
        }
        return line;
}

static gboolean
file_stop (PclIterator *iterator)
{
        PclFile *self = PCL_FILE (iterator);

        g_free (self->buffer);
        self->buffer = NULL;

        errno = 0;
        if (fseeko (self->stream, 0, SEEK_END) != 0)
        {
                file_io_error (self->stream);
                return FALSE;
        }
        return TRUE;
}

PclObject *
file_get_closed (PclFile *self, gpointer closure)
{
        return pcl_bool_from_boolean (self->stream == NULL);
}

PclObject *
file_get_newlines (PclFile *self, gpointer closure)
{
        switch (self->newline_types)
        {
                case 0:  /* unknown */
                        return pcl_object_ref (PCL_NONE);
                case FILE_NEWLINE_CR:
                        return pcl_string_from_string ("\r");
                case FILE_NEWLINE_LF:
                        return pcl_string_from_string ("\n");
                case FILE_NEWLINE_CR | FILE_NEWLINE_LF:
                        return pcl_build_value ("(ss)", "\r", "\n");
                case FILE_NEWLINE_CRLF:
                        return pcl_string_from_string ("\r\n");
                case FILE_NEWLINE_CR | FILE_NEWLINE_CRLF:
                        return pcl_build_value ("(ss)", "\r", "\r\n");
                case FILE_NEWLINE_LF | FILE_NEWLINE_CRLF:
                        return pcl_build_value ("(ss)", "\n", "\r\n");
                case FILE_NEWLINE_CR | FILE_NEWLINE_LF | FILE_NEWLINE_CRLF:
                        return pcl_build_value ("(sss)", "\r", "\n", "\r\n");
                default:
                        pcl_error_set_format (
                                pcl_exception_system_error (),
                                "unknown newlines value 0x%x\n",
                                self->newline_types);
                        return NULL;
        }
}

PclObject *
file_method_close (PclFile *self)
{
        gint status = 0;

        if (self->stream != NULL)
        {
                if (self->close != NULL)
                {
                        errno = 0;
                        status = self->close (self->stream);
                }
                self->stream = NULL;
        }
        if (status == EOF)
        {
                pcl_error_set_from_errno (
                        pcl_exception_io_error ());
                return NULL;
        }
        if (status != 0)
                return pcl_int_from_long ((glong) status);
        return pcl_object_ref (PCL_NONE);
}

static gchar file_method_close_doc[] =
"close() -> None or (perhaps) an integer.  Close the file\n\
\n\
Sets data attribute .closed to True.  A closed file cannot be used for\n\
further I/O operations.  close() may be called more than once without\n\
error.  Some kinds of file objects (for example, opened by popen())\n\
may return an exit status upon closing.";

PclObject *
file_method_fileno (PclFile *self)
{
        if (self->stream == NULL)
                return file_closed_error ();
        return pcl_int_from_long ((glong) fileno (self->stream));
}

static gchar file_method_fileno_doc[] =
"fileno() -> integer \"file descriptor\".\n\
\n\
This is needed for lower-level file interfaces, such as os.read().";

PclObject *
file_method_flush (PclFile *self)
{
        if (self->stream == NULL)
                return file_closed_error ();
        errno = 0;
        if (fflush (self->stream) != 0)
                return file_io_error (self->stream);
        return pcl_object_ref (PCL_NONE);
}

static gchar file_method_flush_doc[] =
"flush() -> None.  Flush the internal I/O buffer.";

PclObject *
file_method_isatty (PclFile *self)
{
        gboolean is_tty;

        if (self->stream == NULL)
                return file_closed_error ();
        is_tty = (gboolean) isatty (fileno (self->stream));
        return pcl_bool_from_boolean (is_tty);
}

static gchar file_method_isatty_doc[] =
"isatty() -> true or false.  True if the file is connected to a tty device.";

PclObject *
file_method_read (PclFile *self, PclObject *args)
{
        PclObject *buffer;
        glong bytes_requested = -1;
        gsize buffer_size;
        gsize bytes_read = 0;
        gsize chunk_size;

        if (self->stream == NULL)
                return file_closed_error ();
        if (!pcl_arg_parse_tuple (args, "|l:read", &bytes_requested))
                return NULL;
        if (bytes_requested < 0)
                buffer_size = file_new_buffer_size (self, 0);
        else
                buffer_size = (gsize) bytes_requested;
        buffer = pcl_string_from_string_and_size (NULL, buffer_size);
        if (buffer == NULL)
                return NULL;
        while (TRUE)
        {
                errno = 0;
                chunk_size = pcl_universal_newline_fread (
                        PCL_STRING_AS_STRING (buffer) + bytes_read,
                        buffer_size - bytes_read, self->stream,
                        PCL_OBJECT (self));
                if (chunk_size == 0)
                {
                        if (!ferror (self->stream))
                                break;
                        clearerr (self->stream);
                        /* When in non-blocking mode, data shouldn't be
                         * discarded if a blocking signal was received.
                         * That will also happen if chunk_size != 0, but
                         * bytes_read < buffer_size. */
                        if (bytes_read > 0 && errno == EWOULDBLOCK)
                                break;
                        pcl_error_set_from_errno (
                                pcl_exception_io_error ());
                        pcl_object_unref (buffer);
                        return NULL;
                }
                bytes_read += chunk_size;
                if (bytes_read < buffer_size)
                {
                        clearerr (self->stream);
                        break;
                }
                if (bytes_requested < 0)
                {
                        buffer_size = file_new_buffer_size (self, buffer_size);
                        if (!pcl_string_resize (buffer, buffer_size))
                                return NULL;
                }
                else
                                break;
        }
        if (bytes_read != buffer_size)
                pcl_string_resize (buffer, bytes_read);
        return buffer;
}

static gchar file_method_read_doc[] =
"read([size]) -> read at most size bytes, returned as a string\n\
\n\
If the size argument is negative or omitted, read until EOF is reached.\n\
Notice that when in non-blocking mode, less data than what was requested\n\
may be returned, even if no size parameter was given.";

PclObject *
file_method_readline (PclFile *self, PclObject *args)
{
        gint size = -1;

        if (self->stream == NULL)
                return file_closed_error ();
        if (!pcl_arg_parse_tuple (args, "|i:readline", &size))
                return NULL;
        if (size == 0)
                return pcl_string_from_string ("");
        if (size < 0)
                size = 0;
        return file_get_line (self, size);
}

static gchar file_method_readline_doc[] =
"readline([size]) -> next line from the file, as a string\n\
\n\
Retain newline.  A non-negative size argument limits the maximum\n\
number of bytes to return (an incomplete line may be returned then).\n\
Return an empty string at EOF.";

PclObject *
file_method_readlines (PclFile *self, PclObject *args)
{
        PclObject *line;
        PclObject *list;
        GString *big_buffer = NULL;
        gboolean short_read = FALSE;
        gboolean success;
        gchar small_buffer[FILE_SMALL_CHUNK];
        gchar *buffer = small_buffer;
        gsize buffer_size = FILE_SMALL_CHUNK;
        gsize n_filled = 0;
        gsize n_read;
        gsize total_read = 0;
        glong size_hint = 0;
        gchar *p, *q, *end;

        if (self->stream == NULL)
                return file_closed_error ();
        if (!pcl_arg_parse_tuple (args, "|l:readlines", &size_hint))
                return NULL;
        list = pcl_list_new (0);
        if (list == NULL)
                return NULL;
        while (TRUE)
        {
                if (short_read)
                        n_read = 0;
                else
                {
                        errno = 0;
                        n_read = pcl_universal_newline_fread (
                                buffer + n_filled, buffer_size - n_filled,
                                self->stream, PCL_OBJECT (self));
                        short_read = (n_read < buffer_size - n_filled);
                }
                if (n_read == 0)
                {
                        size_hint = 0;
                        if (!ferror (self->stream))
                                break;
                        file_io_error (self->stream);
                        goto fail;
                }
                total_read += n_read;
                p = memchr (buffer + n_filled, '\n', n_read);
                if (p == NULL)
                {
                        /* need a larger buffer to fit this line */
                        n_filled += n_read;
                        buffer_size *= 2;
                        if (big_buffer == NULL)
                        {
                                /* create the big buffer */
                                big_buffer = g_string_new (NULL);
                                g_string_set_size (big_buffer, buffer_size);
                                buffer = big_buffer->str;
                                memcpy (buffer, small_buffer, n_filled);
                        }
                        else
                        {
                                /* grow the big buffer */
                                g_string_set_size (big_buffer, buffer_size);
                                buffer = big_buffer->str;
                        }
                        continue;
                }
                end = buffer + n_filled + n_read;
                q = buffer;
                do
                {
                        /* process complete lines */
                        p++;
                        line = pcl_string_from_string_and_size (q, p - q);
                        if (line == NULL)
                                goto fail;
                        success = pcl_list_append (list, line);
                        pcl_object_unref (line);
                        if (!success)
                                goto fail;
                        q = p;
                        p = memchr (q, '\n', end - q);
                }
                while (p != NULL);

                /* move the remaining incomplete line to the start */
                n_filled = end - q;
                memmove (buffer, q, n_filled);
                if (size_hint > 0 && total_read >= (gsize) size_hint)
                        break;
        }

        if (n_filled != 0)
        {
                /* partial last line */
                line = pcl_string_from_string_and_size (buffer, n_filled);
                if (line == NULL)
                        goto fail;
                if (size_hint > 0)
                {
                        /* need to complete the last line */
                        PclObject *rest = file_get_line (self, 0);
                        if (rest == NULL)
                        {
                                pcl_object_unref (line);
                                goto fail;
                        }
                        pcl_string_concat_and_del (&line, rest);
                        if (line == NULL)
                                goto fail;
                }
                success = pcl_list_append (list, line);
                pcl_object_unref (line);
                if (!success)
                        goto fail;
        }

        goto exit;

fail:
        pcl_object_unref (list);
        list = NULL;

exit:
        if (big_buffer != NULL)
                g_string_free (big_buffer, TRUE);
        return list;
}

static gchar file_method_readlines_doc[] =
"readlines([size]) -> list of strings, each a line from the file\n\
\n\
Call readline() repeatedly and return a list of the lines so read.\n\
The optional size argument, if given, is an approximate bound on the\n\
total number of bytes in the lines returned.";

PclObject *
file_method_seek (PclFile *self, PclObject *args)
{
        PclObject *object;
        gint whence = 0;
        off_t offset;

        if (self->stream == NULL)
                return file_closed_error ();
        g_free (self->buffer);
        self->buffer = NULL;
        if (!pcl_arg_parse_tuple (args, "O:i:seek", &object, &whence))
                return NULL;
        offset = (off_t) pcl_int_as_long (object);
        if (pcl_error_occurred ())
                return NULL;
        errno = 0;
        if (fseeko (self->stream, offset, whence) != 0)
                return file_io_error (self->stream);
        self->skip_next_lf = FALSE;
        return pcl_object_ref (PCL_NONE);
}

/* XXX long string emits compiler warning */
static gchar file_method_seek_doc[] =
"seek(offset[, whence]) -> None.  Move to new file position.\n\
\n\
Argument offset is a byte count.  Optional argument whence defaults to\n\
0 (offset from start of file, offset should be >=0); other values are 1\n\
(move relative to current position, positive or negative), and 2 (move\n\
relative to end of file, usually negative, although many platforms allow\n\
seeking beyond the end of a file).  If the file is opened in text move,\n\
only offsets returned by tell() are legal.  Use of other offsets causes\n\
undefined behavior.\n\
\n\
Note that not all file objects are seekable.";

PclObject *
file_method_tell (PclFile *self)
{
        glong position;

        if (self->stream == NULL)
                return file_closed_error ();
        errno = 0;
        position = ftell (self->stream);        
        if (position < 0)
                return file_io_error (self->stream);
        if (self->skip_next_lf)
        {
                gint c;

                c = fgetc (self->stream);
                if (c == '\n')
                {
                        position++;
                        self->skip_next_lf = FALSE;
                }
                else if (c != EOF)
                        ungetc (c, self->stream);
        }
        return pcl_int_from_long (position);
}

static gchar file_method_tell_doc[] =
"tell() -> current file position, an integer (may be a long integer).";

PclObject *
file_method_truncate (PclFile *self, PclObject *args)
{
        PclObject *object = NULL;
        off_t length, offset;

        if (self->stream == NULL)
                return file_closed_error ();
        if (!pcl_arg_unpack_tuple (args, "truncate", 0, 1, &object))
                return NULL;

        errno = 0;
        offset = ftello (self->stream);
        if (offset < 0)
                return file_io_error (self->stream);
        if (object != NULL)
                length = (off_t) pcl_int_as_long (object);
        else
                length = offset;

        /* Flush the stream.  We're mixing stream-level I/O with lower-level
         * I/O, and a flush may be necessary to synch both platform views of
         * the current file state. */
        errno = 0;
        if (fflush (self->stream) != 0)
                return file_io_error (self->stream);

        errno = 0;
        if (ftruncate (fileno (self->stream), length) != 0)
                return file_io_error (self->stream);

        /* Restore original file position. */
        errno = 0;
        if (fseeko (self->stream, offset, SEEK_SET) != 0)
                return file_io_error (self->stream);

        return pcl_object_ref (PCL_NONE);
}

static gchar file_method_truncate_doc[] =
"truncate([size]) -> None.  Truncate the file to at most size bytes.\n\
\n\
Size defaults to the current file position, as returned by tell().";

PclObject *
file_method_write (PclFile *self, PclObject *args)
{
        gchar *string;
        gsize bytes_written;
        gsize size;

        if (self->stream == NULL)
                return file_closed_error ();
        if (!pcl_arg_parse_tuple (args, "s#", &string, &size))
                return NULL;
        self->soft_space = FALSE;
        errno = 0;
        bytes_written = fwrite (string, 1, size, self->stream);
        if (bytes_written != size)
                return file_io_error (self->stream);
        return pcl_object_ref (PCL_NONE);
}

static gchar file_method_write_doc[] =
"write(str) -> None.  Write string str to file.\n\
\n\
Note that due to buffering, flush() or close() may be needed before\n\
the file on disk reflects the data written.";

PclObject *
file_method_writelines (PclFile *self, PclObject *sequence)
{
        const glong chunk_size = 1000; 
        PclObject *iterator;
        PclObject *list;
        PclObject *next;
        PclObject *result = NULL;

        if (self->stream == NULL)
                return file_closed_error ();

        iterator = pcl_object_iterate (sequence);
        if (iterator == NULL)
        {
                pcl_error_set_string (
                        pcl_exception_type_error (),
                        "writelines() requires an iterable argument");
                return NULL;
        }

        list = pcl_list_new (chunk_size);
        if (list == NULL)
        {
                pcl_object_unref (iterator);
                return NULL;
        }

        while (TRUE)
        {
                glong ii, jj;

                for (ii = 0; ii < chunk_size; ii++)
                {
                        next = pcl_iterator_next (iterator);
                        if (next == NULL)
                        {
                                if (pcl_error_occurred ())
                                        goto fail;
                                break;
                        }
                        pcl_list_set_item (list, ii, next);
                }

                if (ii == 0)
                        break;

                /* Check that all entries are indeed strings.  If not,
                 * apply the same rules as for file.write() and convert
                 * the results to strings.  This is slow, but seems to
                 * be the only way since all conversion APIs could
                 * potentially execute PCL code. */
                for (jj = 0; jj < ii; jj++)
                {
                        next = PCL_LIST_GET_ITEM (list, jj);
                        if (!PCL_IS_STRING (next))
                        {
                                /* XXX Skipping buffer conversion. */
                                pcl_error_set_string (
                                        pcl_exception_type_error (),
                                        "writelines() argument must be a "
                                        "sequence of strings");
                                goto fail;
                        }
                }

                errno = 0;
                self->soft_space = FALSE;
                for (jj = 0; jj < ii; jj++)
                {
                        gsize n_bytes, n_written;
                        next = PCL_LIST_GET_ITEM (list, jj);
                        n_bytes = PCL_STRING_GET_SIZE (next);
                        n_written = fwrite (
                                PCL_STRING_AS_STRING (next),
                                1, n_bytes, self->stream);
                        if (n_written != n_bytes)
                        {
                                file_io_error (self->stream);
                                goto fail;
                        }
                }

                if (ii < chunk_size)
                        break;
        }

        result = pcl_object_ref (PCL_NONE);

fail:
        pcl_object_unref (list);
        pcl_object_unref (iterator);
        return result;
}

static gchar file_method_writelines_doc[] =
"writelines(sequence_of_strings) -> None.  Write the strings to the file.\n\
\n\
Note that newlines are not added.  The sequence can be any iterable object\n\
producing strings.  This is equivalent to calling write() for each string.";

static void
file_class_init (PclFileClass *class)
{
        PclIteratorClass *iterator_class;
        PclContainerClass *container_class;
        PclObjectClass *object_class;
        GObjectClass *g_object_class;

        file_parent_class = g_type_class_peek_parent (class);

        iterator_class = PCL_ITERATOR_CLASS (class);
        iterator_class->next = file_next;
        iterator_class->stop = file_stop;

        container_class = PCL_CONTAINER_CLASS (class);
        container_class->traverse = file_traverse;

        object_class = PCL_OBJECT_CLASS (class);
        object_class->type = pcl_file_get_type_object;
        object_class->new_instance = file_new_instance;
        object_class->repr = file_repr;
        object_class->doc = file_doc;

        g_object_class = G_OBJECT_CLASS (class);
        g_object_class->dispose = file_dispose;
        g_object_class->finalize = file_finalize;
}

static PclGetSetDef file_getsets[] = {
        { "closed",             (PclGetFunc) file_get_closed,
                                (PclSetFunc) NULL,
                                "True if the file is closed" },
        { "newlines",           (PclGetFunc) file_get_newlines,
                                (PclSetFunc) NULL,
                                "end-of-line convention used in this file" },
        { NULL }
};

static PclMemberDef file_members[] = {
        { "encoding",           G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclFile, encoding),
                                PCL_MEMBER_FLAG_READONLY,
                                "file encoding" },
        { "mode",               G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclFile, mode),
                                PCL_MEMBER_FLAG_READONLY,
                                "file mode ('r', 'U', 'w', 'a', "
                                "possibly with 'b' or '+' added)" },
        { "name",               G_TYPE_OBJECT,
                                G_STRUCT_OFFSET (PclFile, name),
                                PCL_MEMBER_FLAG_READONLY,
                                "file name" },
        { "softspace",          G_TYPE_BOOLEAN,
                                G_STRUCT_OFFSET (PclFile, soft_space),
                                0,
                                "flag indicating that a space needs to be "
                                "printed; used by print" },
        { NULL }
};

static PclMethodDef file_methods[] = {
        { "close",              (PclCFunction) file_method_close,
                                PCL_METHOD_FLAG_NOARGS,
                                file_method_close_doc },
        { "fileno",             (PclCFunction) file_method_fileno,
                                PCL_METHOD_FLAG_NOARGS,
                                file_method_fileno_doc },
        { "flush",              (PclCFunction) file_method_flush,
                                PCL_METHOD_FLAG_NOARGS,
                                file_method_flush_doc },
        { "isatty",             (PclCFunction) file_method_isatty,
                                PCL_METHOD_FLAG_NOARGS,
                                file_method_isatty_doc },
        { "read",               (PclCFunction) file_method_read,
                                PCL_METHOD_FLAG_VARARGS,
                                file_method_read_doc },
        { "readline",           (PclCFunction) file_method_readline,
                                PCL_METHOD_FLAG_VARARGS,
                                file_method_readline_doc },
        { "readlines",          (PclCFunction) file_method_readlines,
                                PCL_METHOD_FLAG_VARARGS,
                                file_method_readlines_doc },
        { "seek",               (PclCFunction) file_method_seek,
                                PCL_METHOD_FLAG_VARARGS,
                                file_method_seek_doc },
        { "tell",               (PclCFunction) file_method_tell,
                                PCL_METHOD_FLAG_NOARGS,
                                file_method_tell_doc },
        { "truncate",           (PclCFunction) file_method_truncate,
                                PCL_METHOD_FLAG_VARARGS,
                                file_method_truncate_doc },
        { "write",              (PclCFunction) file_method_write,
                                PCL_METHOD_FLAG_VARARGS,
                                file_method_write_doc },
        { "writelines",         (PclCFunction) file_method_writelines,
                                PCL_METHOD_FLAG_ONEARG,
                                file_method_writelines_doc },
        { NULL }
};

GType
pcl_file_get_type (void)
{
        static GType type = 0;
        if (G_UNLIKELY (type == 0))
        {
                static const GTypeInfo type_info = {
                        sizeof (PclFileClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) file_class_init,
                        (GClassFinalizeFunc) NULL,
                        NULL,  /* class_data */
                        sizeof (PclFile),
                        0,     /* n_preallocs */
                        (GInstanceInitFunc) NULL,
                        NULL   /* value_table */
                };

                type = g_type_register_static (
                        PCL_TYPE_ITERATOR, "PclFile", &type_info, 0);
                g_type_set_qdata (
                        type, PCL_DATA_GETSETS, file_getsets);
                g_type_set_qdata (
                        type, PCL_DATA_MEMBERS, file_members);
                g_type_set_qdata (
                        type, PCL_DATA_METHODS, file_methods);
        }
        return type;
}

/**
 * pcl_file_get_type_object:
 *
 * Returns the type object for #PclFile.  During runtime this is the built-in
 * object %file (and %open).
 *
 * Returns: a borrowed reference to the type object for #PclFile
 */
PclObject *
pcl_file_get_type_object (void)
{
        static gpointer object = NULL;
        if (G_UNLIKELY (object == NULL))
        {
                object = pcl_type_new (PCL_TYPE_FILE, "file");
                pcl_register_singleton ("<type 'file'>", &object);
        }
        return object;
}

FILE *
pcl_file_as_file (PclObject *file)
{
        if (!PCL_IS_FILE (file))
                return NULL;
        return PCL_FILE (file)->stream;
}

PclObject *
pcl_file_name (PclObject *file)
{
        if (!PCL_IS_FILE (file))
                return NULL;
        return PCL_FILE (file)->name;
}

gboolean
pcl_file_soft_space (PclObject *file, gboolean new_flag)
{
        PclObject *object;
        gboolean old_flag = FALSE;

        if (file == NULL)
                return old_flag;

        if (PCL_IS_FILE (file))
        {
                PclFile *self = PCL_FILE (file);
                old_flag = self->soft_space;
                self->soft_space = new_flag;
                return old_flag;
        }

        object = pcl_object_get_attr_string (file, "softspace");
        if (object == NULL)
                pcl_error_clear ();
        else
        {
                if (PCL_IS_INT (object))
                        old_flag = (gboolean) pcl_int_as_long (object);
                pcl_object_unref (object);
        }
        object = pcl_int_from_long ((glong) new_flag);
        if (object == NULL)
                pcl_error_clear ();
        else
        {
                if (!pcl_object_set_attr_string (file, "softspace", object))
                        pcl_error_clear ();
                pcl_object_unref (object);
        }
        return old_flag;
}

PclObject *
pcl_file_from_file (FILE *stream, const gchar *name, const gchar *mode,
                    gint (*close) (FILE *))
{
        PclFile *file;
        gint fd = fileno (stream);
        struct stat st;

        /* XXX no GLib support for fstat() */
        if (fstat (fd, &st) == 0 && S_ISDIR (st.st_mode))
        {
                errno = EISDIR;
                pcl_error_set_from_errno (
                        pcl_exception_io_error ());
                return NULL;
        }

        file = pcl_object_new (PCL_TYPE_FILE, NULL);
        file->stream = stream;
        file->name = pcl_string_from_string (name);
        file->mode = pcl_string_from_string (mode);
        file->encoding = pcl_object_ref (PCL_NONE);
        file->close = close;
        file->binary = (strchr (mode, 'b') != NULL);
        file->univ_newline = (strchr (mode, 'U') != NULL);
        return PCL_OBJECT (file);
}

PclObject *
pcl_file_from_string (const gchar *name, const gchar *mode)
{
        PclObject *file;
        FILE *stream;

        if (strcmp (mode, "U") == 0 || strcmp (mode, "rU") == 0)
                mode = "rb";

        errno = 0;
        stream = g_fopen (name, mode);
        if (stream == NULL)
        {
                pcl_error_set_from_errno (
                        pcl_exception_io_error ());
                return NULL;
        }

        file = pcl_file_from_file (stream, name, mode, fclose);
        if (file == NULL)
                fclose (stream);
        return file;
}

PclObject *
pcl_file_get_line (PclObject *file, gint n_bytes)
{
        PclObject *result;

        if (file == NULL)
        {
                pcl_error_bad_internal_call ();
                return NULL;
        }

        if (PCL_IS_FILE (file))
        {
                PclFile *self = PCL_FILE (file);

                if (self->stream == NULL)
                        return file_closed_error ();
                result = NULL;  /* FIXME */
        }
        else
        {
                PclObject *args;
                PclObject *reader;

                reader = pcl_object_get_attr_string (file, "readline");
                if (reader == NULL)
                        return NULL;
                if (n_bytes <= 0)
                        args = pcl_tuple_new (0);
                else
                        args = pcl_build_value ("(i)", n_bytes);
                if (args == NULL)
                {
                        pcl_object_unref (reader);
                        return NULL;
                }
                result = pcl_object_call_object (reader, args);
                pcl_object_unref (reader);
                pcl_object_unref (args);
                if (result != NULL && !PCL_IS_STRING (result))
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "object.readline() returned non-string");
                        pcl_object_unref (result);
                        result = NULL;
                }
        }

        if (n_bytes < 0 && PCL_IS_STRING (result))
        {
                gchar *string = PCL_STRING_AS_STRING (result);
                glong size = PCL_STRING_GET_SIZE (result);

                if (size == 0)
                {
                        pcl_error_set_string (
                                pcl_exception_eof_error (),
                                "EOF when reading a line");
                        pcl_object_unref (result);
                        result = NULL;
                }
                else if (string[size - 1] == '\n')
                {
                        /* XXX skipping shortcut if ref_count == 1 */
                        PclObject *swap;
                        swap = pcl_string_from_string_and_size (
                                string, size - 1);
                        pcl_object_unref (result);
                        result = swap;
                }
        }

        return result;
}

gboolean
pcl_file_set_encoding (PclObject *file, const gchar *encoding)
{
        PclFile *self = PCL_FILE (file);
        PclObject *string;

        string = pcl_string_from_string (encoding);
        if (string == NULL)
                return FALSE;
        pcl_object_unref (self->encoding);
        self->encoding = string;
        return TRUE;
}

gboolean
pcl_file_write_object (PclObject *file, PclObject *object, gint flags)
{
        PclObject *args;
        PclObject *result;
        PclObject *writer;
        PclObject *value;

        if (file == NULL)
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        if (PCL_IS_FILE (file))
        {
                FILE *stream = pcl_file_as_file (file);

                if (stream == NULL)
                {
                        file_closed_error ();
                        return FALSE;
                }
                return pcl_object_print (object, stream, flags);
        }

        writer = pcl_object_get_attr_string (file, "write");
        if (writer == NULL)
                return FALSE;
        if (flags & PCL_PRINT_FLAG_RAW)
                value = pcl_object_str (object);
        else
                value = pcl_object_repr (object);
        if (value == NULL)
        {
                pcl_object_unref (writer);
                return FALSE;
        }
        args = pcl_tuple_pack (1, value);
        if (args == NULL)
        {
                pcl_object_unref (value);
                pcl_object_unref (writer);
                return FALSE;
        }
        result = pcl_object_call_object (writer, args);
        pcl_object_unref (args);
        pcl_object_unref (value);
        pcl_object_unref (writer);
        if (result == NULL)
                return FALSE;
        pcl_object_unref (result);
        return TRUE;
}

gboolean
pcl_file_write_string (PclObject *file, const gchar *string)
{
        if (file == NULL)
        {
                pcl_error_bad_internal_call ();
                return FALSE;
        }

        if (PCL_IS_FILE (file))
        {
                FILE *stream = pcl_file_as_file (file);

                if (stream == NULL)
                {
                        file_closed_error ();
                        return FALSE;
                }
                fputs (string, stream);
                return TRUE;
        }

        if (!pcl_error_occurred ())
        {
                PclObject *object;
                gboolean success;

                object = pcl_string_from_string (string);
                if (object == NULL)
                        return FALSE;
                success = pcl_file_write_object (
                        file, object, PCL_PRINT_FLAG_RAW);
                pcl_object_unref (object);
                return success;
        }

        return FALSE;
}

gint
pcl_file_as_file_descriptor (PclObject *object)
{
        gint fd;

        if (PCL_IS_INT (object))
                fd = (gint) PCL_INT_AS_LONG (object);
        else
        {
                PclObject *method;

                method = pcl_object_get_attr_string (object, "fileno");
                if (method != NULL)
                {
                        PclObject *result;

                        result = pcl_object_call_object (method, NULL);
                        pcl_object_unref (method);
                        if (result == NULL)
                                return -1;
                        if (!PCL_IS_INT (result))
                        {
                                pcl_error_set_string (
                                        pcl_exception_type_error (),
                                        "fileno() returned a non-integer");
                                pcl_object_unref (result);
                                return -1;
                        }
                        fd = (gint) PCL_INT_AS_LONG (result);
                        pcl_object_unref (result);
                }
                else
                {
                        pcl_error_set_string (
                                pcl_exception_type_error (),
                                "argument must be an int, "
                                "or have a fileno() method");
                        return -1;
                }
        }

        if (fd < 0)
        {
                pcl_error_set_format (
                        pcl_exception_value_error (),
                        "file descriptor cannot be a "
                        "negative integer (%i)", fd);
                return -1;
        }
        return fd;
}

gchar *
pcl_universal_newline_fgets (
        gchar *buffer, gsize n_bytes, FILE *stream, PclObject *object)
{
        gboolean skip_next_lf = FALSE;
        gboolean univ_newline = TRUE;
        gint newline_types = 0;
        gchar *cp = buffer;
        gchar c;

        if (object != NULL)
        {
                if (!PCL_IS_FILE (object))
                {
                        errno = ENXIO;
                        return 0;
                }
                univ_newline = PCL_FILE (object)->univ_newline;
                if (!univ_newline)
                        return fgets (buffer, n_bytes, stream);
                newline_types = PCL_FILE (object)->newline_types;
                skip_next_lf = PCL_FILE (object)->skip_next_lf;
        }

        c = fgetc (stream);
        while (n_bytes > 0 && c != EOF)
        {
                if (skip_next_lf)
                {
                        skip_next_lf = FALSE;
                        if (c == '\n')
                        {
                                /* Seeing a LF here with skip_next_lf true
                                 * means we saw a CR before. */
                                newline_types |= FILE_NEWLINE_CRLF;
                                c = fgetc (stream);
                                if (c == EOF)
                                        break;
                        }
                        else
                                newline_types |= FILE_NEWLINE_CR;
                }
                if (c == '\r')
                {
                        /* A CR is translated into a LF, and we skip
                         * an adjacent LF, if any.  We don't set the
                         * newline flag until we've seen the next char. */
                        skip_next_lf = TRUE;
                        c = '\n';
                }
                else if (c == '\n')
                        newline_types |= FILE_NEWLINE_LF;
                *cp++ = c;
                if (c == '\n')
                        break;

                c = fgetc (stream);
                n_bytes--;
        }

        if (c == EOF && skip_next_lf)
                newline_types |= FILE_NEWLINE_CR;
        *cp = '\0';

        if (object != NULL)
        {
                PCL_FILE (object)->newline_types = newline_types;
                PCL_FILE (object)->skip_next_lf = skip_next_lf;
        }
        else if (skip_next_lf)
        {
                /* If we have no file object we cannot save the skip_next_lf
                 * flag.  We have to read ahead, which will cause a pause if
                 * we're reading from an interactive stream.  But that is
                 * very unlikely unless we're doing something silly like
                 * execfile("/dev/tty"). */
                c = fgetc (stream);
                if (c != '\n')
                        ungetc (c, stream);
        }
        if (cp == buffer)
                return NULL;
        return buffer;
}

gsize
pcl_universal_newline_fread (
        gchar *buffer, gsize n_bytes, FILE *stream, PclObject *object)
{
        gchar *dst = buffer;
        gboolean skip_next_lf;
        gint newline_types;
        PclFile *file;

        g_assert (buffer != NULL);
        g_assert (stream != NULL);

        if (!PCL_IS_FILE (object))
        {
                errno = ENXIO;
                return 0;
        }

        file = PCL_FILE (object);
        if (!file->univ_newline)
                return fread (buffer, 1, n_bytes, stream);
        newline_types = file->newline_types;
        skip_next_lf = file->skip_next_lf;

        /* Invariant: n_bytes is the number of bytes remaining
         *            to be filled in the buffer. */
        while (n_bytes > 0)
        {
                gsize n_read;
                gboolean short_read;
                gchar *src = dst;

                n_read = fread (dst, 1, n_bytes, stream);
                g_assert (n_read <= n_bytes);
                if (n_read == 0)
                        break;

                n_bytes -= n_read;
                short_read = (n_bytes != 0);
                while (n_read-- > 0)
                {
                        gchar c = *src++;

                        if (c == '\r')
                        {
                                /* Save as LF and set flag to skip next LF. */
                                *dst++ = '\n';
                                skip_next_lf = TRUE;
                        }
                        else if (skip_next_lf && c == '\n')
                        {
                                /* Skip LF, and remember we saw CR LF. */
                                skip_next_lf = FALSE;
                                newline_types |= FILE_NEWLINE_CRLF;
                                n_read++;
                        }
                        else
                        {
                                /* Normal char to be stored in buffer.  Also
                                 * update the newline_types flag if either this
                                 * is an LF or the previous char was a CR. */
                                if (c == '\n')
                                        newline_types |= FILE_NEWLINE_LF;
                                else if (skip_next_lf)
                                        newline_types |= FILE_NEWLINE_CR;
                                skip_next_lf = FALSE;
                                *dst++ = c;
                        }
                }
                if (short_read)
                {
                        /* If this is EOF, update type flags. */
                        if (skip_next_lf && feof (stream))
                                newline_types |= FILE_NEWLINE_CR;
                        break;
                }
        }

        file->newline_types = newline_types;
        file->skip_next_lf = skip_next_lf;
        return dst - buffer;
}

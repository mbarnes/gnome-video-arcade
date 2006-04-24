# GDB Initialization File
# Requires GDB 6.2 or newer

set $gtrap_last = 0

define gtrap
  if $argc > 0
    set $gtrap_last = $arg0
  end
  set g_trap_object_ref = $gtrap_last
end

define gtrace
  if $argc > 0
    set $gtrap_last = $arg0
  end
  start
  gtrap
  while 1
    continue
    backtrace
  end
end

document gtrap
Trap g_object_ref() or g_object_unref() calls on the specified GObject.
end

document gtrace
Perform a complete reference count trace on the specified GObject.
This restarts the current process and runs it to completion.
end

set environment GOBJECT_DEBUG = objects

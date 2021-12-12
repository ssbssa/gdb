#ifndef TUI_TUI_OUTPUT_H
#define TUI_TUI_OUTPUT_H

#include <gdbsupport/managed-tty.h>

#if GDB_MANAGED_TERMINALS

#include "tui/tui-output-base.h"

/* The TUI output window.  */
struct tui_output_window : public tui_output_base_window
{
  tui_output_window ();

  DISABLE_COPY_AND_ASSIGN (tui_output_window);

  const char *name () const override
  {
    return OUTPUT_NAME;
  }
};

bool tui_output_write (const char *buf, long length_buf);

#endif

#endif

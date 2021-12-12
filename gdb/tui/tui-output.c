
#include "defs.h"
#include <tui/tui-output.h>

#if GDB_MANAGED_TERMINALS

#include <tui/tui-command.h>
#include <tui/tui-io.h>
#include <tui/tui-wingeneral.h>

static std::vector<std::string> app_output;

tui_output_window::tui_output_window ()
     : tui_output_base_window(app_output)
{
  title = OUTPUT_NAME;
}

bool
tui_output_write (const char *buf, long length_buf)
{
  if (buf != nullptr && length_buf > 0)
    add_to_output (buf, length_buf, app_output);

  if (!tui_is_window_visible (OUTPUT_WIN))
    return false;

  TUI_OUTPUT_WIN->refresh (true);

  return true;
}

#endif

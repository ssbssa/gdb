
#include "defs.h"
#include <tui/tui-cmd-history.h>
#include <tui/tui-command.h>
#include <tui/tui-io.h>
#include <tui/tui-wingeneral.h>
#include <readline/readline.h>

static std::vector<std::string> cmd_history;

tui_cmd_history_window::tui_cmd_history_window ()
     : tui_output_base_window(cmd_history)
{
  set_title (CMD_HISTORY_NAME);
}

void
cmd_history_ui_file::write (const char *buf, long length_buf)
{
  write_to_cmd_history (buf, length_buf);
}

void
write_to_cmd_history (const char *buf, long length_buf)
{
  add_to_output (buf, length_buf, cmd_history);
}

bool
tui_cmd_history_refresh ()
{
  if (!tui_is_window_visible (CMD_HISTORY_WIN))
    return false;

  TUI_CMD_HISTORY_WIN->refresh ();

  return true;
}

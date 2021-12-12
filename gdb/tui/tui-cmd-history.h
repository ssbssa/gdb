/* command history window.  */

#ifndef GDB_TUI_TUI_CMD_HISTORY_H
#define GDB_TUI_TUI_CMD_HISTORY_H

#include "tui/tui-output-base.h"

/* The TUI command history window.  */
struct tui_cmd_history_window : public tui_output_base_window
{
  tui_cmd_history_window ();

  DISABLE_COPY_AND_ASSIGN (tui_cmd_history_window);

  const char *name () const override
  {
    return CMD_HISTORY_NAME;
  }
};

inline tui_cmd_history_window *
tui_cmd_history_win ()
{
  return dynamic_cast<tui_cmd_history_window *>
    (tui_win_list[CMD_HISTORY_WIN]);
}

class cmd_history_ui_file : public ui_file
{
public:
  void write (const char *buf, long length_buf) override;
};

void write_to_cmd_history (const char *buf, long length_buf);

bool tui_cmd_history_refresh ();

#endif /* GDB_TUI_TUI_CMD_HISTORY_H */

#ifndef TUI_TUI_OUTPUT_BASE_H
#define TUI_TUI_OUTPUT_BASE_H

#include "tui/tui-data.h"

#include <string>

struct tui_output_base_window : public tui_win_info
{
  tui_output_base_window (const std::vector<std::string> &output)
    : m_y_ofs(0), m_at_bottom(true), m_last_x(-1), m_last_y(-1),
      m_move_cursor(false), m_output(output) {}

  DISABLE_COPY_AND_ASSIGN (tui_output_base_window);

  void refresh (bool = false);

protected:

  void do_scroll_vertical (int num_to_scroll) override;
  void do_scroll_horizontal (int num_to_scroll) override;

  void rerender () override;

private:
  int m_y_ofs;
  bool m_at_bottom;
  int m_last_x, m_last_y;
  bool m_move_cursor;
  const std::vector<std::string> &m_output;
};

void add_to_output (const char *buf, long length_buf,
		    std::vector<std::string> &output);

#endif

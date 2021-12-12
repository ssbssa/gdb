
#include "defs.h"
#include <tui/tui-output-base.h>
#include <tui/tui-io.h>
#include <tui/tui-wingeneral.h>
#include <readline/readline.h>

void
tui_output_base_window::refresh (bool move_cursor)
{
  m_move_cursor = move_cursor;
  if (m_move_cursor)
    m_at_bottom = true;
  rerender ();
  m_move_cursor = false;
}

void
tui_output_base_window::do_scroll_vertical (int num_to_scroll)
{
  m_y_ofs += num_to_scroll;
  if (m_y_ofs < 0)
    m_y_ofs = 0;
  else if (!m_output.empty () && m_y_ofs >= m_output.size())
    m_y_ofs = m_output.size () - 1;
  m_at_bottom = false;

  rerender();
}

void
tui_output_base_window::do_scroll_horizontal (int num_to_scroll)
{
}

void
tui_output_base_window::rerender ()
{
  if (m_at_bottom && height - 2 + m_y_ofs < m_output.size ())
    m_y_ofs = m_output.size () - (height - 2);

  check_and_display_highlight_if_needed ();
  m_last_x = m_last_y = -1;
  for (unsigned i = 0; i < height - 2; ++i)
    {
      wmove (handle.get (), i + 1, 1);
      int printed;
      tui_puts (i + m_y_ofs < m_output.size ()
		? m_output[i + m_y_ofs].c_str () : "",
		handle.get (), width - 2, &printed);
      if (i + m_y_ofs + 1 == m_output.size ())
	{
	  m_last_x = printed;
	  m_last_y = i;
	}
    }
  if (m_move_cursor && m_last_x >= 0)
    wmove (handle.get (), m_last_y + 1, m_last_x + 1);
  tui_wrefresh (handle.get ());

  m_at_bottom = height - 2 + m_y_ofs >= m_output.size ();
}

static int
split_line_pos (std::string &line, int w, std::string &last_m)
{
  int c = 0;
  const char *s = line.c_str ();
  int l = line.size ();
  int last_m_start = -1;
  int last_m_size = 0;
  for (int i = 0; i < l; ++i)
    {
      if (s[i] == '\x1b' && (i + 1 >= l || s[i + 1] == '['))
	{
	  int esc_start = i;
	  for (i += 2; i < l; ++i)
	    if (s[i] >= 0x40 && s[i] <= 0x7e)
	      break;
	  if (i < l && s[i] == 'm')
	    {
	      last_m_start = esc_start;
	      last_m_size = i + 1 - esc_start;
	    }
	  continue;
	}
      else if (s[i] == '\t')
	{
	  int spaces = 8 - i % 8;
	  if (spaces == 1)
	    line[i] = ' ';
	  else
	    {
	      line.replace (i, 1, spaces, ' ');
	      s = line.data ();
	      l = line.size ();
	    }
	}

      ++c;
      if (c >= w && i + 1 < l)
	{
	  if (last_m_start >= 0)
	    last_m = line.substr (last_m_start, last_m_size);
	  else
	    last_m.clear ();
	  return i + 1;
	}
    }
  return -1;
}

void
add_to_output (const char *buf, long length_buf,
	       std::vector<std::string> &output)
{
  int screen_w;
  rl_get_screen_size (NULL, &screen_w);
  int w = screen_w - 2;

  if (output.empty ())
    output.emplace_back();

  const char *p = buf;
  int r = length_buf;
  while (r > 0)
    {
      while (output.size () > 1000)
	output.erase (output.begin ());

      const char *nl = (const char *) memchr (p, '\n', r);
      int count;
      bool has_newline;
      if (nl)
	{
	  has_newline = true;
	  count = nl - p;
	}
      else
	{
	  has_newline = false;
	  count = r;
	}

      const char *back = NULL;
      if (count > 0 && (back = (const char *) memchr (p, '\b', count)) != NULL)
	{
	  has_newline = false;
	  count = back - p;
	}

      std::string &last_line = output.back ();
      if (count > 0)
	last_line.append (p, count);

      r -= count;
      p += count;
      if (has_newline)
	{
	  --r;
	  ++p;

	  if (!last_line.empty () && last_line.back () == '\r')
	    last_line.erase (last_line.end () - 1);
	}
      if (back != NULL)
	{
	  --r;
	  ++p;
	  if (!last_line.empty ())
	    last_line.erase (last_line.end () - 1);
	}

      if (count > 0)
	{
	  int split_pos;
	  std::string m;
	  while ((split_pos = split_line_pos (output.back (), w, m)) >= 0)
	    {
	      std::string part = m + output.back ().substr (split_pos);
	      output.back ().erase (split_pos);
	      output.push_back (part);
	    }
	}

      if (has_newline)
	output.emplace_back();
    }
}

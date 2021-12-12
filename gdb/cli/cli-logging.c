/* Command-line output logging for GDB, the GNU debugger.

   Copyright (C) 2003-2026 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "cli/cli-cmds.h"
#include "ui-out.h"
#include "interps.h"
#include "logging-file.h"
#include "cli/cli-style.h"
#include "cli/cli-decode.h"

#ifdef TUI
#include "tui/tui-cmd-history.h"
#endif

static std::string saved_filename;

static void
maybe_warn_already_logging ()
{
  if (!saved_filename.empty ())
    warning (_("Currently logging to %ps.  Turn the logging off and on to "
	       "make the new setting effective."),
	     styled_string (file_name_style.style (),
			    saved_filename.c_str ()));
}

static std::string logging_filename = "gdb.txt";

static void
set_logging_filename (const char *args,
		      int from_tty, struct cmd_list_element *c)
{
  maybe_warn_already_logging ();
}

static void
show_logging_filename (struct ui_file *file, int from_tty,
		       struct cmd_list_element *c, const char *value)
{
  gdb_printf (file, _("The current logfile is \"%ps\".\n"),
	      styled_string (file_name_style.style (), value));
}

static bool logging_overwrite;

static void
set_logging_overwrite (const char *args,
		       int from_tty, struct cmd_list_element *c)
{
  maybe_warn_already_logging ();
}

static void
show_logging_overwrite (struct ui_file *file, int from_tty,
			struct cmd_list_element *c, const char *value)
{
  if (logging_overwrite)
    gdb_printf (file, _("on: Logging overwrites the log file.\n"));
  else
    gdb_printf (file, _("off: Logging appends to the log file.\n"));
}

/* The current log file, or nullptr if none.  */
static ui_file_up log_file;

#ifdef TUI
static ui_file_up tui_history_file;
#endif

/* Value as configured by the user.  */
static bool logging_redirect;
static bool debug_redirect;

static void
set_logging_redirect (const char *args,
		      int from_tty, struct cmd_list_element *c)
{
  maybe_warn_already_logging ();
}

static void
show_logging_redirect (struct ui_file *file, int from_tty,
		       struct cmd_list_element *c, const char *value)
{
  if (logging_redirect)
    gdb_printf (file, _("on: Output will go only to the log file.\n"));
  else
    gdb_printf
      (file,
       _("off: Output will go to both the screen and the log file.\n"));
}

static void
show_logging_debug_redirect (struct ui_file *file, int from_tty,
		       struct cmd_list_element *c, const char *value)
{
  if (debug_redirect)
    gdb_printf (file, _("on: Debug output will go only to the log file.\n"));
  else
    gdb_printf
      (file,
       _("off: Debug output will go to both the screen and the log file.\n"));
}

/* Values as used by the logging_file implementation.  These are
   separate and only set when logging is enabled, because historically
   gdb required you to disable and re-enable logging to change these
   settings.  */

static bool logging_redirect_for_file;
static bool debug_redirect_for_file;

/* See logging-file.h.  */

template<typename T>
bool
logging_file<T>::ordinary_output () const
{
  if (log_file == nullptr)
    return true;
  if (logging_redirect_for_file)
    return false;
  if (debug_redirect_for_file)
    return !m_for_stdlog;
  return true;
}

/* See logging-file.h.  */

template<typename T>
void
logging_file<T>::flush ()
{
  if (log_file != nullptr)
    log_file->flush ();
  /* Always flushing seems fine.  */
  m_out->flush ();
}

/* See logging-file.h.  */

template<typename T>
bool
logging_file<T>::can_page () const
{
  /* If all output is redirected, do not page.  */
  if (!ordinary_output ())
    return false;
  /* In other cases, paging happens if the underlying stream can
     page.  */
  return m_out->can_page ();
}

/* See logging-file.h.  */

template<typename T>
void
logging_file<T>::write (const char *buf, long length_buf)
{
  if (log_file != nullptr)
    log_file->write (buf, length_buf);
  if (ordinary_output ())
    {
      m_out->write (buf, length_buf);
#ifdef TUI
      tui_history_file->write (buf, length_buf);
#endif
    }
}

/* See logging-file.h.  */

template<typename T>
void
logging_file<T>::write_async_safe (const char *buf, long length_buf)
{
  if (log_file != nullptr)
    log_file->write_async_safe (buf, length_buf);
  if (ordinary_output ())
    m_out->write_async_safe (buf, length_buf);
}

/* See logging-file.h.  */

template<typename T>
void
logging_file<T>::puts (const char *linebuffer)
{
  if (log_file != nullptr)
    log_file->puts (linebuffer);
  if (ordinary_output ())
    {
      m_out->puts (linebuffer);
#ifdef TUI
      tui_history_file->puts (linebuffer);
#endif
    }
}

/* See logging-file.h.  */

template<typename T>
void
logging_file<T>::emit_style_escape (const ui_file_style &style)
{
  if (log_file != nullptr)
    log_file->emit_style_escape (style);
  if (ordinary_output ())
    {
      m_out->emit_style_escape (style);
#ifdef TUI
      tui_history_file->emit_style_escape (style);
#endif
    }
}

/* See logging-file.h.  */

template<typename T>
void
logging_file<T>::puts_unfiltered (const char *str)
{
  if (log_file != nullptr)
    log_file->puts_unfiltered (str);
  if (ordinary_output ())
    {
      m_out->puts_unfiltered (str);
#ifdef TUI
      tui_history_file->puts_unfiltered (str);
#endif
    }
}

/* The available instantiations of logging_file.  */
template class logging_file<ui_file *>;
template class logging_file<ui_file_up>;

/* This is a helper for the `set logging' command.  */
static void
handle_redirections (int from_tty)
{
  if (!saved_filename.empty ())
    {
      gdb_printf ("Already logging to %ps.\n",
		  styled_string (file_name_style.style (),
				 saved_filename.c_str ()));
      return;
    }

  stdio_file_up log = std::make_unique<no_terminal_escape_file<stdio_file>> ();
  if (!log->open (logging_filename.c_str (), logging_overwrite ? "w" : "a"))
    perror_with_name (_("set logging"));

  /* Redirects everything to gdb_stdout while this is running.  */
  if (from_tty)
    {
      if (!logging_redirect)
	gdb_printf ("Copying output to %ps.\n",
		    styled_string (file_name_style.style (),
				   logging_filename.c_str ()));
      else
	gdb_printf ("Redirecting output to %ps.\n",
		    styled_string (file_name_style.style (),
				   logging_filename.c_str ()));

      if (!debug_redirect)
	gdb_printf ("Copying debug output to %ps.\n",
		    styled_string (file_name_style.style (),
				   logging_filename.c_str ()));
      else
	gdb_printf ("Redirecting debug output to %ps.\n",
		    styled_string (file_name_style.style (),
				   logging_filename.c_str ()));
    }

  saved_filename = logging_filename;
  logging_redirect_for_file = logging_redirect;
  debug_redirect_for_file = debug_redirect;
  log_file = std::move (log);
}

static void
set_logging_on (const char *args, int from_tty)
{
  const char *rest = args;

  if (rest && *rest)
    logging_filename = rest;

  handle_redirections (from_tty);
}

static void
set_logging_off (const char *args, int from_tty)
{
  if (saved_filename.empty ())
    return;

  log_file.reset ();
  if (from_tty)
    gdb_printf ("Done logging to %ps.\n",
		styled_string (file_name_style.style (),
			       saved_filename.c_str ()));

  saved_filename.clear ();
}

static bool logging_enabled;

static void
set_logging_enabled (const char *args,
		     int from_tty, struct cmd_list_element *c)
{
  if (logging_enabled)
    set_logging_on (args, from_tty);
  else
    set_logging_off (args, from_tty);
}

static void
show_logging_enabled (struct ui_file *file, int from_tty,
		       struct cmd_list_element *c, const char *value)
{
  if (logging_enabled)
    gdb_printf (file, _("on: Logging is enabled.\n"));
  else
    gdb_printf (file, _("off: Logging is disabled.\n"));
}

INIT_GDB_FILE (cli_logging)
{
  static struct cmd_list_element *set_logging_cmdlist, *show_logging_cmdlist;

  /* Set/show logging.  */
  add_setshow_prefix_cmd ("logging", class_support,
			  _("Set logging options."),
			  _("Show logging options."),
			  &set_logging_cmdlist, &show_logging_cmdlist,
			  &setlist, &showlist);

  /* Set/show logging overwrite.  */
  add_setshow_boolean_cmd ("overwrite", class_support, &logging_overwrite, _("\
Set whether logging overwrites or appends to the log file."), _("\
Show whether logging overwrites or appends to the log file."), _("\
If set, logging overwrites the log file."),
			   set_logging_overwrite,
			   show_logging_overwrite,
			   &set_logging_cmdlist, &show_logging_cmdlist);

  /* Set/show logging redirect.  */
  add_setshow_boolean_cmd ("redirect", class_support, &logging_redirect, _("\
Set the logging output mode."), _("\
Show the logging output mode."), _("\
If redirect is off, output will go to both the screen and the log file.\n\
If redirect is on, output will go only to the log file."),
			   set_logging_redirect,
			   show_logging_redirect,
			   &set_logging_cmdlist, &show_logging_cmdlist);

  /* Set/show logging debugredirect.  */
  add_setshow_boolean_cmd ("debugredirect", class_support,
			   &debug_redirect, _("\
Set the logging debug output mode."), _("\
Show the logging debug output mode."), _("\
If debug redirect is off, debug will go to both the screen and the log file.\n\
If debug redirect is on, debug will go only to the log file."),
			   set_logging_redirect,
			   show_logging_debug_redirect,
			   &set_logging_cmdlist, &show_logging_cmdlist);

  /* Set/show logging file.  */
  add_setshow_filename_cmd ("file", class_support, &logging_filename, _("\
Set the current logfile."), _("\
Show the current logfile."), _("\
The logfile is used when directing GDB's output."),
			    set_logging_filename,
			    show_logging_filename,
			    &set_logging_cmdlist, &show_logging_cmdlist);

  /* Set/show logging enabled.  */
  set_show_commands setshow_logging_enabled_cmds
    = add_setshow_boolean_cmd ("enabled", class_support, &logging_enabled,
			       _("Enable logging."),
			       _("Show whether logging is enabled."),
			       _("When on, enable logging."),
			       set_logging_enabled,
			       show_logging_enabled,
			       &set_logging_cmdlist, &show_logging_cmdlist);

  /* Set logging on, deprecated alias.  */
  cmd_list_element *set_logging_on_cmd
    = add_alias_cmd ("on", setshow_logging_enabled_cmds.set, class_support,
		     false, &set_logging_cmdlist);
  deprecate_cmd (set_logging_on_cmd, "set logging enabled on");
  set_logging_on_cmd->default_args = "on";

  /* Set logging off, deprecated alias.  */
  cmd_list_element *set_logging_off_cmd
    = add_alias_cmd ("off", setshow_logging_enabled_cmds.set, class_support,
		     false, &set_logging_cmdlist);
  deprecate_cmd (set_logging_off_cmd, "set logging enabled off");
  set_logging_off_cmd->default_args = "off";

#ifdef TUI
  tui_history_file = std::make_unique<cmd_history_ui_file> ();
#endif
}

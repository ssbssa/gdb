/* General functions for the GDB TUI.

   Copyright (C) 1998-2026 Free Software Foundation, Inc.

   Contributed by Hewlett-Packard Company.  Developed as part of HP Wildebeest
   (WDB), a port of GDB to HP-UX.

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

#include "event-top.h"
#include "cli/cli-cmds.h"
#include "exceptions.h"
#include "tui/tui.h"
#include "tui/tui-hooks.h"
#include "tui/tui-command.h"
#include "tui/tui-data.h"
#include "tui/tui-layout.h"
#include "tui/tui-io.h"
#include "tui/tui-status.h"
#include "tui/tui-win.h"
#include "tui/tui-wingeneral.h"
#include "tui/tui-winsource.h"
#include "tui/tui-source.h"
#include "target.h"
#include "frame.h"
#include "inferior.h"
#include "symtab.h"
#include "terminal.h"
#include "top.h"
#include "ui.h"
#include "observable.h"
#include "run-on-main-thread.h"

#include <fcntl.h>

#include "gdb_curses.h"
#include "interps.h"

/* See tui.h.  */

bool debug_tui = false;

/* Implement 'show debug tui'.  */

static void
show_tui_debug (struct ui_file *file, int from_tty,
		struct cmd_list_element *c, const char *value)
{
  gdb_printf (file, _("TUI debugging is \"%s\".\n"), value);
}

/* This redefines CTRL if it is not already defined, so it must come
   after terminal state related include files like <term.h> and
   "gdb_curses.h".  */
#include "readline/readline.h"

/* Tells whether the TUI is active or not.  */
bool tui_active = false;

/* See tui.h.  */
bool tui_defer_rerender = false;

/* Tells whether the TUI should do deferred curses initialization.
   If TRIBOOL_TRUE, then yes.  If TRIBOOL_FALSE. then no (because
   initialization is already done).  If TRIBOOL_UNKNOWN, then no (because
   initialization failed).  */
static tribool tui_finish_init = TRIBOOL_TRUE;

enum tui_key_mode tui_current_key_mode = TUI_COMMAND_MODE;

struct tui_char_command
{
  unsigned char key;
  const char *cmd;
};

/* Key mapping to gdb commands when the TUI is using the single key
   mode.  */
static const struct tui_char_command tui_commands[] = {
  { 'c', "continue" },
  { 'C', "reverse-continue" },
  { 'd', "down" },
  { 'f', "finish" },
  { 'F', "reverse-finish" },
  { 'n', "next" },
  { 'N', "reverse-next" },
  { 'o', "nexti" },
  { 'O', "reverse-nexti" },
  { 'r', "run" },
  { 's', "step" },
  { 'S', "reverse-step" },
  { 'i', "stepi" },
  { 'I', "reverse-stepi" },
  { 'u', "up" },
  { 'v', "info locals" },
  { 'w', "where" },
  { 0, 0 },
};

static Keymap tui_keymap;
static Keymap tui_readline_standard_keymap;

/* TUI readline command.
   Switch the output mode between TUI/standard gdb.  */
static int
tui_rl_switch_mode (int notused1 = 0, int notused2 = 0)
{
  gdb_assert (!gdb_in_secondary_prompt_p (current_ui));

  /* This function is called through the run_on_main_thread event loop
     callback mechanism.  That mechanism propagates
     gdb_exception_forced_quit exceptions, but silently discards
     gdb_exception exceptions.  We catch and print gdb_exception
     exceptions before propagating them, the run_on_main_thread
     mechanism will then discard these and return to the event loop.
     Any RAII cleanup between here and there will have been done,
     which is important.  For gdb_exception_forced_quit exceptions we
     just propagate these up the stack without printing, these will be
     handled when they are caught by the event loop.  */
  try
    {
      if (tui_active)
	tui_disable ();
      else
	{
	  /* If we type "foo", entering it into the readline buffer

	       (gdb) foo
			^
	     and then switch to TUI and back, we may get back

	       (gdb) foo
		     ^
	     which is confusing because "foo" is no longer part of the
	     readline buffer.  Fix this by clearing it before switching to
	     TUI.  */
	  rl_clear_visible_line ();

	  /* Disable readline state ahead of enabling TUI mode.  If
	     tui_enable fails then the next display_gdb_prompt will
	     re-prep the terminal for us.  */
	  rl_deprep_terminal ();

	  tui_enable ();
	}
    }
  catch (const gdb_exception_forced_quit &ex)
    {
      throw;
    }
  catch (const gdb_exception &ex)
    {
      exception_print (gdb_stderr, ex);
      throw;
    }

  return 0;
}

/* Try to switch into TUI mode, if not already active.  Return if TUI mode
   is active.  */

static bool
tui_try_activate ()
{
  if (!tui_active)
    tui_rl_switch_mode ();

  return tui_active;
}

/* TUI readline command.
   Change the TUI layout to show a next layout.
   This function is bound to CTRL-X 2.  It is intended to provide
   a functionality close to the Emacs split-window command.  */
static int
tui_rl_change_windows (int notused1, int notused2)
{
  gdb_assert (!gdb_in_secondary_prompt_p (current_ui));

  if (tui_try_activate ())
    tui_next_layout ();

  return 0;
}

/* TUI readline command.
   Delete the second TUI window to only show one.  */
static int
tui_rl_delete_other_windows (int notused1, int notused2)
{
  gdb_assert (!gdb_in_secondary_prompt_p (current_ui));

  if (tui_try_activate ())
    tui_remove_some_windows ();

  return 0;
}

/* TUI readline command.
   Switch the active window to give the focus to a next window.  */
static int
tui_rl_other_window (int count, int key)
{
  gdb_assert (!gdb_in_secondary_prompt_p (current_ui));

  if (tui_try_activate ())
    tui_set_win_focus_to (tui_next_win (tui_win_with_focus ()));

  return 0;
}

/* TUI readline command.
   Execute the gdb command bound to the specified key.  */
static int
tui_rl_command_key (int count, int key)
{
  gdb_assert (!gdb_in_secondary_prompt_p (current_ui));

  reinitialize_more_filter ();
  for (int i = 0; tui_commands[i].cmd; i++)
    {
      if (tui_commands[i].key == key)
	{
	  /* Insert the command in the readline buffer.
	     Avoid calling the gdb command here since it creates
	     a possible recursion on readline if prompt_for_continue
	     is called (See PR 9584).  The command will also appear
	     in the readline history which turns out to be better.  */
	  rl_insert_text (tui_commands[i].cmd);
	  rl_newline (1, '\n');

	  /* Switch to gdb command mode while executing the command.
	     This way the gdb's continue prompt will be displayed.  */
	  tui_set_key_mode (TUI_ONE_COMMAND_MODE);
	  return 0;
	}
    }
  return 0;
}

/* TUI readline command.
   Temporarily leave the TUI SingleKey mode to allow editing
   a gdb command with the normal readline.  Once the command
   is executed, the TUI SingleKey mode is installed back.  */
static int
tui_rl_command_mode (int count, int key)
{
  gdb_assert (!gdb_in_secondary_prompt_p (current_ui));

  tui_set_key_mode (TUI_ONE_COMMAND_MODE);
  return rl_insert (count, key);
}

/* TUI readline command.
   Switch between TUI SingleKey mode and gdb readline editing.  */
static int
tui_rl_next_keymap (int notused1, int notused2)
{
  gdb_assert (!gdb_in_secondary_prompt_p (current_ui));

  if (!tui_try_activate ())
    return 0;

  if (rl_end)
    {
      rl_end = 0;
      rl_point = 0;
      rl_mark = 0;
    }

  tui_set_key_mode (tui_current_key_mode == TUI_COMMAND_MODE
		    ? TUI_SINGLE_KEY_MODE : TUI_COMMAND_MODE);
  return 0;
}

/* Readline hook to redisplay ourself the gdb prompt.
   In the SingleKey mode, the prompt is not printed so that
   the command window is cleaner.  It will be displayed if
   we temporarily leave the SingleKey mode.  */
static int
tui_rl_startup_hook (void)
{
  if (tui_current_key_mode != TUI_COMMAND_MODE
      && !gdb_in_secondary_prompt_p (current_ui))
    tui_set_key_mode (TUI_SINGLE_KEY_MODE);
  return 0;
}

/* Change the TUI key mode by installing the appropriate readline
   keymap.  */
void
tui_set_key_mode (enum tui_key_mode mode)
{
  tui_current_key_mode = mode;
  rl_set_keymap (mode == TUI_SINGLE_KEY_MODE
		 ? tui_keymap : tui_readline_standard_keymap);
  tui_show_status_content ();
}

/* Wrapper around function FPTR, used to add common checks before
   functions that are bound to readline multi-key combinations.  */

template<int (*FPTR) (int, int)>
int
tui_rl_keybinding (int count, int key)
{
  /* Don't allow TUI changes while we're at an interactive prompt.  */
  if (gdb_in_secondary_prompt_p (current_ui))
    return 0;

  run_on_main_thread ([=] () {
    bool was_active = tui_active;

    /* Cleanup required even on the exception path.  */
    SCOPE_EXIT {
      /* If we switched from CLI to TUI (or back) then we should
	 reinitialize the pager in order to avoid spurious pagination
	 prompts.  This is especially important going from CLI to TUI
	 where the command window is usually smaller than the full
	 terminal, so the pager might already think that we have more
	 lines printed than will fit in the window, despite the window
	 starting empty after a mode switch.  */
      if (was_active != tui_active)
	reinitialize_more_filter ();

      /* The user was at a prompt and pressed a multi-key combination
	 (e.g. C-x C-a).  As a result this callback was invoked from
	 the event loop.  We're now exiting this callback and want to
	 ensure that the prompt is drawn correctly.

	 We have two approaches, full display_gdb_prompt, or a light
	 weight tui_redisplay_readline.  The former will clear the
	 readline input buffer and redisplay the prompt, while the
	 second will redraw the prompt along with anything in the
	 input buffer.  We want the light weight option where
	 possible, but there are times when this isn't an option:

	 1. A successful switch between CLI and TUI, in either
	    direction, changes how we update the prompt.  We need to
	    call display_gdb_prompt the first time to ensure
	    everything is done correctly.  We also need to consider
	    the case where tui_enable fails, leaving us in CLI mode,
	    this also requires a call to display_gdb_prompt as the
	    light weight tui_redisplay_readline is not appropriate for
	    CLI use.

	 2. Usually the TUI will have the RL_STATE_CALLBACK state flag
	    set because of when this event callback is called.  If a
	    secondary prompt has been displayed, then once the
	    secondary prompt completed, the RL_STATE_CALLBACK flag
	    will have been cleared.  This is good for us, because
	    after a secondary prompt rl_prompt will still hold the
	    secondary prompt string, so we need display_gdb_prompt to
	    set the correct top-level prompt.  */
      if (!was_active || !tui_active || !RL_ISSTATE (RL_STATE_CALLBACK))
	{
	  current_ui->prompt_state = PROMPT_NEEDED;
	  display_gdb_prompt (nullptr);
	}
      else
	{
	  /* We can only reach here when was_active and tui_active are
	     both true: starting in CLI mode (!was_active) or ending
	     in CLI mode (!tui_active) both take the if block
	     above.  */
	  gdb_assert (tui_active);

	  /* The only time rl_prompt will be NULL is when we switch
	     from CLI to TUI, but that will be handled by the block
	     above.  */
	  gdb_assert (rl_prompt != nullptr);

	  tui_redisplay_readline ();
	}
    };

    (void) FPTR (count, key);
  });

  return 0;
}

/* Initialize readline and configure the keymap for the switching
   key shortcut.  */
void
tui_ensure_readline_initialized ()
{
  static bool initialized;

  if (initialized)
    return;
  initialized = true;

  int i;
  Keymap tui_ctlx_keymap;

  rl_add_defun ("tui-switch-mode",
		tui_rl_keybinding<tui_rl_switch_mode>, -1);
  rl_add_defun ("next-keymap",
		tui_rl_keybinding<tui_rl_next_keymap>, -1);
  rl_add_defun ("tui-delete-other-windows",
		tui_rl_keybinding<tui_rl_delete_other_windows>, -1);
  rl_add_defun ("tui-change-windows",
		tui_rl_keybinding<tui_rl_change_windows>, -1);
  rl_add_defun ("tui-other-window",
		tui_rl_keybinding<tui_rl_other_window>, -1);

  tui_keymap = rl_make_bare_keymap ();

  /* The named keymap feature was added in Readline 8.0.  */
#if RL_READLINE_VERSION >= 0x800
  rl_set_keymap_name ("SingleKey", tui_keymap);
#endif

  tui_ctlx_keymap = rl_make_bare_keymap ();
  tui_readline_standard_keymap = rl_get_keymap ();

  for (i = 0; tui_commands[i].cmd; i++)
    rl_bind_key_in_map (tui_commands[i].key, tui_rl_command_key, tui_keymap);

  rl_generic_bind (ISKMAP, "\\C-x", (char*) tui_ctlx_keymap, tui_keymap);

  /* Bind all other keys to tui_rl_command_mode so that we switch
     temporarily from SingleKey mode and can enter a gdb command.  */
  for (i = ' '; i < 0x7f; i++)
    {
      int j;

      for (j = 0; tui_commands[j].cmd; j++)
	if (tui_commands[j].key == i)
	  break;

      if (tui_commands[j].cmd)
	continue;

      rl_bind_key_in_map (i, tui_rl_command_mode, tui_keymap);
    }

  rl_bind_key_in_map ('a', tui_rl_keybinding<tui_rl_switch_mode>, emacs_ctlx_keymap);
  rl_bind_key_in_map ('a', tui_rl_keybinding<tui_rl_switch_mode>, tui_ctlx_keymap);
  rl_bind_key_in_map ('A', tui_rl_keybinding<tui_rl_switch_mode>, emacs_ctlx_keymap);
  rl_bind_key_in_map ('A', tui_rl_keybinding<tui_rl_switch_mode>, tui_ctlx_keymap);
  rl_bind_key_in_map (c_ctrl ('A'), tui_rl_keybinding<tui_rl_switch_mode>, emacs_ctlx_keymap);
  rl_bind_key_in_map (c_ctrl ('A'), tui_rl_keybinding<tui_rl_switch_mode>, tui_ctlx_keymap);
  rl_bind_key_in_map ('1', tui_rl_keybinding<tui_rl_delete_other_windows>, emacs_ctlx_keymap);
  rl_bind_key_in_map ('1', tui_rl_keybinding<tui_rl_delete_other_windows>, tui_ctlx_keymap);
  rl_bind_key_in_map ('2', tui_rl_keybinding<tui_rl_change_windows>, emacs_ctlx_keymap);
  rl_bind_key_in_map ('2', tui_rl_keybinding<tui_rl_change_windows>, tui_ctlx_keymap);
  rl_bind_key_in_map ('o', tui_rl_keybinding<tui_rl_other_window>, emacs_ctlx_keymap);
  rl_bind_key_in_map ('o', tui_rl_keybinding<tui_rl_other_window>, tui_ctlx_keymap);
  rl_bind_key_in_map ('q', tui_rl_keybinding<tui_rl_next_keymap>, tui_keymap);
  rl_bind_key_in_map ('s', tui_rl_keybinding<tui_rl_next_keymap>, emacs_ctlx_keymap);
  rl_bind_key_in_map ('s', tui_rl_keybinding<tui_rl_next_keymap>, tui_ctlx_keymap);

  /* Initialize readline after the above.  */
  rl_initialize ();
}

/* Return the TERM variable from the environment, or "<unset>"
   if not set.  */

static const char *
gdb_getenv_term (void)
{
  const char *term;

  term = getenv ("TERM");
  if (term != NULL)
    return term;
  return "<unset>";
}

/* Error out if the toplevel interpreter is not the TUI interpreter.  */

static void
require_tui_interpreter ()
{
  const char *interp = top_level_interpreter ()->name ();
  if (!streq (interp, INTERP_TUI))
    error (_("Cannot enable or disable the TUI when the interpreter is '%s'"),
	   interp);
}

/* Error out if the terminal doesn't support TUI.  */

static void
require_tui_terminal ()
{
  /* Don't try to setup curses (and print funny control
     characters) if we're not outputting to a terminal.  */
  if (!gdb_stderr->isatty ())
    error (_("Cannot enable the TUI when output is not a terminal"));

  /* Check required terminal capabilities.  The MinGW port of
     ncurses does have them, but doesn't expose them through "cup".  */
#ifndef __MINGW32__
  const char *cap = tigetstr ((char *) "cup");
  const char *not_a_string_capability = (char *) -1;
  if (cap == nullptr || cap == not_a_string_capability || *cap == '\0')
    error (_("Cannot enable the TUI: "
	     "terminal doesn't support cursor addressing [TERM=%s]"),
	   gdb_getenv_term ());
#endif
}

/* Initialize ncurses, if necessary.  */

static SCREEN *
init_ncurses ()
{
  static SCREEN *tui_screen = nullptr;
  if (tui_screen != nullptr)
    {
      /* Init ncurses only once.  */
      return tui_screen;
    }

  tui_screen = newterm (nullptr, stdout, stdin);

#ifdef __MINGW32__
  /* The MinGW port of ncurses requires $TERM to be unset in order
     to activate the Windows console driver.  */
  if (tui_screen == nullptr)
    tui_screen = newterm ((char *) "unknown", stdout, stdin);
#endif

  if (tui_screen == nullptr)
    error (_("Cannot enable the TUI: error opening terminal [TERM=%s]"),
	   gdb_getenv_term ());

  return tui_screen;
}

/* Enter in the tui mode (curses).
   When in normal mode, it installs the tui hooks in gdb, redirects
   the gdb output, configures the readline to work in tui mode.
   When in curses mode, it does nothing.  */
void
tui_enable (void)
{
  TUI_SCOPED_DEBUG_ENTER_EXIT;

  if (tui_active)
    return;

  tui_batch_rendering defer;

  /* Defer filling in window contents at this time.  The window
     content will be filled in by calling rerender later.  We do this
     so that we can be sure the CMD window will exist as other windows
     are rendered, filling in some windows might trigger a secondary
     prompt (e.g. debuginfod prompt) and we want to be sure that the
     CMD window exists to display the prompt in.  */
  tui_defer_rerender = true;

  /* To avoid to initialize curses when gdb starts, there is a deferred
     curses initialization.  This initialization is made only once
     and the first time the curses mode is entered.  */
  if (tui_finish_init == TRIBOOL_UNKNOWN)
    {
      /* Initialization failed before, just throw a generic error, don't try
	 again.  */
      error (_("Cannot enable the TUI"));
    }

  if (tui_finish_init == TRIBOOL_TRUE)
    {
      WINDOW *w;

      /* If the top level interpreter is not the console/tui (e.g.,
	 MI), enabling curses will certainly lose.  */
      require_tui_interpreter ();

      /* Require a terminal that supports TUI.  */
      require_tui_terminal ();

      /* Don't try initialization again.  */
      tui_finish_init = TRIBOOL_UNKNOWN;

      init_ncurses ();
      w = stdscr;

      if (has_colors ())
	{
#ifdef HAVE_USE_DEFAULT_COLORS
	  /* Ncurses extension to help with resetting to the default
	     color.  */
	  use_default_colors ();
#endif
	  start_color ();
	}

      /* We must mark the tui sub-system active before trying to setup the
	 current layout as tui windows defined by an extension language
	 rely on this flag being true in order to know that the window
	 they are creating is currently valid.  */
      tui_active = true;
      try
	{
	  cbreak ();
	  noecho ();
	  /* timeout (1); */
	  nodelay (w, FALSE);
	  nl ();
	  keypad (w, TRUE);
	  tui_set_term_height_to (LINES);
	  tui_set_term_width_to (COLS);
	  def_prog_mode ();
	  tui_show_frame_info (deprecated_safe_get_selected_frame ());
	  tui_set_initial_layout ();
	  tui_set_win_focus_to (tui_src_win ());
	  keypad (tui_cmd_win ()->handle.get (), TRUE);
	  wrefresh (tui_cmd_win ()->handle.get ());
	  tui_set_win_resized_to (false);
	}
      catch (const gdb_exception &)
	{
	  endwin ();

	  /* Initialization failed, so TUI is not active.  */
	  tui_active = false;

	  /* Allow trying to initialize TUI again.  */
	  tui_finish_init = TRIBOOL_TRUE;

	  throw;
	}

      tui_finish_init = TRIBOOL_FALSE;
    }
  else
    {
      /* Save the current gdb setting of the terminal.
	 Curses will restore this state when endwin() is called.  */
      def_shell_mode ();
      clearok (stdscr, TRUE);

      tui_active = true;
    }

  gdb_assert (tui_active);

  if (tui_update_variables ())
    tui_rehighlight_all ();

  tui_setup_io (1);

  /* Resize windows before anything might display/refresh a
     window.  */
  if (tui_win_resized ())
    {
      tui_set_win_resized_to (false);
      tui_resize_all ();
    }

  /* Install the TUI specific hooks.  This must be done after the call to
     tui_display_main so that we don't detect the symtab changed event it
     can cause.  */
  tui_install_hooks ();
  rl_startup_hook = tui_rl_startup_hook;

  /* Restore TUI keymap.  */
  tui_set_key_mode (tui_current_key_mode);

  /* Refresh the screen.  */
  tui_refresh_all_win ();

  /* Update gdb's knowledge of its terminal.  */
  gdb_save_tty_state ();
  tui_update_gdb_sizes ();

  gdb::observers::tui_enabled.notify (true);
}

/* Leave the tui mode.
   Remove the tui hooks and configure the gdb output and readline
   back to their original state.  The curses mode is left so that
   the terminal setting is restored to the point when we entered.  */
void
tui_disable (void)
{
  TUI_SCOPED_DEBUG_ENTER_EXIT;

  if (!tui_active)
    return;

  require_tui_interpreter ();

  /* Restore initial readline keymap.  */
  rl_set_keymap (tui_readline_standard_keymap);

  /* Remove TUI hooks.  */
  tui_remove_hooks ();
  rl_startup_hook = 0;
  rl_already_prompted = 0;

#ifdef NCURSES_MOUSE_VERSION
  mousemask (0, NULL);
#endif

  /* Leave curses and restore previous gdb terminal setting.  */
  endwin ();

  /* gdb terminal has changed, update gdb internal copy of it
     so that terminal management with the inferior works.  */
  tui_setup_io (0);

  /* Update gdb's knowledge of its terminal.  */
  gdb_save_tty_state ();

  tui_active = false;
  tui_update_gdb_sizes ();

#ifdef __MINGW32__
    {
      int width, height;
      rl_reset_screen_size ();
      rl_get_screen_size (&height, &width);
      width += readline_hidden_cols;
      set_screen_width_and_height (width, height);
      tui_set_win_resized_to (true);
    }
#endif

  gdb::observers::tui_enabled.notify (false);
}

/* Command wrapper for enabling tui mode.  */

static void
tui_enable_command (const char *args, int from_tty)
{
  tui_enable ();
}

/* Command wrapper for leaving tui mode.  */

static void
tui_disable_command (const char *args, int from_tty)
{
  tui_disable ();
}

void
tui_show_assembly (struct gdbarch *gdbarch, CORE_ADDR addr)
{
  tui_batch_rendering suppress;
  tui_add_win_to_layout (DISASSEM_WIN);
  tui_update_source_windows_with_addr (gdbarch, addr);
}

bool
tui_is_window_visible (enum tui_win_type type)
{
  if (!tui_active)
    return false;

  if (tui_win_list[type] == nullptr)
    return false;

  return tui_win_list[type]->is_visible ();
}

bool
tui_get_command_dimension (unsigned int *width,
			   unsigned int *height)
{
  if (!tui_active || (tui_cmd_win () == NULL))
    return false;

  *width = tui_cmd_win ()->width;
  *height = tui_cmd_win ()->height;
  return true;
}

INIT_GDB_FILE (tui)
{
  struct cmd_list_element **tuicmd;

  tuicmd = tui_get_cmd_list ();

  add_cmd ("enable", class_tui, tui_enable_command,
	   _("Enable TUI display mode.\n\
Usage: tui enable"),
	   tuicmd);
  add_cmd ("disable", class_tui, tui_disable_command,
	   _("Disable TUI display mode.\n\
Usage: tui disable"),
	   tuicmd);

  /* Debug this tui internals.  */
  add_setshow_boolean_cmd ("tui", class_maintenance, &debug_tui,  _("\
Set tui debugging."), _("\
Show tui debugging."), _("\
When true, tui specific internal debugging is enabled."),
			   NULL,
			   show_tui_debug,
			   &setdebuglist, &showdebuglist);
}

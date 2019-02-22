/* Specific command window processing.

   Copyright (C) 1998-2019 Free Software Foundation, Inc.

   Contributed by Hewlett-Packard Company.

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

#include "defs.h"
#include "tui/tui.h"
#include "tui/tui-data.h"
#include "tui/tui-win.h"
#include "tui/tui-io.h"
#include "tui/tui-command.h"

#include "gdb_curses.h"

#include "breakpoint.h"
#include "linespec.h"
#include "source.h"
#include "filenames.h"

/*****************************************
** STATIC LOCAL FUNCTIONS FORWARD DECLS    **
******************************************/



/*****************************************
** PUBLIC FUNCTIONS                        **
******************************************/

static int tui_resizer = -1;

/* Dispatch the correct tui function based upon the control
   character.  */
unsigned int
tui_dispatch_ctrl_char (unsigned int ch)
{
  struct tui_win_info *win_info = tui_win_with_focus ();

  /* Handle the CTRL-L refresh for each window.  */
  if (ch == '\f')
    tui_refresh_all_win ();

  /* If the command window has the logical focus, or no-one does
     assume it is the command window; in this case, pass the character
     on through and do nothing here.  */
  if (win_info == NULL || win_info == TUI_CMD_WIN)
    return ch;

  switch (ch)
    {
    case KEY_NPAGE:
      tui_scroll_forward (win_info, 0);
      break;
    case KEY_PPAGE:
      tui_scroll_backward (win_info, 0);
      break;
    case KEY_DOWN:
    case KEY_SF:
      tui_scroll_forward (win_info, 1);
      break;
    case KEY_UP:
    case KEY_SR:
      tui_scroll_backward (win_info, 1);
      break;
    case KEY_RIGHT:
      tui_scroll_left (win_info, 1);
      break;
    case KEY_LEFT:
      tui_scroll_right (win_info, 1);
      break;
    case KEY_MOUSE:
      request_mouse_pos ();
      if (MOUSE_WHEEL_UP)
	tui_scroll_backward (win_info, 3);
      else if (MOUSE_WHEEL_DOWN)
	tui_scroll_forward (win_info, 3);
      else if ((BUTTON_CHANGED(1) && BUTTON_STATUS(1) == BUTTON_CLICKED)
	       || (BUTTON_CHANGED(2) && BUTTON_STATUS(2) == BUTTON_CLICKED))
	{
	  if (TUI_SRC_WIN && TUI_SRC_WIN->generic.handle
	      && TUI_SRC_WIN->generic.content)
	    {
	      struct tui_gen_win_info *gwi = &TUI_SRC_WIN->generic;

	      if (MOUSE_X_POS < gwi->origin.x
		  && MOUSE_Y_POS > gwi->origin.y
		  && MOUSE_Y_POS < gwi->origin.y + gwi->height - 1)
		{
		  struct tui_source_info *src =
		    &TUI_SRC_WIN->detail.source_info;
		  char *fn = (char *) xmalloc (strlen (src->fullname) + 20);
		  sprintf (fn, "%s:%d", src->fullname,
			   src->start_line_or_addr.u.line_no
			   + MOUSE_Y_POS - 1 - gwi->origin.y);
		  const char *fullname = fn;

		  struct linespec_sals lsal;
		  lsal.canonical = NULL;

		  TRY
		    {
		      lsal.sals
			= decode_line_with_current_source (fullname, 0);
		    }
		  CATCH (except, RETURN_MASK_ERROR)
		    {
		    }
		  END_CATCH

		  if (lsal.sals.size () == 1)
		    {
		      symtab_and_line &sal = lsal.sals[0];
		      struct breakpoint *bp;
		      extern struct breakpoint *breakpoint_chain;

		      for (bp = breakpoint_chain; bp != NULL; bp = bp->next)
			{
			  struct bp_location *loc;

			  for (loc = bp->loc; loc != NULL; loc = loc->next)
			    {
			      if (loc->symtab != NULL
				  && loc->line_number == sal.line
				  && filename_cmp (src->fullname,
						   symtab_to_fullname (loc->symtab)) == 0)
				break;
			    }
			  if (loc)
			    break;
			}

		      if (BUTTON_CHANGED(2) && BUTTON_STATUS(2) == BUTTON_CLICKED)
			{
			  if (bp)
			    delete_breakpoint (bp);
			}
		      else if (bp)
			{
			  if (bp->enable_state == bp_disabled)
			    enable_breakpoint (bp);
			  else
			    disable_breakpoint (bp);
			}
		      else
			{
			  struct linespec_result canonical;
			  canonical.lsals.push_back (std::move (lsal));

			  bkpt_breakpoint_ops.
			    create_breakpoints_sal (src->gdbarch,
						    &canonical,
						    NULL,
						    NULL,
						    bp_breakpoint,
						    disp_donttouch,
						    -1, 0, 0,
						    &bkpt_breakpoint_ops,
						    0, 1, 0, 0);
			}
		    }

		  xfree (fn);
		}
	    }
	}
      else if (BUTTON_CHANGED(1) && BUTTON_STATUS(1) == BUTTON_PRESSED)
	{
	  int w;
	  for (w = 0; w < MAX_MAJOR_WINDOWS; w++)
	    {
	      if (!tui_win_list[w] || !tui_win_list[w]->generic.handle)
		continue;

	      struct tui_gen_win_info *gwi = &tui_win_list[w]->generic;

	      if (MOUSE_Y_POS == gwi->origin.y + gwi->height - 1
		  && MOUSE_X_POS > gwi->origin.x
		  && MOUSE_X_POS < gwi->origin.x + gwi->width - 1)
		{
		  tui_resizer = w;
		  break;
		}
	    }
	}
      else if (BUTTON_CHANGED(1) && BUTTON_STATUS(1) == BUTTON_MOVED
	       && tui_resizer >= 0)
	{
	  struct tui_gen_win_info *gwi = &tui_win_list[tui_resizer]->generic;
	  int new_height = MOUSE_Y_POS + 1 - gwi->origin.y;
	  if (new_height != gwi->height
	      && tui_adjust_win_heights (tui_win_list[tui_resizer],
					 new_height) == TUI_SUCCESS)
	    tui_update_gdb_sizes ();
	}
      else if (BUTTON_CHANGED(1) && BUTTON_STATUS(1) == BUTTON_RELEASED)
	{
	  tui_resizer = -1;
	}
      break;
    case '\f':
      break;
    default:
      /* We didn't recognize the character as a control character, so pass it
         through.  */
      return ch;
    }

  /* We intercepted the control character, so return 0 (which readline
     will interpret as a no-op).  */
  return 0;
}

/* See tui-command.h.  */

void
tui_refresh_cmd_win (void)
{
  WINDOW *w = TUI_CMD_WIN->generic.handle;

  wrefresh (w);

  /* FIXME: It's not clear why this is here.
     It was present in the original tui_puts code and is kept in order to
     not introduce some subtle breakage.  */
  fflush (stdout);
}

/* TUI display source window.

   Copyright (C) 1998-2018 Free Software Foundation, Inc.

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
#include <ctype.h>
#include "symtab.h"
#include "frame.h"
#include "breakpoint.h"
#include "source.h"
#include "symtab.h"
#include "objfiles.h"
#include "filenames.h"
#include "language.h"

#include "tui/tui.h"
#include "tui/tui-data.h"
#include "tui/tui-stack.h"
#include "tui/tui-winsource.h"
#include "tui/tui-source.h"
#include "gdb_curses.h"


#ifdef TUI_SYNTAX_HIGHLIGHT
extern int tui_can_syntax_highlight;

static const char *syntax_type_c_3[] = {
    "int",
    NULL
};
static const char *syntax_type_c_4[] = {
    "auto",
    "char",
    "enum",
    "long",
    "void",
    NULL
};
static const char *syntax_type_c_5[] = {
    "const",
    "float",
    "short",
    "union",
    NULL
};
static const char *syntax_type_c_6[] = {
    "double",
    "extern",
    "inline",
    "signed",
    "static",
    "struct",
    NULL
};
static const char *syntax_type_c_7[] = {
    "typedef",
    NULL
};
static const char *syntax_type_c_8[] = {
    "unsigned",
    "register",
    "restrict",
    "volatile",
    NULL
};
static const char **syntax_type_c[] = {
    NULL,
    NULL,
    syntax_type_c_3,
    syntax_type_c_4,
    syntax_type_c_5,
    syntax_type_c_6,
    syntax_type_c_7,
    syntax_type_c_8,
};

static const char *syntax_type_cpp_4[] = {
    "bool",
    NULL
};
static const char *syntax_type_cpp_5[] = {
    "class",
    NULL
};
static const char *syntax_type_cpp_6[] = {
    "export",
    NULL
};
static const char *syntax_type_cpp_7[] = {
    "alignas",
    "mutable",
    "virtual",
    "wchar_t",
    NULL
};
static const char *syntax_type_cpp_8[] = {
    "char16_t",
    "char32_t",
    "decltype",
    "explicit",
    "template",
    "typename",
    NULL
};
static const char *syntax_type_cpp_9[] = {
    "constexpr",
    "namespace",
    NULL
};
static const char *syntax_type_cpp_12[] = {
    "thread_local",
    NULL
};
static const char **syntax_type_cpp[] = {
    NULL,
    NULL,
    NULL,
    syntax_type_cpp_4,
    syntax_type_cpp_5,
    syntax_type_cpp_6,
    syntax_type_cpp_7,
    syntax_type_cpp_8,
    syntax_type_cpp_9,
    NULL,
    NULL,
    syntax_type_cpp_12,
};

static const char *syntax_keyword_c_2[] = {
    "do",
    "if",
    NULL
};
static const char *syntax_keyword_c_3[] = {
    "asm",
    "for",
    NULL
};
static const char *syntax_keyword_c_4[] = {
    "case",
    "else",
    "goto",
    NULL
};
static const char *syntax_keyword_c_5[] = {
    "break",
    "while",
    NULL
};
static const char *syntax_keyword_c_6[] = {
    "return",
    "sizeof",
    "switch",
    NULL
};
static const char *syntax_keyword_c_7[] = {
    "default",
    NULL
};
static const char *syntax_keyword_c_8[] = {
    "continue",
    NULL
};
static const char **syntax_keyword_c[] = {
    NULL,
    syntax_keyword_c_2,
    syntax_keyword_c_3,
    syntax_keyword_c_4,
    syntax_keyword_c_5,
    syntax_keyword_c_6,
    syntax_keyword_c_7,
    syntax_keyword_c_8,
};

static const char *syntax_keyword_cpp_2[] = {
    "or",
    NULL
};
static const char *syntax_keyword_cpp_3[] = {
    "and",
    "new",
    "not",
    "try",
    "xor",
    NULL
};
static const char *syntax_keyword_cpp_4[] = {
    "this",
    NULL
};
static const char *syntax_keyword_cpp_5[] = {
    "bitor",
    "catch",
    "compl",
    "or_eq",
    "throw",
    "using",
    NULL
};
static const char *syntax_keyword_cpp_6[] = {
    "and_eq",
    "bitand",
    "delete",
    "friend",
    "not_eq",
    "public",
    "typeid",
    "xor_eq",
    NULL
};
static const char *syntax_keyword_cpp_7[] = {
    "alignof",
    "private",
    NULL
};
static const char *syntax_keyword_cpp_8[] = {
    "noexcept",
    "operator",
    NULL
};
static const char *syntax_keyword_cpp_9[] = {
    "protected",
    NULL
};
static const char *syntax_keyword_cpp_10[] = {
    "const_cast",
    NULL
};
static const char *syntax_keyword_cpp_11[] = {
    "static_cast",
    NULL
};
static const char *syntax_keyword_cpp_12[] = {
    "dynamic_cast",
    NULL
};
static const char *syntax_keyword_cpp_13[] = {
    "static_assert",
    NULL
};
static const char *syntax_keyword_cpp_16[] = {
    "reinterpret_cast",
    NULL
};
static const char **syntax_keyword_cpp[] = {
    NULL,
    syntax_keyword_cpp_2,
    syntax_keyword_cpp_3,
    syntax_keyword_cpp_4,
    syntax_keyword_cpp_5,
    syntax_keyword_cpp_6,
    syntax_keyword_cpp_7,
    syntax_keyword_cpp_8,
    syntax_keyword_cpp_9,
    syntax_keyword_cpp_10,
    syntax_keyword_cpp_11,
    syntax_keyword_cpp_12,
    syntax_keyword_cpp_13,
    NULL,
    NULL,
    syntax_keyword_cpp_16,
};

static const char *syntax_preproc_2[] = {
    "if",
    NULL
};
static const char *syntax_preproc_4[] = {
    "elif",
    "else",
    "line",
    "warn",
    NULL
};
static const char *syntax_preproc_5[] = {
    "ifdef",
    "endif",
    "error",
    "undef",
    NULL
};
static const char *syntax_preproc_6[] = {
    "define",
    "ifndef",
    "pragma",
    NULL
};
static const char *syntax_preproc_7[] = {
    "include",
    NULL
};
static const char **syntax_preproc[] = {
    NULL,
    syntax_preproc_2,
    NULL,
    syntax_preproc_4,
    syntax_preproc_5,
    syntax_preproc_6,
    syntax_preproc_7,
};

static const char *syntax_literal_4[] = {
    "NULL",
    "true",
    NULL
};
static const char *syntax_literal_5[] = {
    "false",
    NULL
};
static const char *syntax_literal_7[] = {
    "nullptr",
    NULL
};
static const char **syntax_literal[] = {
    NULL,
    NULL,
    NULL,
    syntax_literal_4,
    syntax_literal_5,
    NULL,
    syntax_literal_7,
};

#define SYNTAX_HIGHLIGHT_EXTRA 16

static int tui_keyword_highlight (const char *word,
				  char *color_word,
				  int word_len,
				  char color,
				  const char ***keywords,
				  int maxlen)
{
  const char **kw;

  if (word_len>maxlen) return 0;

  kw = keywords[word_len - 1];
  if (!kw) return 0;

  while (*kw)
    {
      if (!memcmp (word, *kw, word_len))
	{
	  memset (color_word, color, word_len);
	  return 1;
	}

      kw++;
    }

  return 0;
}

enum
{
  COL_NORMAL,
  COL_LITERAL,
  COL_TYPE,
  COL_KEYWORD,
  COL_PREPROC,
  COL_COMMENT,
};

const char *
find_end_comment (const char *line, const char *line_end);
const char *
find_end_string (const char *line, const char *line_end);

static void
tui_syntax_highlight (enum language lang,
		      const char *src_line,
		      char *col_line,
		      int syntax_status)
{
  int preproc = 0;

  if( lang != language_c && lang != language_cplus ) return;

  if (syntax_status)
    {
      const char *(*find_end_func) (const char *, const char *);
      char color;
      int count;
      const char *line_start = src_line;
      const char *line_end = line_start + strlen (line_start);

      if (syntax_status == 1)
	{
	  find_end_func = &find_end_comment;
	  color = COL_COMMENT;
	}
      else
	{
	  find_end_func = &find_end_string;
	  color = COL_LITERAL;
	}

      src_line = find_end_func (line_start, line_end);
      if (!src_line)
	src_line = line_end;

      count = src_line - line_start;
      memset (col_line, color, count);
      col_line += count;

      preproc = 1;
    }

  while (src_line[0])
    {
      char c = src_line[0];

      if (!preproc && c != ' ')
	{
	  if (c == '#')
	    {
	      preproc = 2;
	      col_line[0] = COL_PREPROC;
	    }
	  else
	    preproc = 1;
	}

      if (c >= '0' && c <= '9')
	{
	  col_line[0] = COL_LITERAL;

	  c = src_line[1];
	  while ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
		 (c >= 'A' && c <= 'Z') || c == '_')
	    {
	      src_line++;
	      col_line++;
	      c = src_line[1];

	      col_line[0] = COL_LITERAL;
	    }

	  preproc = 1;
	}
      else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
	{
	  const char *word_start = src_line;
	  char *col_start = col_line;
	  int word_len;

	  c = src_line[1];
	  while ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
		 (c >= 'A' && c <= 'Z') || c == '_')
	    {
	      src_line++;
	      col_line++;
	      c = src_line[1];
	    }

	  word_len = src_line - word_start + 1;

	  do
	    {
	      if (preproc==2 &&
		  tui_keyword_highlight (word_start, col_start, word_len,
					 COL_PREPROC, syntax_preproc, 7))
		break;

	      if (tui_keyword_highlight (word_start, col_start, word_len,
					 COL_TYPE, syntax_type_c, 8))
		break;
	      if (lang==language_cplus &&
		  tui_keyword_highlight (word_start, col_start, word_len,
					 COL_TYPE, syntax_type_cpp, 12))
		break;

	      if (tui_keyword_highlight (word_start, col_start, word_len,
					 COL_KEYWORD, syntax_keyword_c, 8))
		break;
	      if (lang==language_cplus &&
		  tui_keyword_highlight (word_start, col_start, word_len,
					 COL_KEYWORD, syntax_keyword_cpp, 16))
		break;

	      if (tui_keyword_highlight (word_start, col_start, word_len,
					 COL_LITERAL, syntax_literal, 7))
		break;
	    }
	  while (0);

	  preproc = 1;
	}
      else if (c == '"')
	{
	  const char *string_start = src_line;
	  char *col_start = col_line;

	  while (src_line[1])
	    {
	      src_line++;
	      col_line++;
	      c = src_line[0];

	      if (c == '\\' && src_line[1])
		{
		  src_line++;
		  col_line++;
		  continue;
		}

	      if (c == '"')
		break;
	    }

	  memset (col_start, COL_LITERAL, (src_line - string_start) + 1);

	  preproc = 1;
	}
      else if (c == '\'')
	{
	  const char *char_start = src_line;
	  char *col_start = col_line;

	  if (char_start[1] == '\\' && char_start[2] )
	    src_line = strchr (char_start + 3, '\'');
	  else if (char_start[1])
	    src_line = strchr (char_start + 2, '\'');
	  else
	    src_line = NULL;
	  if (!src_line)
	    src_line = char_start + (strlen (char_start) - 1);

	  col_line += src_line - char_start;

	  memset (col_start, COL_LITERAL, (src_line - char_start) + 1);

	  preproc = 1;
	}
      else if (c == '/' && (src_line[1] == '/' || src_line[1] == '*'))
	{
	  const char *comment_start = src_line;
	  char *col_start = col_line;

	  src_line = comment_start[1]=='*' ?
	    strstr (comment_start + 2, "*/") : NULL;
	  if (!src_line)
	    src_line = comment_start + (strlen (comment_start) - 1);
	  else
	    src_line++;

	  col_line += src_line - comment_start;

	  memset (col_start, COL_COMMENT, (src_line - comment_start) + 1);

	  preproc = 1;
	}
      else if (c != ' ' && c != '#')
	{
	  preproc = 1;
	}

      src_line++;
      col_line++;
    }
}
#endif


/* Function to display source in the source window.  */
enum tui_status
tui_set_source_content (struct symtab *s, 
			int line_no,
			int noerror)
{
  enum tui_status ret = TUI_FAILURE;

  if (s != (struct symtab *) NULL)
    {
      FILE *stream;
      int i, desc, c, line_width, nlines;
      char *src_line = 0;
#ifdef TUI_SYNTAX_HIGHLIGHT
      char *col_line = 0;
#endif

      if ((ret = tui_alloc_source_buffer (TUI_SRC_WIN)) == TUI_SUCCESS)
	{
	  line_width = TUI_SRC_WIN->generic.width - 1;
	  /* Take hilite (window border) into account, when
	     calculating the number of lines.  */
	  nlines = (line_no + (TUI_SRC_WIN->generic.height - 2)) - line_no;
	  desc = open_source_file (s);
	  if (desc < 0)
	    {
	      if (!noerror)
		{
		  const char *filename = symtab_to_filename_for_display (s);
		  char *name = (char *) alloca (strlen (filename) + 100);

		  sprintf (name, "%s:%d", filename, line_no);
		  print_sys_errmsg (name, errno);
		}
	      ret = TUI_FAILURE;
	    }
	  else
	    {
	      if (s->line_charpos == 0)
		find_source_lines (s, desc);

	      if (line_no < 1 || line_no > s->nlines)
		{
		  close (desc);
		  printf_unfiltered ("Line number %d out of range; "
				     "%s has %d lines.\n",
				     line_no,
				     symtab_to_filename_for_display (s),
				     s->nlines);
		}
	      else if (lseek (desc, s->line_charpos[line_no - 1], 0) < 0)
		{
		  close (desc);
		  perror_with_name (symtab_to_filename_for_display (s));
		}
	      else
		{
		  int offset, cur_line_no, cur_line, cur_len, threshold;
		  struct tui_gen_win_info *locator
		    = tui_locator_win_info_ptr ();
                  struct tui_source_info *src
		    = &TUI_SRC_WIN->detail.source_info;
		  const char *s_filename = symtab_to_filename_for_display (s);
#ifdef TUI_SYNTAX_HIGHLIGHT
		  enum language lang;
#endif

                  if (TUI_SRC_WIN->generic.title)
                    xfree (TUI_SRC_WIN->generic.title);
                  TUI_SRC_WIN->generic.title = xstrdup (s_filename);

		  xfree (src->fullname);
		  src->fullname = xstrdup (symtab_to_fullname (s));

#ifdef TUI_SYNTAX_HIGHLIGHT
		  lang = deduce_language_from_filename (src->fullname);
		  if (lang == language_unknown)
		    {
		      struct frame_info *fi = get_selected_frame_if_set ();
		      if (fi)
			lang = get_frame_language (fi);
		    }
		  if (lang == language_unknown)
		    lang = language_cplus;
#endif

		  /* Determine the threshold for the length of the
                     line and the offset to start the display.  */
		  offset = src->horizontal_offset;
		  threshold = (line_width - 1) + offset;
#ifdef TUI_SYNTAX_HIGHLIGHT
		  if (tui_can_syntax_highlight)
		    threshold += SYNTAX_HIGHLIGHT_EXTRA;
#endif
		  stream = fdopen (desc, FOPEN_RT);
		  clearerr (stream);
		  cur_line = 0;
		  src->gdbarch = get_objfile_arch (SYMTAB_OBJFILE (s));
		  src->start_line_or_addr.loa = LOA_LINE;
		  cur_line_no = src->start_line_or_addr.u.line_no = line_no;
		  src_line =
		    (char *) xmalloc ((threshold + 1) * sizeof (char));
#ifdef TUI_SYNTAX_HIGHLIGHT
		  if (tui_can_syntax_highlight)
		    col_line =
		      (char *) xmalloc ((threshold + 1) * sizeof (char));
#endif
		  while (cur_line < nlines)
		    {
		      struct tui_win_element *element
			= TUI_SRC_WIN->generic.content[cur_line];

		      /* Get the first character in the line.  */
		      c = fgetc (stream);

#ifdef TUI_SYNTAX_HIGHLIGHT
		      if (tui_can_syntax_highlight)
			memset (col_line, 0, threshold + 1);
#endif

		      /* Init the line with the line number.  */
		      sprintf (src_line, "%-6d", cur_line_no);
		      cur_len = strlen (src_line);
		      i = cur_len - ((cur_len / tui_default_tab_len ())
				     * tui_default_tab_len ());
		      while (i < tui_default_tab_len ())
			{
			  src_line[cur_len] = ' ';
			  i++;
			  cur_len++;
			}
		      src_line[cur_len] = (char) 0;

		      /* Set whether element is the execution point
		         and whether there is a break point on it.  */
		      element->which_element.source.line_or_addr.loa =
			LOA_LINE;
		      element->which_element.source.line_or_addr.u.line_no =
			cur_line_no;
		      element->which_element.source.is_exec_point =
			(filename_cmp (locator->content[0]
				         ->which_element.locator.full_name,
				       symtab_to_fullname (s)) == 0
				         && cur_line_no
					      == locator->content[0]
						   ->which_element.locator.line_no);
		      if (c != EOF)
			{
			  i = strlen (src_line) - 1;
			  do
			    {
			      if ((c != '\n') && (c != '\r') 
				  && (++i < threshold))
				{
				  if (c < 040 && c != '\t')
				    {
				      src_line[i++] = '^';
				      src_line[i] = c + 0100;
				    }
				  else if (c == 0177)
				    {
				      src_line[i++] = '^';
				      src_line[i] = '?';
				    }
				  else
				    { /* Store the charcter in the
					 line buffer.  If it is a tab,
					 then translate to the correct
					 number of chars so we don't
					 overwrite our buffer.  */
				      if (c == '\t')
					{
					  int j, max_tab_len
					    = tui_default_tab_len ();

					  for (j = i - ((i / max_tab_len)
							* max_tab_len);
					       j < max_tab_len
						 && i < threshold;
					       i++, j++)
					    src_line[i] = ' ';
					  i--;
					}
				      else
					src_line[i] = c;
				    }
				  src_line[i + 1] = 0;
				}
			      else
				{ /* If we have not reached EOL, then
				     eat chars until we do.  */
				  while (c != EOF && c != '\n' && c != '\r')
				    c = fgetc (stream);
				  /* Handle non-'\n' end-of-line.  */
				  if (c == '\r' 
				      && (c = fgetc (stream)) != '\n' 
				      && c != EOF)
				    {
				       ungetc (c, stream);
				       c = '\r';
				    }
				  
				}
			    }
			  while (c != EOF && c != '\n' && c != '\r' 
				 && i < threshold 
				 && (c = fgetc (stream)));
			}
		      /* Now copy the line taking the offset into
			 account.  */
		      if (strlen (src_line) > offset)
			{
#ifdef TUI_SYNTAX_HIGHLIGHT
			  if (tui_can_syntax_highlight)
			    {
			      if (cur_line_no <= s->nlines)
				tui_syntax_highlight (lang,
						      src_line + cur_len,
						      col_line + cur_len,
						      s->line_charpos[s->nlines
						      + cur_line_no - 1]);

			      src_line[threshold-SYNTAX_HIGHLIGHT_EXTRA] = 0;

			      memcpy (TUI_SRC_WIN->generic.content[cur_line]
				      ->which_element.source.line +
				      line_width, &col_line[offset],
				      strlen(&src_line[offset]));
			    }
#endif

			  strcpy (TUI_SRC_WIN->generic.content[cur_line]->which_element.source.line,
				  &src_line[offset]);
			}
		      else
			TUI_SRC_WIN->generic.content[cur_line]
			  ->which_element.source.line[0] = (char) 0;
		      cur_line++;
		      cur_line_no++;
		    }
		  xfree (src_line);
#ifdef TUI_SYNTAX_HIGHLIGHT
		  xfree (col_line);
#endif
		  fclose (stream);
		  TUI_SRC_WIN->generic.content_size = nlines;
		  ret = TUI_SUCCESS;
		}
	    }
	}
    }
  return ret;
}


/* elz: This function sets the contents of the source window to empty
   except for a line in the middle with a warning message about the
   source not being available.  This function is called by
   tui_erase_source_contents(), which in turn is invoked when the
   source files cannot be accessed.  */

void
tui_set_source_content_nil (struct tui_win_info *win_info, 
			    const char *warning_string)
{
  int line_width;
  int n_lines;
  int curr_line = 0;

  line_width = win_info->generic.width - 1;
  n_lines = win_info->generic.height - 2;

  /* Set to empty each line in the window, except for the one which
     contains the message.  */
  while (curr_line < win_info->generic.content_size)
    {
      /* Set the information related to each displayed line to null:
         i.e. the line number is 0, there is no bp, it is not where
         the program is stopped.  */

      struct tui_win_element *element = win_info->generic.content[curr_line];

      element->which_element.source.line_or_addr.loa = LOA_LINE;
      element->which_element.source.line_or_addr.u.line_no = 0;
      element->which_element.source.is_exec_point = FALSE;
      element->which_element.source.has_break = FALSE;

      /* Set the contents of the line to blank.  */
      element->which_element.source.line[0] = (char) 0;

      /* If the current line is in the middle of the screen, then we
         want to display the 'no source available' message in it.
         Note: the 'weird' arithmetic with the line width and height
         comes from the function tui_erase_source_content().  We need
         to keep the screen and the window's actual contents in
         synch.  */

      if (curr_line == (n_lines / 2 + 1))
	{
	  int i;
	  int xpos;
	  int warning_length = strlen (warning_string);
	  char *src_line;

	  src_line = element->which_element.source.line;

	  if (warning_length >= ((line_width - 1) / 2))
	    xpos = 1;
	  else
	    xpos = (line_width - 1) / 2 - warning_length;

	  for (i = 0; i < xpos; i++)
	    src_line[i] = ' ';

	  sprintf (src_line + i, "%s", warning_string);

	  for (i = xpos + warning_length; i < line_width; i++)
	    src_line[i] = ' ';

	  src_line[i] = '\n';

	}			/* end if */

      curr_line++;

    }				/* end while */
}


/* Function to display source in the source window.  This function
   initializes the horizontal scroll to 0.  */
void
tui_show_symtab_source (struct gdbarch *gdbarch, struct symtab *s,
			struct tui_line_or_address line, 
			int noerror)
{
  TUI_SRC_WIN->detail.source_info.horizontal_offset = 0;
  tui_update_source_window_as_is (TUI_SRC_WIN, gdbarch, s, line, noerror);
}


/* Answer whether the source is currently displayed in the source
   window.  */
int
tui_source_is_displayed (const char *fullname)
{
  return (TUI_SRC_WIN != NULL
	  && TUI_SRC_WIN->generic.content_in_use 
	  && (filename_cmp (tui_locator_win_info_ptr ()->content[0]
			      ->which_element.locator.full_name,
			    fullname) == 0));
}


/* Scroll the source forward or backward vertically.  */
void
tui_vertical_source_scroll (enum tui_scroll_direction scroll_direction,
			    int num_to_scroll)
{
  if (TUI_SRC_WIN->generic.content != NULL)
    {
      struct tui_line_or_address l;
      struct symtab *s;
      tui_win_content content = (tui_win_content) TUI_SRC_WIN->generic.content;
      struct symtab_and_line cursal = get_current_source_symtab_and_line ();

      if (cursal.symtab == (struct symtab *) NULL)
	s = find_pc_line_symtab (get_frame_pc (get_selected_frame (NULL)));
      else
	s = cursal.symtab;

      l.loa = LOA_LINE;
      if (scroll_direction == FORWARD_SCROLL)
	{
	  l.u.line_no = content[0]->which_element.source.line_or_addr.u.line_no
	    + num_to_scroll;
	  if (l.u.line_no > s->nlines)
	    /* line = s->nlines - win_info->generic.content_size + 1; */
	    /* elz: fix for dts 23398.  */
	    l.u.line_no
	      = content[0]->which_element.source.line_or_addr.u.line_no;
	}
      else
	{
	  l.u.line_no = content[0]->which_element.source.line_or_addr.u.line_no
	    - num_to_scroll;
	  if (l.u.line_no <= 0)
	    l.u.line_no = 1;
	}

      print_source_lines (s, l.u.line_no, l.u.line_no + 1, 0);
    }
}

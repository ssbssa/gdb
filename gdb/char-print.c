/* Character and string printing

   Copyright (C) 2025, 2026 Free Software Foundation, Inc.

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

#include "char-print.h"
#include "event-top.h"
#include "extract-store-integer.h"
#include "valprint.h"
#include "value.h"

/* See char-print.h.  */

bool
wchar_printer::printable (gdb_wchar_t w) const
{
  if (w == LCST ('\a') || w == LCST ('\b')
      || w == LCST ('\f') || w == LCST ('\n')
      || w == LCST ('\r') || w == LCST ('\t')
      || w == LCST ('\v'))
    return true;
  if (!gdb_iswprint (w))
    return false;
  /* If we previously emitted a hex escape, then we may need to emit
     an escape again, if W is a hex digit.  */
  if (!m_need_escape)
    return true;
  return !gdb_iswxdigit (w);
}

/* See char-print.h.  */

bool
wchar_printer::printable_and_convertible (gdb_wchar_t w) const
{
  if (!printable (w))
    return false;

  /* We think it is printable -- but is it really?  It must also be
     convertible from the intermediate charset (normally wchar_t) to
     the host charset, which is not always possible.  Note we don't
     try to handle stateful encodings, this seems difficult and also
     somewhat pointless.  And, if the host encoding is UTF-8, we
     simply assume this is fine without checking any more.  */
  if (m_host_utf8)
    return true;

  mbstate_t state;
  memset (&state, 0, sizeof (state));
  char ignore[MB_LEN_MAX];
  return wcrtomb (ignore, w, &state) != (size_t) -1;
}

/* See char-print.h.  */

void
wchar_printer::print_char (gdb_wchar_t w)
{
  m_need_escape = false;

  switch (w)
    {
    case LCST ('\a'):
      m_file.write (LCST ("\\a"));
      break;
    case LCST ('\b'):
      m_file.write (LCST ("\\b"));
      break;
    case LCST ('\f'):
      m_file.write (LCST ("\\f"));
      break;
    case LCST ('\n'):
      m_file.write (LCST ("\\n"));
      break;
    case LCST ('\r'):
      m_file.write (LCST ("\\r"));
      break;
    case LCST ('\t'):
      m_file.write (LCST ("\\t"));
      break;
    case LCST ('\v'):
      m_file.write (LCST ("\\v"));
      break;
    default:
      if (w == gdb_btowc (m_quoter) || w == LCST ('\\'))
	m_file.write (LCST ("\\"));
      m_file.write (w);
      break;
    }
}

/* See char-print.h.  */

void
wchar_printer::print_escape (const gdb_byte *orig, int orig_len)
{
  m_need_escape = false;

  int i;
  for (i = 0; i + m_width <= orig_len; i += m_width)
    {
      ULONGEST value;

      value = extract_unsigned_integer (&orig[i], m_width,
					m_byte_order);
      /* If the value fits in 3 octal digits, print it that
	 way.  Otherwise, print it as a hex escape.  */
      if (value <= 0777)
	{
	  gdb_printf (&m_file, "\\%.3o", (int) (value & 0777));
	  m_need_escape = false;
	}
      else
	{
	  gdb_printf (&m_file, "\\x%lx", (long) value);
	  /* A hex escape might require the next character
	     to be escaped, because, unlike with octal,
	     hex escapes have no length limit.  */
	  m_need_escape = true;
	}
    }

  /* If we somehow have extra bytes, print them now.  */
  while (i < orig_len)
    {
      gdb_printf (&m_file, "\\%.3o", orig[i] & 0xff);
      m_need_escape = false;
      ++i;
    }
}

const char *
wchar_printer::get_default_encoding (type *chtype)
{
  const char *encoding;
  if (chtype->length () == 1)
    encoding = target_charset (chtype->arch ());
  else if (chtype->name () != nullptr && streq (chtype->name (), "wchar_t"))
    encoding = target_wide_charset (chtype->arch ());
  else if (chtype->length () == 2)
    {
      if (type_byte_order (chtype) == BFD_ENDIAN_BIG)
	encoding = "UTF-16BE";
      else
	encoding = "UTF-16LE";
    }
  else if (chtype->length () == 4)
    {
      if (type_byte_order (chtype) == BFD_ENDIAN_BIG)
	encoding = "UTF-32BE";
      else
	encoding = "UTF-32LE";
    }
  else
    {
      /* No idea.  */
      encoding = target_charset (chtype->arch ());
    }
  return encoding;
}

void
wchar_printer::print (int c, ui_file *stream)
{
  gdb_byte *c_buf = (gdb_byte *) alloca (m_width);
  pack_long (c_buf, m_type, c);

  gdb_putc (m_quoter, stream);
  wchar_iterator iter (c_buf, m_width, m_encoding, m_width);

  while (1)
    {
      int num_chars;
      gdb_wchar_t *chars;
      const gdb_byte *buf;
      size_t buflen;
      bool need_escape = true;
      enum wchar_iterate_result result;

      num_chars = iter.iterate (&result, &chars, &buf, &buflen);
      if (num_chars < 0)
	break;
      if (num_chars > 0)
	{
	  /* If all characters are printable, print them.  Otherwise,
	     we're going to have to print an escape sequence.  We
	     check all characters because we want to print the target
	     bytes in the escape sequence, and we don't know character
	     boundaries there.  */
	  int i;

	  need_escape = false;
	  for (i = 0; i < num_chars; ++i)
	    if (!printable_and_convertible (chars[i]))
	      {
		need_escape = true;
		break;
	      }

	  if (!need_escape)
	    {
	      for (i = 0; i < num_chars; ++i)
		print_char (chars[i]);
	    }
	}

      /* This handles the NUM_CHARS == 0 case as well.  */
      if (need_escape)
	print_escape (buf, buflen);
    }

  /* The output in the host encoding.  */
  auto_obstack output;

  convert_between_encodings (INTERMEDIATE_ENCODING, host_charset (),
			     (gdb_byte *) obstack_base (&m_wchar_buf),
			     obstack_object_size (&m_wchar_buf),
			     sizeof (gdb_wchar_t), &output, translit_char);
  obstack_1grow (&output, '\0');

  gdb_puts ((const char *) obstack_base (&output), stream);
  gdb_putc (m_quoter, stream);
}

/* See char-print.h.  */

int
wchar_printer::count_next_character (wchar_iterator *iter,
				     std::vector<converted_character> *vec)
{
  struct converted_character *current;

  if (vec->empty ())
    {
      struct converted_character tmp;
      gdb_wchar_t *chars;

      tmp.num_chars
	= iter->iterate (&tmp.result, &chars, &tmp.buf, &tmp.buflen);
      if (tmp.num_chars > 0)
	{
	  gdb_assert (tmp.num_chars < MAX_WCHARS);
	  memcpy (tmp.chars, chars, tmp.num_chars * sizeof (gdb_wchar_t));
	}
      vec->push_back (tmp);
    }

  current = &vec->back ();

  /* Count repeated characters or bytes.  */
  current->repeat_count = 1;
  if (current->num_chars == -1)
    {
      /* EOF */
      return -1;
    }
  else
    {
      gdb_wchar_t *chars;
      struct converted_character d;
      int repeat;

      d.repeat_count = 0;

      while (1)
	{
	  if (current->num_chars > 0
	      && iter->bytes () >= current->buflen
	      && memcmp (current->buf, iter->input (), current->buflen) == 0)
	    {
	      iter->eat_bytes (current->buflen);
	      ++current->repeat_count;
	      continue;
	    }

	  /* Get the next character.  */
	  d.num_chars = iter->iterate (&d.result, &chars, &d.buf, &d.buflen);

	  /* If a character was successfully converted, save the character
	     into the converted character.  */
	  if (d.num_chars > 0)
	    {
	      gdb_assert (d.num_chars < MAX_WCHARS);
	      memcpy (d.chars, chars, d.num_chars * sizeof (gdb_wchar_t));
	    }

	  /* Determine if the current character is the same as this
	     new character.  */
	  if (d.num_chars == current->num_chars && d.result == current->result)
	    {
	      /* There are two cases to consider:

		 1) Equality of converted character (num_chars > 0)
		 2) Equality of non-converted character (num_chars == 0)  */
	      if ((current->num_chars > 0
		   && memcmp (current->chars, d.chars,
			      current->num_chars * sizeof (gdb_wchar_t)) == 0)
		  || (current->num_chars == 0
		      && current->buflen == d.buflen
		      && memcmp (current->buf, d.buf, current->buflen) == 0))
		++current->repeat_count;
	      else
		break;
	    }
	  else
	    break;
	}

      /* Push this next converted character onto the result vector.  */
      repeat = current->repeat_count;
      vec->push_back (d);
      return repeat;
    }
}

/* See char-print.h.  */

void
wchar_printer::print_converted_chars_to_obstack
     (const std::vector<converted_character> &chars,
      const struct value_print_options *options,
      int *finished)
{
  unsigned int idx, num_elements;
  const converted_character *elem;
  enum {START, SINGLE, REPEAT, INCOMPLETE, FINISH} state, last;
  gdb_wchar_t wide_quote_char = gdb_btowc (m_quoter);
  const int print_max = options->print_max_chars > 0
      ? options->print_max_chars : options->print_max;

  /* Set the start state.  */
  idx = num_elements = 0;
  last = state = START;
  elem = NULL;

  while (1)
    {
      switch (state)
	{
	case START:
	  /* Nothing to do.  */
	  break;

	case SINGLE:
	  {
	    int j;

	    /* We are outputting a single character
	       (< options->repeat_count_threshold).  */

	    if (last != SINGLE)
	      {
		/* We were outputting some other type of content, so we
		   must output and a comma and a quote.  */
		if (last != START)
		  m_file.write (LCST (", "));
		m_file.write (wide_quote_char);
	      }
	    /* Output the character.  */
	    int repeat_count = elem->repeat_count;
	    if (print_max < repeat_count + num_elements)
	      {
		repeat_count = print_max - num_elements;
		*finished = 0;
	      }
	    for (j = 0; j < repeat_count; ++j)
	      {
		if (elem->result == wchar_iterate_ok
		    && printable_and_convertible (elem->chars[0]))
		  print_char (elem->chars[0]);
		else
		  print_escape (elem->buf, elem->buflen);
		num_elements += 1;
	      }
	  }
	  break;

	case REPEAT:
	  {
	    int j;

	    /* We are outputting a character with a repeat count
	       greater than options->repeat_count_threshold.  */

	    if (last == SINGLE)
	      {
		/* We were outputting a single string.  Terminate the
		   string.  */
		m_file.write (wide_quote_char);
	      }
	    if (last != START)
	      m_file.write (LCST (", "));

	    /* Output the character and repeat string.  */
	    m_file.write (LCST ("'"));
	    if (elem->result == wchar_iterate_ok
		&& printable_and_convertible (elem->chars[0]))
	      print_char (elem->chars[0]);
	    else
	      print_escape (elem->buf, elem->buflen);
	    m_file.write (LCST ("'"));
	    std::string s = string_printf (_(" <repeats %u times>"),
					   elem->repeat_count);
	    num_elements += elem->repeat_count;
	    for (j = 0; s[j]; ++j)
	      {
		gdb_wchar_t w = gdb_btowc (s[j]);
		m_file.write (w);
	      }
	  }
	  break;

	case INCOMPLETE:
	  /* We are outputting an incomplete sequence.  */
	  if (last == SINGLE)
	    {
	      /* If we were outputting a string of SINGLE characters,
		 terminate the quote.  */
	      m_file.write (wide_quote_char);
	    }
	  if (last != START)
	    m_file.write (LCST (", "));

	  /* Output the incomplete sequence string.  */
	  m_file.write (LCST ("<incomplete sequence "));
	  print_escape (elem->buf, elem->buflen);
	  m_file.write (LCST (">"));
	  num_elements += 1;

	  /* We do not attempt to output anything after this.  */
	  state = FINISH;
	  break;

	case FINISH:
	  /* All done.  If we were outputting a string of SINGLE
	     characters, the string must be terminated.  Otherwise,
	     REPEAT and INCOMPLETE are always left properly terminated.  */
	  if (last == SINGLE)
	    m_file.write (wide_quote_char);

	  return;
	}

      /* Get the next element and state.  */
      last = state;
      if (state != FINISH)
	{
	  elem = &chars[idx++];
	  switch (elem->result)
	    {
	    case wchar_iterate_ok:
	    case wchar_iterate_invalid:
	      if (elem->repeat_count > options->repeat_count_threshold)
		state = REPEAT;
	      else
		state = SINGLE;
	      break;

	    case wchar_iterate_incomplete:
	      state = INCOMPLETE;
	      break;

	    case wchar_iterate_eof:
	      state = FINISH;
	      break;
	    }
	}
    }
}

/* See char-print.h.  */

void
wchar_printer::print (struct ui_file *stream, const gdb_byte *string,
		      unsigned int length, int force_ellipses,
		      int c_style_terminator,
		      const struct value_print_options *options)
{
  unsigned int i;
  int finished = 0;
  struct converted_character *last;

  if (length == -1)
    {
      unsigned long current_char = 1;

      for (i = 0; current_char; ++i)
	{
	  QUIT;
	  current_char = extract_unsigned_integer (string + i * m_width,
						   m_width, m_byte_order);
	}
      length = i;
    }

  /* If the string was not truncated due to `set print elements', and
     the last byte of it is a null, we don't print that, in
     traditional C style.  */
  if (c_style_terminator
      && !force_ellipses
      && length > 0
      && (extract_unsigned_integer (string + (length - 1) * m_width,
				    m_width, m_byte_order) == 0))
    length--;

  if (length == 0)
    {
      gdb_printf (stream, "%c%c", m_quoter, m_quoter);
      return;
    }

  /* Arrange to iterate over the characters, in wchar_t form.  */
  wchar_iterator iter (string, length * m_width, m_encoding, m_width);
  std::vector<converted_character> converted_chars;

  /* Convert characters until the string is over or the maximum
     number of printed characters has been reached.  */
  i = 0;
  unsigned int print_max_chars = get_print_max_chars (options);
  while (i < print_max_chars)
    {
      int r;

      QUIT;

      /* Grab the next character and repeat count.  */
      r = count_next_character (&iter, &converted_chars);

      /* If less than zero, the end of the input string was reached.  */
      if (r < 0)
	break;

      /* Otherwise, add the count to the total print count and get
	 the next character.  */
      i += r;
    }

  /* Get the last element and determine if the entire string was
     processed.  */
  last = &converted_chars.back ();
  finished = (last->result == wchar_iterate_eof);

  /* Ensure that CONVERTED_CHARS is terminated.  */
  last->result = wchar_iterate_eof;

  /* Print the output string to the obstack.  */
  print_converted_chars_to_obstack (converted_chars, options, &finished);

  if (force_ellipses || !finished)
    m_file.write (LCST ("..."));

  /* OUTPUT is where we collect `char's for printing.  */
  auto_obstack output;

  convert_between_encodings (INTERMEDIATE_ENCODING, host_charset (),
			     (gdb_byte *) obstack_base (&m_wchar_buf),
			     obstack_object_size (&m_wchar_buf),
			     sizeof (gdb_wchar_t), &output, translit_char);
  obstack_1grow (&output, '\0');

  gdb_puts ((const char *) obstack_base (&output), stream);
}

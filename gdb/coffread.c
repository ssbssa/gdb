/* Read coff symbol tables and convert to internal format, for GDB.
   Copyright (C) 1987-2026 Free Software Foundation, Inc.
   Contributed by David D. Johnson, Brown University (ddj@cs.brown.edu).

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
#include "symtab.h"
#include "gdbtypes.h"
#include "bfd.h"
#include "coff/internal.h"
#include "libcoff.h"
#include "objfiles.h"
#include "target.h"
#include "dwarf2/public.h"
#include "coff-pe-read.h"
#include "cli/cli-style.h"

/* Simplified internal version of coff symbol table information.  */

struct coff_symbol
{
  const char *c_name;
  /* 0 if syment only, 1 if syment + auxent, etc.  */
  int c_naux;
  CORE_ADDR c_value;
  int c_sclass;
  int c_secnum;
  unsigned int c_type;
};

/* A class that reads symbols from a COFF file.  */
struct coff_reader
{
  explicit coff_reader (objfile *objfile)
    : coffread_objfile (objfile),
      symfile_bfd (objfile->obfd.get ())
  {
  }

  /* Do the work of reading.  */
  void symfile_read (symfile_add_flags symfile_flags);

private:

  /* The objfile we are currently reading.  */
  objfile *coffread_objfile;

  /* The BFD for this file.  */
  bfd *symfile_bfd;

  /* Pointers to scratch storage, used for reading raw symbols and
     auxents.  */
  char *temp_sym;
  char *temp_aux;

  /* Local variables that hold the shift and mask values for the
     COFF file that we are currently reading.  These come back to us
     from BFD, and are referenced by their macro names, as well as
     internally to the ISFCN macro from include/coff/internal.h .  */
  unsigned local_n_btshft;
  unsigned local_n_tmask;

#define	N_BTSHFT	local_n_btshft
#define	N_TMASK		local_n_tmask

  /* Local variables that hold the sizes in the file of various COFF
     structures.  (We only need to know this to read them from the file
     -- BFD will then translate the data in them, into `internal_xxx'
     structs in the right byte order, alignment, etc.)  */
  unsigned local_symesz;
  unsigned local_auxesz;

  /* This is set if this is a PE format file.  */
  bool pe_file;

  char *stringtab = NULL;
  long stringtab_length = 0;

  /* Used when reading coff symbols.  */
  int symnum;

  asection *cs_to_bfd_section (coff_symbol *cs);
  int cs_to_section (coff_symbol *cs);
  CORE_ADDR cs_section_address (coff_symbol *cs);
  bool is_import_fixup_symbol (coff_symbol *cs, minimal_symbol_type type);
  minimal_symbol *record_minimal_symbol (minimal_symbol_reader &reader,
					 coff_symbol *cs,
					 unrelocated_addr address,
					 minimal_symbol_type type,
					 int section);
  void read_minsyms (file_ptr symtab_offset, unsigned int nsyms);
  void symtab_read (minimal_symbol_reader &reader, file_ptr symtab_offset,
		    unsigned int nsyms);
  void read_one_sym (coff_symbol *cs);
  int init_stringtab (file_ptr offset,
		      gdb::unique_xmalloc_ptr<char> *storage);
  const char *getsymname (struct internal_syment *symbol_entry);
};

/* Return the BFD section that CS points to.  */

asection *
coff_reader::cs_to_bfd_section (struct coff_symbol *cs)
{
  for (asection *sect : gdb_bfd_sections (symfile_bfd))
    if (sect->target_index == cs->c_secnum)
      return sect;

  return nullptr;
}

/* Return the section number (SECT_OFF_*) that CS points to.  */
int
coff_reader::cs_to_section (struct coff_symbol *cs)
{
  asection *sect = cs_to_bfd_section (cs);

  if (sect == NULL)
    return SECT_OFF_TEXT (coffread_objfile);
  return gdb_bfd_section_index (symfile_bfd, sect);
}

/* Return the address of the section of a COFF symbol.  */

CORE_ADDR
coff_reader::cs_section_address (struct coff_symbol *cs)
{
  asection *sect = cs_to_bfd_section (cs);

  if (sect == nullptr)
    return 0;

  return bfd_section_vma (sect);
}

/* The linker sometimes generates some non-function symbols inside
   functions referencing variables imported from another DLL.
   Return true if the given symbol corresponds to one of them.  */

bool
coff_reader::is_import_fixup_symbol (struct coff_symbol *cs,
				     enum minimal_symbol_type type)
{
  /* The following is a bit of a heuristic using the characteristics
     of these fixup symbols, but should work well in practice...  */
  int i;

  /* Must be a non-static text symbol.  */
  if (type != mst_text)
    return false;

  /* Must be a non-function symbol.  */
  if (ISFCN (cs->c_type))
    return false;

  /* The name must start with "__fu<digits>__".  */
  if (!startswith (cs->c_name, "__fu"))
    return false;
  if (! c_isdigit (cs->c_name[4]))
    return false;
  for (i = 5; cs->c_name[i] != '\0' && c_isdigit (cs->c_name[i]); i++)
    /* Nothing, just incrementing index past all digits.  */;
  if (cs->c_name[i] != '_' || cs->c_name[i + 1] != '_')
    return false;

  return true;
}

struct minimal_symbol *
coff_reader::record_minimal_symbol (minimal_symbol_reader &reader,
				    struct coff_symbol *cs,
				    unrelocated_addr address,
				    enum minimal_symbol_type type,
				    int section)
{
  /* We don't want TDESC entry points in the minimal symbol table.  */
  if (cs->c_name[0] == '@')
    return NULL;

  if (is_import_fixup_symbol (cs, type))
    {
      /* Because the value of these symbols is within a function code
	 range, these symbols interfere with the symbol-from-address
	 reverse lookup; this manifests itself in backtraces, or any
	 other commands that prints symbolic addresses.  Just pretend
	 these symbols do not exist.  */
      return NULL;
    }

  return reader.record_full (cs->c_name, true, address, type, section);
}

/* A helper function for coff_symfile_read that reads minimal
   symbols.  It may also read other forms of symbol as well.  */

void
coff_reader::read_minsyms (file_ptr symtab_offset, unsigned int nsyms)
{
  /* If minimal symbols were already read, and if we know we aren't
     going to read any other kind of symbol here, then we can just
     return.  */
  if (coffread_objfile->per_bfd->minsyms_read && pe_file && nsyms == 0)
    return;

  minimal_symbol_reader reader (coffread_objfile);

  if (pe_file && nsyms == 0)
    {
      /* We've got no debugging symbols, but it's a portable
	 executable, so try to read the export table.  */
      read_pe_exported_syms (reader, coffread_objfile);
    }
  else
    {
      /* Now that the executable file is positioned at symbol table,
	 process it and define symbols accordingly.  */
      symtab_read (reader, symtab_offset, nsyms);
    }

  /* Install any minimal symbols that have been collected as the
     current minimal symbols for this objfile.  */

  reader.install ();

  if (pe_file)
    {
      for (minimal_symbol *msym : coffread_objfile->msymbols ())
	{
	  const char *name = msym->linkage_name ();

	  /* If the minimal symbols whose name are prefixed by "__imp_"
	     or "_imp_", get rid of the prefix, and search the minimal
	     symbol in OBJFILE.  Note that 'maintenance print msymbols'
	     shows that type of these "_imp_XXXX" symbols is mst_data.  */
	  if (msym->type () == mst_data)
	    {
	      const char *name1 = NULL;

	      if (startswith (name, "_imp_"))
		name1 = name + 5;
	      else if (startswith (name, "__imp_"))
		name1 = name + 6;
	      if (name1 != NULL)
		{
		  int lead = bfd_get_symbol_leading_char (symfile_bfd);

		  if (lead != '\0' && *name1 == lead)
		    name1 += 1;

		  bound_minimal_symbol found
		    = lookup_minimal_symbol (current_program_space, name1,
					     coffread_objfile);

		  /* If found, there are symbols named "_imp_foo" and "foo"
		     respectively in OBJFILE.  Set the type of symbol "foo"
		     as 'mst_solib_trampoline'.  */
		  if (found.minsym != NULL
		      && found.minsym->type () == mst_text)
		    found.minsym->set_type (mst_solib_trampoline);
		}
	    }
	}
    }
}

/* Read a symbol file, after initialization by coff_symfile_init.  */

void
coff_reader::symfile_read (symfile_add_flags symfile_flags)
{
  coff_data_type *cdata = coff_data (symfile_bfd);
  const char *filename = bfd_get_filename (symfile_bfd);
  int val;
  unsigned int num_symbols;
  file_ptr symtab_offset;
  file_ptr stringtab_offset;

  /* WARNING WILL ROBINSON!  ACCESSING BFD-PRIVATE DATA HERE!  FIXME!  */
  num_symbols = bfd_get_symcount (symfile_bfd); /* How many syms */
  symtab_offset = cdata->sym_filepos;	/* Symbol table file offset */
  stringtab_offset = symtab_offset +	/* String table file offset */
    num_symbols * cdata->local_symesz;

  /* Set a few file-statics that give us specific information about
     the particular COFF file format we're reading.  */
  local_n_btshft = cdata->local_n_btshft;
  local_n_tmask = cdata->local_n_tmask;
  local_symesz = cdata->local_symesz;
  local_auxesz = cdata->local_auxesz;

  /* Allocate space for raw symbol and aux entries, based on their
     space requirements as reported by BFD.  */
  gdb::def_vector<char> temp_storage (cdata->local_symesz
				      + cdata->local_auxesz);
  temp_sym = temp_storage.data ();
  temp_aux = temp_sym + cdata->local_symesz;

  /* We need to know whether this is a PE file, because in PE files,
     unlike standard COFF files, symbol values are stored as offsets
     from the section address, rather than as absolute addresses.
     FIXME: We should use BFD to read the symbol table, and thus avoid
     this problem.  */
  pe_file = (startswith (bfd_get_target (symfile_bfd), "pe")
	     || startswith (bfd_get_target (symfile_bfd), "epoc-pe"));

  /* End of warning.  */

  /* Now read the string table, all at once.  */

  scoped_restore restore_stringtab = make_scoped_restore (&stringtab);
  gdb::unique_xmalloc_ptr<char> stringtab_storage;
  val = init_stringtab (stringtab_offset, &stringtab_storage);
  if (val < 0)
    error (_("\"%ps\": can't get string table"),
	   styled_string (file_name_style.style (), filename));

  read_minsyms (symtab_offset, num_symbols);

  if (!(coffread_objfile->flags & OBJF_READNEVER))
    {
      bool found_stab_section = false;

      for (asection *sect : gdb_bfd_sections (symfile_bfd))
	if (startswith (bfd_section_name (sect), ".stab"))
	  {
	    found_stab_section = true;
	    break;
	  }

      if (found_stab_section)
	warning (_ ("stabs debug information is not supported."));
    }

  if (dwarf2_initialize_objfile (coffread_objfile))
    {
      /* Nothing.  */
    }

  /* Try to add separate debug file if no symbols table found.   */
  else if (!coffread_objfile->has_partial_symbols ()
	   && coffread_objfile->separate_debug_objfile == NULL
	   && coffread_objfile->separate_debug_objfile_backlink == NULL)
    {
      if (coffread_objfile->find_and_add_separate_symbol_file (symfile_flags))
	gdb_assert (coffread_objfile->separate_debug_objfile != nullptr);
    }
}

static void
coff_symfile_read (struct objfile *objfile, symfile_add_flags symfile_flags)
{
  coff_reader reader (objfile);
  reader.symfile_read (symfile_flags);
}

/* Given pointers to a symbol table in coff style exec file,
   analyze them and create struct symtab's describing the symbols.
   NSYMS is the number of symbols in the symbol table.
   We read them one at a time using read_one_sym ().  */

void
coff_reader::symtab_read (minimal_symbol_reader &reader,
			  file_ptr symtab_offset, unsigned int nsyms)
{
  struct gdbarch *gdbarch = coffread_objfile->arch ();
  struct coff_symbol coff_symbol;
  struct coff_symbol *cs = &coff_symbol;
  int val;
  CORE_ADDR tmpaddr;
  struct minimal_symbol *msym;

  symnum = 0;

  /* Position to read the symbol table.  */
  val = bfd_seek (symfile_bfd, symtab_offset, 0);
  if (val < 0)
    error (_("Error reading symbols from %ps: %s"),
	   styled_string (file_name_style.style (),
			  objfile_name (coffread_objfile)),
	   bfd_errmsg (bfd_get_error ()));

  while (symnum < nsyms)
    {
      QUIT;			/* Make this command interruptible.  */

      read_one_sym (cs);

      /* Typedefs should not be treated as symbol definitions.  */
      if (ISFCN (cs->c_type) && cs->c_sclass != C_TPDEF)
	{
	  /* Record all functions -- external and static -- in
	     minsyms.  */
	  int section = cs_to_section (cs);

	  tmpaddr = cs->c_value;
	  /* Don't record unresolved symbols.  */
	  if (!(cs->c_secnum <= 0 && cs->c_value == 0))
	    record_minimal_symbol (reader, cs,
				   unrelocated_addr (tmpaddr),
				   mst_text, section);

	  continue;
	}

      switch (cs->c_sclass)
	{
	  /* C_LABEL is used for labels and static functions.
	     Including it here allows gdb to see static functions when
	     no debug info is available.  */
	case C_LABEL:
	case C_STAT:
	case C_THUMBLABEL:
	case C_THUMBSTAT:
	case C_THUMBSTATFUNC:
	  if (cs->c_name[0] == '.')
	    {
	      /* Flush rest of '.' symbols.  */
	      break;
	    }
	  else if (cs->c_name[0] == 'L'
		   && (startswith (cs->c_name, "LI%")
		       || startswith (cs->c_name, "LF%")
		       || startswith (cs->c_name, "LC%")
		       || startswith (cs->c_name, "LP%")
		       || startswith (cs->c_name, "LPB%")
		       || startswith (cs->c_name, "LBB%")
		       || startswith (cs->c_name, "LBE%")
		       || startswith (cs->c_name, "LPBX%")))
	    /* At least on a 3b1, gcc generates swbeg and string labels
	       that look like this.  Ignore them.  */
	    break;
	  /* For static symbols that don't start with '.'...  */
	  [[fallthrough]];
	case C_THUMBEXT:
	case C_THUMBEXTFUNC:
	case C_EXT:
	  {
	    /* Record it in the minimal symbols regardless of
	       SDB_TYPE.  This parallels what we do for other debug
	       formats, and probably is needed to make
	       print_address_symbolic work right without the (now
	       gone) "set fast-symbolic-addr off" kludge.  */

	    enum minimal_symbol_type ms_type;
	    int sec;

	    if (cs->c_secnum == N_UNDEF)
	      {
		/* This is a common symbol.  We used to rely on
		   the target to tell us whether it knows where
		   the symbol has been relocated to, but none of
		   the target implementations actually provided
		   that operation.  So we just ignore the symbol,
		   the same way we would do if we had a target-side
		   symbol lookup which returned no match.  */
		break;
	      }
	    else if (cs->c_secnum == N_ABS)
	      {
		/* Use the correct minimal symbol type (and don't
		   relocate) for absolute values.  */
		ms_type = mst_abs;
		sec = cs_to_section (cs);
		tmpaddr = cs->c_value;
	      }
	    else
	      {
		asection *bfd_section = cs_to_bfd_section (cs);

		sec = cs_to_section (cs);
		tmpaddr = cs->c_value;

		if (bfd_section->flags & SEC_CODE)
		  {
		    ms_type =
		      cs->c_sclass == C_EXT || cs->c_sclass == C_THUMBEXTFUNC
		      || cs->c_sclass == C_THUMBEXT ?
		      mst_text : mst_file_text;
		    tmpaddr = gdbarch_addr_bits_remove (gdbarch, tmpaddr);
		  }
		else if (bfd_section->flags & SEC_ALLOC
			 && bfd_section->flags & SEC_LOAD)
		  {
		    ms_type =
		      cs->c_sclass == C_EXT || cs->c_sclass == C_THUMBEXT
		      ? mst_data : mst_file_data;
		  }
		else if (bfd_section->flags & SEC_ALLOC)
		  {
		    ms_type =
		      cs->c_sclass == C_EXT || cs->c_sclass == C_THUMBEXT
		      ? mst_bss : mst_file_bss;
		  }
		else
		  ms_type = mst_unknown;
	      }

	    msym = record_minimal_symbol (reader, cs,
					  unrelocated_addr (tmpaddr),
					  ms_type, sec);
	    if (msym)
	      gdbarch_coff_make_msymbol_special (gdbarch,
						 cs->c_sclass, msym);
	  }
	  break;
	}
    }
}

/* Routines for reading headers and symbols from executable.  */

/* Read the next symbol into CS.  */

void
coff_reader::read_one_sym (struct coff_symbol *cs)
{
  int i;
  bfd_size_type bytes;
  internal_syment sym;

  bytes = bfd_read (temp_sym, local_symesz, symfile_bfd);
  if (bytes != local_symesz)
    error (_("%ps: error reading symbols"),
	   styled_string (file_name_style.style (),
			  objfile_name (coffread_objfile)));
  bfd_coff_swap_sym_in (symfile_bfd, temp_sym, &sym);
  cs->c_naux = sym.n_numaux & 0xff;
  if (cs->c_naux >= 1)
    {
      /* We don't need aux entries, read past them.  */
      for (i = 0; i < cs->c_naux; i++)
	{
	  bytes = bfd_read (temp_aux, local_auxesz, symfile_bfd);
	  if (bytes != local_auxesz)
	    error (_("%ps: error reading symbols"),
		   styled_string (file_name_style.style (),
				  objfile_name (coffread_objfile)));
	}
    }
  cs->c_name = getsymname (&sym);
  cs->c_value = sym.n_value;
  cs->c_sclass = (sym.n_sclass & 0xff);
  cs->c_secnum = sym.n_scnum;
  cs->c_type = (unsigned) sym.n_type;

  symnum += 1 + cs->c_naux;

  /* The PE file format stores symbol values as offsets within the
     section, rather than as absolute addresses.  We correct that
     here, if the symbol has an appropriate storage class.  FIXME: We
     should use BFD to read the symbols, rather than duplicating the
     work here.  */
  if (pe_file)
    {
      switch (cs->c_sclass)
	{
	case C_EXT:
	case C_THUMBEXT:
	case C_THUMBEXTFUNC:
	case C_SECTION:
	case C_NT_WEAK:
	case C_STAT:
	case C_THUMBSTAT:
	case C_THUMBSTATFUNC:
	case C_LABEL:
	case C_THUMBLABEL:
	case C_BLOCK:
	case C_FCN:
	case C_EFCN:
	  if (cs->c_secnum != 0)
	    cs->c_value += cs_section_address (cs);
	  break;
	}
    }
}

/* Support for string table handling.  */

int
coff_reader::init_stringtab (file_ptr offset,
			     gdb::unique_xmalloc_ptr<char> *storage)
{
  long length;
  int val;
  unsigned char lengthbuf[4];

  /* If the file is stripped, the offset might be zero, indicating no
     string table.  Just return with `stringtab' set to null.  */
  if (offset == 0)
    return 0;

  if (bfd_seek (symfile_bfd, offset, 0) < 0)
    return -1;

  val = bfd_read (lengthbuf, sizeof lengthbuf, symfile_bfd);
  /* If no string table is needed, then the file may end immediately
     after the symbols.  Just return with `stringtab' set to null.  */
  if (val != sizeof lengthbuf)
    return 0;
  length = bfd_h_get_32 (symfile_bfd, lengthbuf);
  if (length < sizeof lengthbuf)
    return 0;

  storage->reset ((char *) xmalloc (length));
  stringtab = storage->get ();
  /* This is in target format (probably not very useful, and not
     currently used), not host format.  */
  memcpy (stringtab, lengthbuf, sizeof lengthbuf);
  stringtab_length = length;
  if (length == sizeof length)	/* Empty table -- just the count.  */
    return 0;

  val = bfd_read (stringtab + sizeof lengthbuf,
		  length - sizeof lengthbuf, symfile_bfd);
  if (val != length - sizeof lengthbuf || stringtab[length - 1] != '\0')
    return -1;

  return 0;
}

const char *
coff_reader::getsymname (struct internal_syment *symbol_entry)
{
  static char buffer[SYMNMLEN + 1];
  const char *result;

  if (symbol_entry->_n._n_n._n_zeroes == 0)
    {
      if (symbol_entry->_n._n_n._n_offset >= stringtab_length)
	error (_("COFF Error: string table offset (%s) outside string table (length %ld)"),
	       hex_string (symbol_entry->_n._n_n._n_offset), stringtab_length);
      result = stringtab + symbol_entry->_n._n_n._n_offset;
    }
  else
    {
      strncpy (buffer, symbol_entry->_n._n_name, SYMNMLEN);
      buffer[SYMNMLEN] = '\0';
      result = buffer;
    }
  return result;
}

/* Register our ability to parse symbols for coff BFD files.  */

static const struct sym_fns coff_sym_fns =
{
  [] (objfile *) { },		/* sym_init: read initial info, setup
				   for sym_read() */
  coff_symfile_read,		/* sym_read: read a symbol file into
				   symtab */
  default_symfile_offsets,	/* sym_offsets: xlate external to
				   internal form */
  default_symfile_segments,	/* sym_segments: Get segment
				   information from a file */

  default_symfile_relocate,	/* sym_relocate: Relocate a debug
				   section.  */
  NULL,				/* sym_probe_fns */
};

INIT_GDB_FILE (coffread)
{
  add_symtab_fns (bfd_target_coff_flavour, &coff_sym_fns);
}

/* Build symbol tables in GDB's internal format.
   Copyright (C) 1986-2026 Free Software Foundation, Inc.

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

#ifndef GDB_BUILDSYM_H
#define GDB_BUILDSYM_H

#include "block.h"
#include "gdbsupport/gdb_obstack.h"
#include "symtab.h"
#include "addrmap.h"

struct objfile;
struct symbol;
struct compunit_symtab;
enum language;

/* This module provides definitions used for creating and adding to
   the symbol table.  These routines are called from various symbol-
   file-reading routines.

   They originated in dbxread.c of gdb-4.2, and were split out to
   make xcoffread.c more maintainable by sharing code.  */

struct block;
struct pending_block;

struct dynamic_prop;

/* The list of sub-source-files within the current individual
   compilation.  Each file gets its own symtab with its own linetable
   and associated info, but they all share one blockvector.  */

struct subfile
{
  subfile () = default;

  /* There's nothing wrong with copying a subfile, but we don't need to, so use
     this to avoid copying one by mistake.  */
  DISABLE_COPY_AND_ASSIGN (subfile);

  struct subfile *next = nullptr;
  std::string name;

  /* This field is analogous in function to symtab::filename_for_id.

     It is used to look up existing subfiles in calls to start_subfile.  */
  std::string name_for_id;

  std::vector<linetable_entry> line_vector_entries;
  enum language language = language_unknown;
  struct symtab *symtab = nullptr;
};

using subfile_up = std::unique_ptr<subfile>;

/* Flags associated with a linetable entry.  */

enum linetable_entry_flag : unsigned
{
  /* Indicates this PC is a good location to place a breakpoint at LINE.  */
  LEF_IS_STMT = 1 << 1,

  /* Indicates this PC is a good location to place a breakpoint at the first
     instruction past a function prologue.  */
  LEF_PROLOGUE_END = 1 << 2,

  /* Indicated that this PC is part of the epilogue of a function, making
     software watchpoints unreliable.  */
  LEF_EPILOGUE_BEGIN = 1 << 3,
};
DEF_ENUM_FLAGS_TYPE (enum linetable_entry_flag, linetable_entry_flags);


/* Buildsym's counterpart to struct compunit_symtab.  */

struct buildsym_compunit
{
  /* Start recording information about a primary source file (IOW, not an
     included source file).

     COMP_DIR is the directory in which the compilation unit was compiled
     (or NULL if not known).

     NAME and NAME_FOR_ID have the same purpose as for the start_subfile
     method.  */

  buildsym_compunit (struct objfile *objfile_, const char *name,
		     const char *comp_dir_, const char *name_for_id,
		     enum language language_, CORE_ADDR last_addr);

  /* Same as above, but passes NAME for NAME_FOR_ID.  */

  buildsym_compunit (struct objfile *objfile_, const char *name,
		     const char *comp_dir_, enum language language_,
		     CORE_ADDR last_addr)
    : buildsym_compunit (objfile_, name, comp_dir_, name, language_, last_addr)
  {}

  /* Reopen an existing compunit_symtab so that additional symbols can
     be added to it.  Arguments are as for the main constructor.  CUST
     is the expandable compunit_symtab to be reopened.  */

  buildsym_compunit (struct objfile *objfile_, const char *name,
		     const char *comp_dir_, enum language language_,
		     CORE_ADDR last_addr, struct compunit_symtab *cust)
    : m_objfile (objfile_),
      m_comp_dir (comp_dir_ == nullptr ? "" : comp_dir_),
      m_compunit_symtab (cust),
      m_language (language_),
      m_last_source_start_addr (last_addr)
  {
  }

  ~buildsym_compunit ();

  DISABLE_COPY_AND_ASSIGN (buildsym_compunit);

  struct macro_table *get_macro_table ();

  struct block *finish_block (struct symbol *symbol,
			      struct pending_block *old_blocks,
			      const struct dynamic_prop *static_link,
			      CORE_ADDR start, CORE_ADDR end);

  /* Start recording information about source code that comes from a source
     file.  This sets the current subfile, creating it if necessary.

     NAME is the user-visible name of the subfile.

     NAME_FOR_ID is a name that must be stable between the different calls to
     start_subfile referring to the same file (it is used for looking up
     existing subfiles).  It can be equal to NAME if NAME follows that rule.  */
  void start_subfile (const char *name, const char *name_for_id);

  void record_line (struct subfile *subfile, int line, int column,
		    unrelocated_addr pc, linetable_entry_flags flags);

  struct compunit_symtab *get_compunit_symtab ()
  {
    return m_compunit_symtab;
  }

  struct using_direct **get_local_using_directives ()
  {
    return &m_local_using_directives;
  }

  struct using_direct **get_global_using_directives ()
  {
    return &m_global_using_directives;
  }

  bool outermost_context_p () const
  {
    return m_context_stack.empty ();
  }

  /* Return true if the lexical context currently being constructed
     has a symbol, false otherwise.  */
  bool current_context_has_function () const
  {
    return (!m_context_stack.empty ()
	    && m_context_stack.back ().name != nullptr);
  }

  /* Set the symbol on the lexical context currently being
     constructed.  */
  void set_current_context_function (symbol *fun)
  {
    gdb_assert (!m_context_stack.empty ());
    gdb_assert (m_context_stack.back ().name == nullptr);
    gdb_assert (fun != nullptr);
    m_context_stack.back ().name = fun;
  }

  struct subfile *get_current_subfile ()
  {
    return m_current_subfile;
  }

  std::vector<symbol *> &get_local_symbols ()
  {
    return m_local_symbols;
  }

  std::vector<symbol *> &get_file_symbols ()
  {
    return m_file_symbols;
  }

  std::vector<symbol *> &get_global_symbols ()
  {
    return m_global_symbols;
  }

  void record_debugformat (const char *format)
  {
    m_debugformat = format;
  }

  void record_producer (const char *producer)
  {
    m_producer = producer;
  }

  void push_context (CORE_ADDR value);

  /* Pop a context and create the corresponding block.  Returns the
     block.  END_ADDR is the final address of the block.  STATIC_LINK,
     if provided, is the static link.  REQUIRED controls whether the
     block is required.  When false, if the block does not contain any
     variables or 'using' directives, this method will return
     nullptr.  */
  block *pop_context (CORE_ADDR end_addr,
		      const struct dynamic_prop *static_link = nullptr,
		      bool required = true);

  struct block *end_compunit_symtab_get_static_block
    (CORE_ADDR end_addr, bool expandable, bool required);

  struct compunit_symtab *end_compunit_symtab_from_static_block
    (struct block *static_block, bool expandable);

  struct compunit_symtab *end_compunit_symtab (CORE_ADDR end_addr);

  struct compunit_symtab *end_expandable_symtab (CORE_ADDR end_addr);

  void augment_type_symtab ();

private:

  void record_pending_block (struct block *block, struct pending_block *opblock);

  struct block *finish_block_internal (struct symbol *symbol,
				       std::vector<struct symbol *> &symbol_list,
				       struct pending_block *old_blocks,
				       const struct dynamic_prop *static_link,
				       CORE_ADDR start, CORE_ADDR end,
				       bool is_global, bool expandable);

  blockvector_up make_blockvector ();

  void watch_main_source_file_lossage ();

  /* The objfile we're reading debug info from.  */
  struct objfile *m_objfile;

  /* List of subfiles (source files).
     Files are added to the front of the list.
     This is important mostly for the language determination hacks we use,
     which iterate over previously added files.  */
  struct subfile *m_subfiles = nullptr;

  /* The subfile of the main source file.  */
  struct subfile *m_main_subfile = nullptr;

  /* E.g., DW_AT_comp_dir if DWARF.  Space for this is malloc'd.  */
  std::string m_comp_dir;

  /* Space for this is not malloc'd, and is assumed to have at least
     the same lifetime as objfile.  */
  const char *m_producer = nullptr;

  /* Space for this is not malloc'd, and is assumed to have at least
     the same lifetime as objfile.  */
  const char *m_debugformat = nullptr;

  /* The compunit we are building.  If the symtab is owned by this
     object, both fields are set.  For a re-opened symtab, only
     m_compunit_symtab is set.  */
  std::unique_ptr<compunit_symtab> m_owned_compunit_symtab;
  compunit_symtab *m_compunit_symtab;

  /* Language of this compunit_symtab.  */
  enum language m_language;

  /* The macro table for the compilation unit whose symbols we're
     currently reading.  */
  struct macro_table *m_pending_macros = nullptr;

  /* True if symtab has line number info.  This prevents an otherwise
     empty symtab from being tossed.  */
  bool m_have_line_numbers = false;

  /* Core address of start of text of current source file.  For DWARF it
     typically comes from the DW_AT_low_pc attribute of a
     DW_TAG_compile_unit DIE.  */
  CORE_ADDR m_last_source_start_addr;

  /* Stack of subfile names.  */
  std::vector<const char *> m_subfile_stack;

  /* The "using" directives local to lexical context.  */
  struct using_direct *m_local_using_directives = nullptr;

  /* Global "using" directives.  */
  struct using_direct *m_global_using_directives = nullptr;

  /* Unclosed lexical contexts (that will become blocks,
     eventually).  */
  struct lexical_context
  {
    lexical_context (std::vector<symbol *> locals,
		     using_direct *local_using_directives,
		     pending_block *old_blocks, CORE_ADDR start_addr)
      : locals (std::move (locals)),
	local_using_directives (local_using_directives),
	old_blocks (old_blocks),
	start_addr (start_addr)
    {}

    /* Outer locals at the time we entered.  */
    std::vector<symbol *> locals;

    /* Pending using directives at the time we entered.  */
    using_direct *local_using_directives;

    /* Pointer into blocklist as of entry.  */
    pending_block *old_blocks;

    /* Name of function, if any, defining context.  */
    symbol *name = nullptr;

    /* PC where this context starts.  */
    CORE_ADDR start_addr;
  };

  /* The stack of contexts that are pushed by push_context and popped
     by pop_context.  */
  std::vector<lexical_context> m_context_stack;

  struct subfile *m_current_subfile = nullptr;

  /* An obstack used for allocating pending blocks.  */
  auto_obstack m_pending_block_obstack;

  /* Pointer to the head of a linked list of symbol blocks which have
     already been finalized (lexical contexts already closed) and which
     are just waiting to be built into a blockvector when finalizing the
     associated symtab.  */
  struct pending_block *m_pending_blocks = nullptr;

  /* Pending static symbols and types at the top level.  */
  std::vector<symbol *> m_file_symbols;

  /* Pending global functions and variables.  */
  std::vector<symbol *> m_global_symbols;

  /* Pending symbols that are local to the lexical context.  */
  std::vector<symbol *> m_local_symbols;
};

using buildsym_compunit_up = std::unique_ptr<buildsym_compunit>;

static inline
void add_symbol_to_list (symbol *symbol, std::vector<struct symbol *> &list)
{
  list.push_back (symbol);
}

#endif /* GDB_BUILDSYM_H */

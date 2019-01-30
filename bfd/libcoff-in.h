/* BFD COFF object file private structure.
   Copyright (C) 1990-2022 Free Software Foundation, Inc.
   Written by Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _LIBCOFF_H
#define _LIBCOFF_H 1

#include "bfdlink.h"
#include "coff-bfd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Object file tdata; access macros.  */

#define coff_data(bfd)		      ((bfd)->tdata.coff_obj_data)
#define obj_pe(bfd)		      (coff_data (bfd)->pe)
#define obj_go32(bfd)		      (coff_data (bfd)->go32)
#define obj_symbols(bfd)	      (coff_data (bfd)->symbols)
#define obj_sym_filepos(bfd)	      (coff_data (bfd)->sym_filepos)
#define obj_relocbase(bfd)	      (coff_data (bfd)->relocbase)
#define obj_raw_syments(bfd)	      (coff_data (bfd)->raw_syments)
#define obj_raw_syment_count(bfd)     (coff_data (bfd)->raw_syment_count)
#define obj_convert(bfd)	      (coff_data (bfd)->conversion_table)
#define obj_conv_table_size(bfd)      (coff_data (bfd)->conv_table_size)
#define obj_coff_external_syms(bfd)   (coff_data (bfd)->external_syms)
#define obj_coff_keep_syms(bfd)	      (coff_data (bfd)->keep_syms)
#define obj_coff_strings(bfd)	      (coff_data (bfd)->strings)
#define obj_coff_strings_len(bfd)     (coff_data (bfd)->strings_len)
#define obj_coff_keep_strings(bfd)    (coff_data (bfd)->keep_strings)
#define obj_coff_sym_hashes(bfd)      (coff_data (bfd)->sym_hashes)
#define obj_coff_strings_written(bfd) (coff_data (bfd)->strings_written)
#define obj_coff_local_toc_table(bfd) (coff_data (bfd)->local_toc_sym_map)

/* `Tdata' information kept for COFF files.  */

typedef struct coff_tdata
{
  struct coff_symbol_struct *symbols;	/* Symtab for input bfd.  */
  unsigned int *conversion_table;
  int conv_table_size;
  file_ptr sym_filepos;

  struct coff_ptr_struct *raw_syments;
  unsigned long raw_syment_count;

  /* These are only valid once writing has begun.  */
  unsigned long int relocbase;

  /* These members communicate important constants about the symbol table
     to GDB's symbol-reading code.  These `constants' unfortunately vary
     from coff implementation to implementation...  */
  unsigned local_n_btmask;
  unsigned local_n_btshft;
  unsigned local_n_tmask;
  unsigned local_n_tshift;
  unsigned local_symesz;
  unsigned local_auxesz;
  unsigned local_linesz;

  /* The unswapped external symbols.  May be NULL.  Read by
     _bfd_coff_get_external_symbols.  */
  void * external_syms;
  /* If this is TRUE, the external_syms may not be freed.  */
  bool keep_syms;

  /* The string table.  May be NULL.  Read by
     _bfd_coff_read_string_table.  */
  char *strings;
  /* The length of the strings table.  For error checking.  */
  bfd_size_type strings_len;
  /* If this is TRUE, the strings may not be freed.  */
  bool keep_strings;
  /* If this is TRUE, the strings have been written out already.  */
  bool strings_written;

  /* Is this a PE format coff file?  */
  int pe;
  /* Used by the COFF backend linker.  */
  struct coff_link_hash_entry **sym_hashes;

  /* Used by the pe linker for PowerPC.  */
  int *local_toc_sym_map;

  struct bfd_link_info *link_info;

  /* Used by coff_find_nearest_line.  */
  void * line_info;

  /* A place to stash dwarf2 info for this bfd.  */
  void * dwarf2_find_line_info;

  /* The timestamp from the COFF file header.  */
  long timestamp;

  /* Copy of some of the f_flags bits in the COFF filehdr structure,
     used by ARM code.  */
  flagword flags;

  /* Is this a GO32 coff file?  */
  bool go32;

  /* A stub (extra data prepended before the COFF image) and its size.
     Used by coff-go32-exe, it contains executable data that loads the
     COFF object into memory.  */
  char * stub;
  bfd_size_type stub_size;
} coff_data_type;

/* Tdata for pe image files.  */
typedef struct pe_tdata
{
  coff_data_type coff;
  struct internal_extra_pe_aouthdr pe_opthdr;
  int dll;
  int has_reloc_section;
  int dont_strip_reloc;
  int dos_message[16];
  /* The timestamp to insert into the output file.
     If the timestamp is -1 then the current time is used.  */
  int timestamp;
  bool (*in_reloc_p) (bfd *, reloc_howto_type *);
  flagword real_flags;

  /* Build-id info.  */
  struct
  {
    bool (*after_write_object_contents) (bfd *);
    const char *style;
    asection *sec;
  } build_id;
} pe_data_type;

#define pe_data(bfd)		((bfd)->tdata.pe_obj_data)

/* Tdata for XCOFF files.  */

struct xcoff_tdata
{
  /* Basic COFF information.  */
  coff_data_type coff;

  /* TRUE if this is an XCOFF64 file. */
  bool xcoff64;

  /* TRUE if a large a.out header should be generated.  */
  bool full_aouthdr;

  /* TOC value.  */
  bfd_vma toc;

  /* Index of section holding TOC.  */
  int sntoc;

  /* Index of section holding entry point.  */
  int snentry;

  /* .text alignment from optional header.  */
  int text_align_power;

  /* .data alignment from optional header.  */
  int data_align_power;

  /* modtype from optional header.  */
  short modtype;

  /* cputype from optional header.  */
  short cputype;

  /* maxdata from optional header.  */
  bfd_vma maxdata;

  /* maxstack from optional header.  */
  bfd_vma maxstack;

  /* Used by the XCOFF backend linker.  */
  asection **csects;
  long *debug_indices;
  unsigned int *lineno_counts;
  unsigned int import_file_id;
};

#define xcoff_data(abfd) ((abfd)->tdata.xcoff_obj_data)

/* We take the address of the first element of an asymbol to ensure that the
   macro is only ever applied to an asymbol.  */
#define coffsymbol(asymbol) ((coff_symbol_type *)(&((asymbol)->the_bfd)))

/* Tdata for sections in XCOFF files.  This is used by the linker.  */

struct xcoff_section_tdata
{
  /* Used for XCOFF csects created by the linker; points to the real
     XCOFF section which contains this csect.  */
  asection *enclosing;
  /* The lineno_count field for the enclosing section, because we are
     going to clobber it there.  */
  unsigned int lineno_count;
  /* The first and last symbol indices for symbols used by this csect.  */
  unsigned long first_symndx;
  unsigned long last_symndx;
};

/* An accessor macro the xcoff_section_tdata structure.  */
#define xcoff_section_data(abfd, sec) \
  ((struct xcoff_section_tdata *) coff_section_data ((abfd), (sec))->tdata)

/* Tdata for sections in PE files.  */

struct pei_section_tdata
{
  /* The virtual size of the section.  */
  bfd_size_type virt_size;
  /* The PE section flags.  */
  long pe_flags;
};

/* An accessor macro for the pei_section_tdata structure.  */
#define pei_section_data(abfd, sec) \
  ((struct pei_section_tdata *) coff_section_data ((abfd), (sec))->tdata)

/* COFF linker hash table entries.  */

struct coff_link_hash_entry
{
  struct bfd_link_hash_entry root;

  /* Symbol index in output file.  This is initialized to -1.  It is
     set to -2 if the symbol is used by a reloc.  It is set to -3 if
     this symbol is defined in a discarded section.  */
  long indx;

  /* Symbol type.  */
  unsigned short type;

  /* Symbol class.  */
  unsigned char symbol_class;

  /* Number of auxiliary entries.  */
  char numaux;

  /* BFD to take auxiliary entries from.  */
  bfd *auxbfd;

  /* Pointer to array of auxiliary entries, if any.  */
  union internal_auxent *aux;

  /* Flag word; legal values follow.  */
  unsigned short coff_link_hash_flags;
  /* Symbol is a PE section symbol.  */
#define COFF_LINK_HASH_PE_SECTION_SYMBOL (01)
};

/* COFF linker hash table.  */

struct coff_link_hash_table
{
  struct bfd_link_hash_table root;
  /* A pointer to information used to link stabs in sections.  */
  struct stab_info stab_info;
};

struct coff_reloc_cookie
{
  struct internal_reloc *	  rels;
  struct internal_reloc *	  rel;
  struct internal_reloc *	  relend;
  struct coff_symbol_struct *	  symbols;	/* Symtab for input bfd.  */
  bfd *				  abfd;
  struct coff_link_hash_entry **  sym_hashes;
};

/* Look up an entry in a COFF linker hash table.  */

#define coff_link_hash_lookup(table, string, create, copy, follow)	\
  ((struct coff_link_hash_entry *)					\
   bfd_link_hash_lookup (&(table)->root, (string), (create),		\
			 (copy), (follow)))

/* Traverse a COFF linker hash table.  */

#define coff_link_hash_traverse(table, func, info)			\
  (bfd_link_hash_traverse						\
   (&(table)->root,							\
    (bool (*) (struct bfd_link_hash_entry *, void *)) (func),		\
    (info)))

/* Get the COFF linker hash table from a link_info structure.  */

#define coff_hash_table(p) ((struct coff_link_hash_table *) ((p)->hash))

/* Functions in coffgen.c.  */
extern bfd_cleanup coff_object_p
  (bfd *);
extern struct bfd_section *coff_section_from_bfd_index
  (bfd *, int);
extern long coff_get_symtab_upper_bound
  (bfd *);
extern long coff_canonicalize_symtab
  (bfd *, asymbol **);
extern int coff_count_linenumbers
  (bfd *);
extern bool coff_renumber_symbols
  (bfd *, int *);
extern void coff_mangle_symbols
  (bfd *);
extern bool coff_write_symbols
  (bfd *);
extern bool coff_write_alien_symbol
  (bfd *, asymbol *, struct internal_syment *, bfd_vma *,
   struct bfd_strtab_hash *, bool, asection **, bfd_size_type *);
extern bool coff_write_linenumbers
  (bfd *);
extern alent *coff_get_lineno
  (bfd *, asymbol *);
extern asymbol *coff_section_symbol
  (bfd *, char *);
extern bool _bfd_coff_get_external_symbols
  (bfd *);
extern const char *_bfd_coff_read_string_table
  (bfd *);
extern bool _bfd_coff_free_symbols
  (bfd *);
extern struct coff_ptr_struct *coff_get_normalized_symtab
  (bfd *);
extern long coff_get_reloc_upper_bound
  (bfd *, sec_ptr);
extern asymbol *coff_make_empty_symbol
  (bfd *);
extern void coff_print_symbol
  (bfd *, void * filep, asymbol *, bfd_print_symbol_type);
extern void coff_get_symbol_info
  (bfd *, asymbol *, symbol_info *ret);
#define coff_get_symbol_version_string \
  _bfd_nosymbols_get_symbol_version_string
extern bool _bfd_coff_is_local_label_name
  (bfd *, const char *);
extern asymbol *coff_bfd_make_debug_symbol
  (bfd *, void *, unsigned long);
extern bool coff_find_nearest_line
  (bfd *, asymbol **, asection *, bfd_vma,
   const char **, const char **, unsigned int *, unsigned int *);
#define coff_find_nearest_line_with_alt \
  _bfd_nosymbols_find_nearest_line_with_alt
#define coff_find_line _bfd_nosymbols_find_line
struct dwarf_debug_section;
extern bool coff_find_nearest_line_with_names
  (bfd *, asymbol **, asection *, bfd_vma, const char **, const char **,
   unsigned int *, const struct dwarf_debug_section *);
extern bool coff_find_inliner_info
  (bfd *, const char **, const char **, unsigned int *);
extern int coff_sizeof_headers
  (bfd *, struct bfd_link_info *);
extern bool bfd_coff_reloc16_relax_section
  (bfd *, asection *, struct bfd_link_info *, bool *);
extern bfd_byte *bfd_coff_reloc16_get_relocated_section_contents
  (bfd *, struct bfd_link_info *, struct bfd_link_order *,
   bfd_byte *, bool, asymbol **);
extern bfd_vma bfd_coff_reloc16_get_value
  (arelent *, struct bfd_link_info *, asection *);
extern void bfd_perform_slip
  (bfd *, unsigned int, asection *, bfd_vma);
extern bool _bfd_coff_close_and_cleanup
  (bfd *);

/* Functions and types in cofflink.c.  */

#define STRING_SIZE_SIZE 4

/* We use a hash table to merge identical enum, struct, and union
   definitions in the linker.  */

/* Information we keep for a single element (an enum value, a
   structure or union field) in the debug merge hash table.  */

struct coff_debug_merge_element
{
  /* Next element.  */
  struct coff_debug_merge_element *next;

  /* Name.  */
  const char *name;

  /* Type.  */
  unsigned int type;

  /* Symbol index for complex type.  */
  long tagndx;
};

/* A linked list of debug merge entries for a given name.  */

struct coff_debug_merge_type
{
  /* Next type with the same name.  */
  struct coff_debug_merge_type *next;

  /* Class of type.  */
  int type_class;

  /* Symbol index where this type is defined.  */
  long indx;

  /* List of elements.  */
  struct coff_debug_merge_element *elements;
};

/* Information we store in the debug merge hash table.  */

struct coff_debug_merge_hash_entry
{
  struct bfd_hash_entry root;

  /* A list of types with this name.  */
  struct coff_debug_merge_type *types;
};

/* The debug merge hash table.  */

struct coff_debug_merge_hash_table
{
  struct bfd_hash_table root;
};

/* Initialize a COFF debug merge hash table.  */

#define coff_debug_merge_hash_table_init(table) \
  (bfd_hash_table_init (&(table)->root, _bfd_coff_debug_merge_hash_newfunc, \
			sizeof (struct coff_debug_merge_hash_entry)))

/* Free a COFF debug merge hash table.  */

#define coff_debug_merge_hash_table_free(table) \
  (bfd_hash_table_free (&(table)->root))

/* Look up an entry in a COFF debug merge hash table.  */

#define coff_debug_merge_hash_lookup(table, string, create, copy) \
  ((struct coff_debug_merge_hash_entry *) \
   bfd_hash_lookup (&(table)->root, (string), (create), (copy)))

/* Information we keep for each section in the output file when doing
   a relocatable link.  */

struct coff_link_section_info
{
  /* The relocs to be output.  */
  struct internal_reloc *relocs;
  /* For each reloc against a global symbol whose index was not known
     when the reloc was handled, the global hash table entry.  */
  struct coff_link_hash_entry **rel_hashes;
};

/* Information that we pass around while doing the final link step.  */

struct coff_final_link_info
{
  /* General link information.  */
  struct bfd_link_info *info;
  /* Output BFD.  */
  bfd *output_bfd;
  /* Used to indicate failure in traversal routine.  */
  bool failed;
  /* If doing "task linking" set only during the time when we want the
     global symbol writer to convert the storage class of defined global
     symbols from global to static. */
  bool global_to_static;
  /* Hash table for long symbol names.  */
  struct bfd_strtab_hash *strtab;
  /* When doing a relocatable link, an array of information kept for
     each output section, indexed by the target_index field.  */
  struct coff_link_section_info *section_info;
  /* Symbol index of last C_FILE symbol (-1 if none).  */
  long last_file_index;
  /* Contents of last C_FILE symbol.  */
  struct internal_syment last_file;
  /* Symbol index of first aux entry of last .bf symbol with an empty
     endndx field (-1 if none).  */
  long last_bf_index;
  /* Contents of last_bf_index aux entry.  */
  union internal_auxent last_bf;
  /* Hash table used to merge debug information.  */
  struct coff_debug_merge_hash_table debug_merge;
  /* Buffer large enough to hold swapped symbols of any input file.  */
  struct internal_syment *internal_syms;
  /* Buffer large enough to hold sections of symbols of any input file.  */
  asection **sec_ptrs;
  /* Buffer large enough to hold output indices of symbols of any
     input file.  */
  long *sym_indices;
  /* Buffer large enough to hold output symbols for any input file.  */
  bfd_byte *outsyms;
  /* Buffer large enough to hold external line numbers for any input
     section.  */
  bfd_byte *linenos;
  /* Buffer large enough to hold any input section.  */
  bfd_byte *contents;
  /* Buffer large enough to hold external relocs of any input section.  */
  bfd_byte *external_relocs;
  /* Buffer large enough to hold swapped relocs of any input section.  */
  struct internal_reloc *internal_relocs;
};

/* Most COFF variants have no way to record the alignment of a
   section.  This struct is used to set a specific alignment based on
   the name of the section.  */

struct coff_section_alignment_entry
{
  /* The section name.  */
  const char *name;

  /* This is either (unsigned int) -1, indicating that the section
     name must match exactly, or it is the number of letters which
     must match at the start of the name.  */
  unsigned int comparison_length;

  /* These macros may be used to fill in the first two fields in a
     structure initialization.  */
#define COFF_SECTION_NAME_EXACT_MATCH(name) (name), ((unsigned int) -1)
#define COFF_SECTION_NAME_PARTIAL_MATCH(name) (name), (sizeof (name) - 1)

  /* Only use this entry if the default section alignment for this
     target is at least that much (as a power of two).  If this field
     is COFF_ALIGNMENT_FIELD_EMPTY, it should be ignored.  */
  unsigned int default_alignment_min;

  /* Only use this entry if the default section alignment for this
     target is no greater than this (as a power of two).  If this
     field is COFF_ALIGNMENT_FIELD_EMPTY, it should be ignored.  */
  unsigned int default_alignment_max;

#define COFF_ALIGNMENT_FIELD_EMPTY ((unsigned int) -1)

  /* The desired alignment for this section (as a power of two).  */
  unsigned int alignment_power;
};

extern struct bfd_hash_entry *_bfd_coff_link_hash_newfunc
  (struct bfd_hash_entry *, struct bfd_hash_table *, const char *);
extern bool _bfd_coff_link_hash_table_init
  (struct coff_link_hash_table *, bfd *,
   struct bfd_hash_entry *(*) (struct bfd_hash_entry *,
			       struct bfd_hash_table *,
			       const char *),
   unsigned int);
extern struct bfd_link_hash_table *_bfd_coff_link_hash_table_create
  (bfd *);
extern const char *_bfd_coff_internal_syment_name
  (bfd *, const struct internal_syment *, char *);
extern bool _bfd_coff_section_already_linked
  (bfd *, asection *, struct bfd_link_info *);
extern bool _bfd_coff_link_add_symbols
  (bfd *, struct bfd_link_info *);
extern bool _bfd_coff_final_link
  (bfd *, struct bfd_link_info *);
extern struct internal_reloc *_bfd_coff_read_internal_relocs
  (bfd *, asection *, bool, bfd_byte *, bool,
   struct internal_reloc *);
extern bool _bfd_coff_generic_relocate_section
  (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
   struct internal_reloc *, struct internal_syment *, asection **);
extern struct bfd_hash_entry *_bfd_coff_debug_merge_hash_newfunc
  (struct bfd_hash_entry *, struct bfd_hash_table *, const char *);
extern bool _bfd_coff_write_global_sym
  (struct bfd_hash_entry *, void *);
extern bool _bfd_coff_write_task_globals
  (struct coff_link_hash_entry *, void *);
extern bool _bfd_coff_link_input_bfd
  (struct coff_final_link_info *, bfd *);
extern bool _bfd_coff_reloc_link_order
  (bfd *, struct coff_final_link_info *, asection *,
   struct bfd_link_order *);
extern bool bfd_coff_gc_sections
  (bfd *, struct bfd_link_info *);
extern const char *bfd_coff_group_name
  (bfd *, const asection *);

extern bfd_cleanup coff_core_file_p
  (bfd *);


#define coff_get_section_contents_in_window \
  _bfd_generic_get_section_contents_in_window

/* Functions in xcofflink.c.  */

extern long _bfd_xcoff_get_dynamic_symtab_upper_bound
  (bfd *);
extern long _bfd_xcoff_canonicalize_dynamic_symtab
  (bfd *, asymbol **);
extern long _bfd_xcoff_get_dynamic_reloc_upper_bound
  (bfd *);
extern long _bfd_xcoff_canonicalize_dynamic_reloc
  (bfd *, arelent **, asymbol **);
extern struct bfd_link_hash_table *_bfd_xcoff_bfd_link_hash_table_create
  (bfd *);
extern bool _bfd_xcoff_bfd_link_add_symbols
  (bfd *, struct bfd_link_info *);
extern bool _bfd_xcoff_bfd_final_link
  (bfd *, struct bfd_link_info *);
extern bool _bfd_xcoff_define_common_symbol
  (bfd *, struct bfd_link_info *, struct bfd_link_hash_entry *);
extern bool _bfd_ppc_xcoff_relocate_section
  (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
   struct internal_reloc *, struct internal_syment *, asection **);

/* Copyright (C) 2008-2019 Free Software Foundation, Inc.

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
#include "windows-tdep.h"
#include "gdb_obstack.h"
#include "xml-support.h"
#include "gdbarch.h"
#include "target.h"
#include "value.h"
#include "inferior.h"
#include "command.h"
#include "gdbcmd.h"
#include "gdbthread.h"
#include "objfiles.h"
#include "symfile.h"
#include "coff-pe-read.h"
#include "gdb_bfd.h"
#include "complaints.h"
#include "solib.h"
#include "solib-target.h"
#include "gdbcore.h"
#include "charset.h"
#include "coff/internal.h"
#include "libcoff.h"

struct cmd_list_element *info_w32_cmdlist;

typedef struct thread_information_block_32
  {
    uint32_t current_seh;			/* %fs:0x0000 */
    uint32_t current_top_of_stack; 		/* %fs:0x0004 */
    uint32_t current_bottom_of_stack;		/* %fs:0x0008 */
    uint32_t sub_system_tib;			/* %fs:0x000c */
    uint32_t fiber_data;			/* %fs:0x0010 */
    uint32_t arbitrary_data_slot;		/* %fs:0x0014 */
    uint32_t linear_address_tib;		/* %fs:0x0018 */
    uint32_t environment_pointer;		/* %fs:0x001c */
    uint32_t process_id;			/* %fs:0x0020 */
    uint32_t current_thread_id;			/* %fs:0x0024 */
    uint32_t active_rpc_handle;			/* %fs:0x0028 */
    uint32_t thread_local_storage;		/* %fs:0x002c */
    uint32_t process_environment_block;		/* %fs:0x0030 */
    uint32_t last_error_number;			/* %fs:0x0034 */
  }
thread_information_32;

typedef struct thread_information_block_64
  {
    uint64_t current_seh;			/* %gs:0x0000 */
    uint64_t current_top_of_stack; 		/* %gs:0x0008 */
    uint64_t current_bottom_of_stack;		/* %gs:0x0010 */
    uint64_t sub_system_tib;			/* %gs:0x0018 */
    uint64_t fiber_data;			/* %gs:0x0020 */
    uint64_t arbitrary_data_slot;		/* %gs:0x0028 */
    uint64_t linear_address_tib;		/* %gs:0x0030 */
    uint64_t environment_pointer;		/* %gs:0x0038 */
    uint64_t process_id;			/* %gs:0x0040 */
    uint64_t current_thread_id;			/* %gs:0x0048 */
    uint64_t active_rpc_handle;			/* %gs:0x0050 */
    uint64_t thread_local_storage;		/* %gs:0x0058 */
    uint64_t process_environment_block;		/* %gs:0x0060 */
    uint64_t last_error_number;			/* %gs:0x0068 */
  }
thread_information_64;


static const char* TIB_NAME[] =
  {
    " current_seh                 ",	/* %fs:0x0000 */
    " current_top_of_stack        ", 	/* %fs:0x0004 */
    " current_bottom_of_stack     ",	/* %fs:0x0008 */
    " sub_system_tib              ",	/* %fs:0x000c */
    " fiber_data                  ",	/* %fs:0x0010 */
    " arbitrary_data_slot         ",	/* %fs:0x0014 */
    " linear_address_tib          ",	/* %fs:0x0018 */
    " environment_pointer         ",	/* %fs:0x001c */
    " process_id                  ",	/* %fs:0x0020 */
    " current_thread_id           ",	/* %fs:0x0024 */
    " active_rpc_handle           ",	/* %fs:0x0028 */
    " thread_local_storage        ",	/* %fs:0x002c */
    " process_environment_block   ",	/* %fs:0x0030 */
    " last_error_number           "	/* %fs:0x0034 */
  };

static const int MAX_TIB32 =
  sizeof (thread_information_32) / sizeof (uint32_t);
static const int MAX_TIB64 =
  sizeof (thread_information_64) / sizeof (uint64_t);
static const int FULL_TIB_SIZE = 0x1000;

static int maint_display_all_tib = 0;

static struct gdbarch_data *windows_gdbarch_data_handle;

struct windows_gdbarch_data
  {
    struct type *siginfo_type;
  };

static void *
init_windows_gdbarch_data (struct gdbarch *gdbarch)
{
  return GDBARCH_OBSTACK_ZALLOC (gdbarch, struct windows_gdbarch_data);
}

static struct windows_gdbarch_data *
get_windows_gdbarch_data (struct gdbarch *gdbarch)
{
  return ((struct windows_gdbarch_data *)
	  gdbarch_data (gdbarch, windows_gdbarch_data_handle));
}

/* Define Thread Local Base pointer type.  */

static struct type *
windows_get_tlb_type (struct gdbarch *gdbarch)
{
  static struct gdbarch *last_gdbarch = NULL;
  static struct type *last_tlb_type = NULL;
  struct type *dword_ptr_type, *dword32_type, *void_ptr_type;
  struct type *peb_ldr_type, *peb_ldr_ptr_type;
  struct type *peb_type, *peb_ptr_type, *list_type;
  struct type *module_list_ptr_type;
  struct type *tib_type, *seh_type, *tib_ptr_type, *seh_ptr_type;
  struct type *word_type, *wchar_type, *wchar_ptr_type;
  struct type *uni_str_type, *rupp_type, *rupp_ptr_type;

  /* Do not rebuild type if same gdbarch as last time.  */
  if (last_tlb_type && last_gdbarch == gdbarch)
    return last_tlb_type;
  
  dword_ptr_type = arch_integer_type (gdbarch, gdbarch_ptr_bit (gdbarch),
				 1, "DWORD_PTR");
  dword32_type = arch_integer_type (gdbarch, 32,
				 1, "DWORD32");
  word_type = arch_integer_type (gdbarch, 16,
				 1, "WORD");
  wchar_type = arch_integer_type (gdbarch, 16,
				  1, "wchar_t");
  void_ptr_type = lookup_pointer_type (builtin_type (gdbarch)->builtin_void);
  wchar_ptr_type = arch_type (gdbarch, TYPE_CODE_PTR,
			      TYPE_LENGTH (void_ptr_type) * TARGET_CHAR_BIT,
			      NULL);
  TYPE_TARGET_TYPE (wchar_ptr_type) = wchar_type;

  /* list entry */

  list_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_STRUCT);
  TYPE_NAME (list_type) = xstrdup ("list");

  module_list_ptr_type = void_ptr_type;

  append_composite_type_field (list_type, "forward_list",
			       module_list_ptr_type);
  append_composite_type_field (list_type, "backward_list",
			       module_list_ptr_type);

  /* Structured Exception Handler */

  seh_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_STRUCT);
  TYPE_NAME (seh_type) = xstrdup ("seh");

  seh_ptr_type = arch_type (gdbarch, TYPE_CODE_PTR,
			    TYPE_LENGTH (void_ptr_type) * TARGET_CHAR_BIT,
			    NULL);
  TYPE_TARGET_TYPE (seh_ptr_type) = seh_type;

  append_composite_type_field (seh_type, "next_seh", seh_ptr_type);
  append_composite_type_field (seh_type, "handler",
			       builtin_type (gdbarch)->builtin_func_ptr);

  /* struct _PEB_LDR_DATA */
  peb_ldr_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_STRUCT);
  TYPE_NAME (peb_ldr_type) = xstrdup ("peb_ldr_data");

  append_composite_type_field (peb_ldr_type, "length", dword32_type);
  append_composite_type_field (peb_ldr_type, "initialized", dword32_type);
  append_composite_type_field (peb_ldr_type, "ss_handle", void_ptr_type);
  append_composite_type_field (peb_ldr_type, "in_load_order", list_type);
  append_composite_type_field (peb_ldr_type, "in_memory_order", list_type);
  append_composite_type_field (peb_ldr_type, "in_init_order", list_type);
  append_composite_type_field (peb_ldr_type, "entry_in_progress",
			       void_ptr_type);
  peb_ldr_ptr_type = arch_type (gdbarch, TYPE_CODE_PTR,
				TYPE_LENGTH (void_ptr_type) * TARGET_CHAR_BIT,
				NULL);
  TYPE_TARGET_TYPE (peb_ldr_ptr_type) = peb_ldr_type;

  /* struct UNICODE_STRING */
  uni_str_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_STRUCT);
  TYPE_NAME (uni_str_type) = xstrdup ("unicode_string");

  append_composite_type_field (uni_str_type, "length", word_type);
  append_composite_type_field (uni_str_type, "maximum_length", word_type);
  append_composite_type_field_aligned (uni_str_type, "buffer",
				       wchar_ptr_type,
				       TYPE_LENGTH (wchar_ptr_type));

  /* struct _RTL_USER_PROCESS_PARAMETERS */
  rupp_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_STRUCT);
  TYPE_NAME (rupp_type) = xstrdup ("rtl_user_process_parameters");

  append_composite_type_field (rupp_type, "maximum_length", dword32_type);
  append_composite_type_field (rupp_type, "length", dword32_type);
  append_composite_type_field (rupp_type, "flags", dword32_type);
  append_composite_type_field (rupp_type, "debug_flags", dword32_type);
  append_composite_type_field (rupp_type, "console_handle", void_ptr_type);
  append_composite_type_field (rupp_type, "console_flags", dword32_type);
  append_composite_type_field_aligned (rupp_type, "standard_input",
				       void_ptr_type,
				       TYPE_LENGTH (void_ptr_type));
  append_composite_type_field (rupp_type, "standard_output", void_ptr_type);
  append_composite_type_field (rupp_type, "standard_error", void_ptr_type);
  append_composite_type_field (rupp_type, "current_directory", uni_str_type);
  append_composite_type_field (rupp_type, "current_directory_handle",
			       void_ptr_type);
  append_composite_type_field (rupp_type, "dll_path", uni_str_type);
  append_composite_type_field (rupp_type, "image_path_name", uni_str_type);
  append_composite_type_field (rupp_type, "command_line", uni_str_type);
  append_composite_type_field (rupp_type, "environment", void_ptr_type);
  append_composite_type_field (rupp_type, "starting_x", dword32_type);
  append_composite_type_field (rupp_type, "starting_y", dword32_type);
  append_composite_type_field (rupp_type, "count_x", dword32_type);
  append_composite_type_field (rupp_type, "count_y", dword32_type);
  append_composite_type_field (rupp_type, "count_chars_x", dword32_type);
  append_composite_type_field (rupp_type, "count_chars_y", dword32_type);
  append_composite_type_field (rupp_type, "fill_attribute", dword32_type);
  append_composite_type_field (rupp_type, "window_flags", dword32_type);
  append_composite_type_field (rupp_type, "show_window_flags", dword32_type);
  append_composite_type_field_aligned (rupp_type, "window_title",
				       uni_str_type,
				       TYPE_LENGTH (void_ptr_type));
  append_composite_type_field (rupp_type, "desktop_info", uni_str_type);
  append_composite_type_field (rupp_type, "shell_info", uni_str_type);
  append_composite_type_field (rupp_type, "runtime_data", uni_str_type);

  rupp_ptr_type = arch_type (gdbarch, TYPE_CODE_PTR,
			     TYPE_LENGTH (void_ptr_type) * TARGET_CHAR_BIT,
			     NULL);
  TYPE_TARGET_TYPE (rupp_ptr_type) = rupp_type;


  /* struct process environment block */
  peb_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_STRUCT);
  TYPE_NAME (peb_type) = xstrdup ("peb");

  /* First bytes contain several flags.  */
  append_composite_type_field (peb_type, "flags", dword_ptr_type);
  append_composite_type_field (peb_type, "mutant", void_ptr_type);
  append_composite_type_field (peb_type, "image_base_address", void_ptr_type);
  append_composite_type_field (peb_type, "ldr", peb_ldr_ptr_type);
  append_composite_type_field (peb_type, "process_parameters", rupp_ptr_type);
  append_composite_type_field (peb_type, "sub_system_data", void_ptr_type);
  append_composite_type_field (peb_type, "process_heap", void_ptr_type);
  append_composite_type_field (peb_type, "fast_peb_lock", void_ptr_type);
  peb_ptr_type = arch_type (gdbarch, TYPE_CODE_PTR,
			    TYPE_LENGTH (void_ptr_type) * TARGET_CHAR_BIT,
			    NULL);
  TYPE_TARGET_TYPE (peb_ptr_type) = peb_type;


  /* struct thread information block */
  tib_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_STRUCT);
  TYPE_NAME (tib_type) = xstrdup ("tib");

  /* uint32_t current_seh;			%fs:0x0000 */
  append_composite_type_field (tib_type, "current_seh", seh_ptr_type);
  /* uint32_t current_top_of_stack; 		%fs:0x0004 */
  append_composite_type_field (tib_type, "current_top_of_stack",
			       void_ptr_type);
  /* uint32_t current_bottom_of_stack;		%fs:0x0008 */
  append_composite_type_field (tib_type, "current_bottom_of_stack",
			       void_ptr_type);
  /* uint32_t sub_system_tib;			%fs:0x000c */
  append_composite_type_field (tib_type, "sub_system_tib", void_ptr_type);

  /* uint32_t fiber_data;			%fs:0x0010 */
  append_composite_type_field (tib_type, "fiber_data", void_ptr_type);
  /* uint32_t arbitrary_data_slot;		%fs:0x0014 */
  append_composite_type_field (tib_type, "arbitrary_data_slot", void_ptr_type);
  /* uint32_t linear_address_tib;		%fs:0x0018 */
  append_composite_type_field (tib_type, "linear_address_tib", void_ptr_type);
  /* uint32_t environment_pointer;		%fs:0x001c */
  append_composite_type_field (tib_type, "environment_pointer", void_ptr_type);
  /* uint32_t process_id;			%fs:0x0020 */
  append_composite_type_field (tib_type, "process_id", dword_ptr_type);
  /* uint32_t current_thread_id;		%fs:0x0024 */
  append_composite_type_field (tib_type, "thread_id", dword_ptr_type);
  /* uint32_t active_rpc_handle;		%fs:0x0028 */
  append_composite_type_field (tib_type, "active_rpc_handle", dword_ptr_type);
  /* uint32_t thread_local_storage;		%fs:0x002c */
  append_composite_type_field (tib_type, "thread_local_storage",
			       void_ptr_type);
  /* uint32_t process_environment_block;	%fs:0x0030 */
  append_composite_type_field (tib_type, "process_environment_block",
			       peb_ptr_type);
  /* uint32_t last_error_number;		%fs:0x0034 */
  append_composite_type_field (tib_type, "last_error_number", dword_ptr_type);

  tib_ptr_type = arch_type (gdbarch, TYPE_CODE_PTR,
			    TYPE_LENGTH (void_ptr_type) * TARGET_CHAR_BIT,
			    NULL);
  TYPE_TARGET_TYPE (tib_ptr_type) = tib_type;

  last_tlb_type = tib_ptr_type;
  last_gdbarch = gdbarch;

  return tib_ptr_type;
}

/* The $_tlb convenience variable is a bit special.  We don't know
   for sure the type of the value until we actually have a chance to
   fetch the data.  The type can change depending on gdbarch, so it is
   also dependent on which thread you have selected.  */

/* This function implements the lval_computed support for reading a
   $_tlb value.  */

static void
tlb_value_read (struct value *val)
{
  CORE_ADDR tlb;
  struct type *type = check_typedef (value_type (val));

  if (!target_get_tib_address (inferior_ptid, &tlb))
    error (_("Unable to read tlb"));
  store_typed_address (value_contents_raw (val), type, tlb);
}

/* This function implements the lval_computed support for writing a
   $_tlb value.  */

static void
tlb_value_write (struct value *v, struct value *fromval)
{
  error (_("Impossible to change the Thread Local Base"));
}

static const struct lval_funcs tlb_value_funcs =
  {
    tlb_value_read,
    tlb_value_write
  };


/* Return a new value with the correct type for the tlb object of
   the current thread using architecture GDBARCH.  Return a void value
   if there's no object available.  */

static struct value *
tlb_make_value (struct gdbarch *gdbarch, struct internalvar *var, void *ignore)
{
  if (target_has_stack && inferior_ptid != null_ptid)
    {
      struct type *type = windows_get_tlb_type (gdbarch);
      return allocate_computed_value (type, &tlb_value_funcs, NULL);
    }

  return allocate_value (builtin_type (gdbarch)->builtin_void);
}


/* Display thread information block of a given thread.  */

static int
display_one_tib (ptid_t ptid)
{
  gdb_byte *tib = NULL;
  gdb_byte *index;
  CORE_ADDR thread_local_base;
  ULONGEST i, val, max, max_name, size, tib_size;
  ULONGEST sizeof_ptr = gdbarch_ptr_bit (target_gdbarch ());
  enum bfd_endian byte_order = gdbarch_byte_order (target_gdbarch ());

  if (sizeof_ptr == 64)
    {
      size = sizeof (uint64_t);
      tib_size = sizeof (thread_information_64);
      max = MAX_TIB64;
    }
  else
    {
      size = sizeof (uint32_t);
      tib_size = sizeof (thread_information_32);
      max = MAX_TIB32;
    }

  max_name = max;

  if (maint_display_all_tib)
    {
      tib_size = FULL_TIB_SIZE;
      max = tib_size / size;
    }
  
  tib = (gdb_byte *) alloca (tib_size);

  if (target_get_tib_address (ptid, &thread_local_base) == 0)
    {
      printf_filtered (_("Unable to get thread local base for %s\n"),
	target_pid_to_str (ptid));
      return -1;
    }

  if (target_read (current_top_target (), TARGET_OBJECT_MEMORY,
		   NULL, tib, thread_local_base, tib_size) != tib_size)
    {
      printf_filtered (_("Unable to read thread information "
			 "block for %s at address %s\n"),
	target_pid_to_str (ptid), 
	paddress (target_gdbarch (), thread_local_base));
      return -1;
    }

  printf_filtered (_("Thread Information Block %s at %s\n"),
		   target_pid_to_str (ptid),
		   paddress (target_gdbarch (), thread_local_base));

  index = (gdb_byte *) tib;

  /* All fields have the size of a pointer, this allows to iterate 
     using the same for loop for both layouts.  */
  for (i = 0; i < max; i++)
    {
      val = extract_unsigned_integer (index, size, byte_order);
      if (i < max_name)
	printf_filtered (_("%s is 0x%s\n"), TIB_NAME[i], phex (val, size));
      else if (val != 0)
	printf_filtered (_("TIB[0x%s] is 0x%s\n"), phex (i * size, 2),
			 phex (val, size));
      index += size;
    } 
  return 1;  
}

/* Display thread information block of the current thread.  */

static void
display_tib (const char * args, int from_tty)
{
  if (inferior_ptid != null_ptid)
    display_one_tib (inferior_ptid);
}

void
windows_xfer_shared_library (const char* so_name, CORE_ADDR load_addr,
			     struct gdbarch *gdbarch, struct obstack *obstack)
{
  CORE_ADDR text_offset;

  obstack_grow_str (obstack, "<library name=\"");
  std::string p = xml_escape_text (so_name);
  obstack_grow_str (obstack, p.c_str ());
  obstack_grow_str (obstack, "\"><segment address=\"");
  gdb_bfd_ref_ptr dll (gdb_bfd_open (so_name, gnutarget, -1));
  /* The following calls are OK even if dll is NULL.
     The default value 0x1000 is returned by pe_text_section_offset
     in that case.  */
  text_offset = pe_text_section_offset (dll.get ());
  obstack_grow_str (obstack, paddress (gdbarch, load_addr + text_offset));
  obstack_grow_str (obstack, "\"/></library>");
}

/* Implement the "iterate_over_objfiles_in_search_order" gdbarch
   method.  It searches all objfiles, starting with CURRENT_OBJFILE
   first (if not NULL).

   On Windows, the system behaves a little differently when two
   objfiles each define a global symbol using the same name, compared
   to other platforms such as GNU/Linux for instance.  On GNU/Linux,
   all instances of the symbol effectively get merged into a single
   one, but on Windows, they remain distinct.

   As a result, it usually makes sense to start global symbol searches
   with the current objfile before expanding it to all other objfiles.
   This helps for instance when a user debugs some code in a DLL that
   refers to a global variable defined inside that DLL.  When trying
   to print the value of that global variable, it would be unhelpful
   to print the value of another global variable defined with the same
   name, but in a different DLL.  */

static void
windows_iterate_over_objfiles_in_search_order
  (struct gdbarch *gdbarch,
   iterate_over_objfiles_in_search_order_cb_ftype *cb,
   void *cb_data, struct objfile *current_objfile)
{
  int stop;

  if (current_objfile)
    {
      stop = cb (current_objfile, cb_data);
      if (stop)
	return;
    }

  for (objfile *objfile : current_program_space->objfiles ())
    {
      if (objfile != current_objfile)
	{
	  stop = cb (objfile, cb_data);
	  if (stop)
	    return;
	}
    }
}

static void
show_maint_show_all_tib (struct ui_file *file, int from_tty,
		struct cmd_list_element *c, const char *value)
{
  fprintf_filtered (file, _("Show all non-zero elements of "
			    "Thread Information Block is %s.\n"), value);
}

static void
info_w32_command (const char *args, int from_tty)
{
  help_list (info_w32_cmdlist, "info w32 ", class_info, gdb_stdout);
}

static int w32_prefix_command_valid = 0;
void
init_w32_command_list (void)
{
  if (!w32_prefix_command_valid)
    {
      add_prefix_cmd ("w32", class_info, info_w32_command,
		      _("Print information specific to Win32 debugging."),
		      &info_w32_cmdlist, "info w32 ", 0, &infolist);
      w32_prefix_command_valid = 1;
    }
}

struct enum_value_name
{
  uint32_t value;
  const char *name;
};

static struct type *
create_enum (struct gdbarch *gdbarch, int bit, const char *name,
	     const struct enum_value_name *values, int count)
{
  struct type *type;
  int i;

  type = arch_type (gdbarch, TYPE_CODE_ENUM, bit, name);
  TYPE_NFIELDS (type) = count;
  TYPE_FIELDS (type) = (struct field *)
    TYPE_ZALLOC (type, sizeof (struct field) * count);
  TYPE_UNSIGNED (type) = 1;

  for (i = 0; i < count; i++)
  {
    TYPE_FIELD_NAME (type, i) = values[i].name;
    SET_FIELD_ENUMVAL (TYPE_FIELD (type, i), values[i].value);
  }

  return type;
}

static const struct enum_value_name exception_values[] =
{
  { 0xC0000005, "ACCESS_VIOLATION" },
  { 0xC00000FD, "STACK_OVERFLOW" },
  { 0xC000008D, "FLOAT_DENORMAL_OPERAND" },
  { 0xC000008C, "ARRAY_BOUNDS_EXCEEDED" },
  { 0xC000008F, "FLOAT_INEXACT_RESULT" },
  { 0xC0000090, "FLOAT_INVALID_OPERATION" },
  { 0xC0000091, "FLOAT_OVERFLOW" },
  { 0xC0000092, "FLOAT_STACK_CHECK" },
  { 0xC0000093, "FLOAT_UNDERFLOW" },
  { 0xC000008E, "FLOAT_DIVIDE_BY_ZERO" },
  { 0xC0000094, "INTEGER_DIVIDE_BY_ZERO" },
  { 0xC0000095, "INTEGER_OVERFLOW" },
  { 0x80000003, "BREAKPOINT" },
  { 0x40010005, "DBG_CONTROL_C" },
  { 0x40010008, "DBG_CONTROL_BREAK" },
  { 0x80000004, "SINGLE_STEP" },
  { 0xC000001D, "ILLEGAL_INSTRUCTION" },
  { 0xC0000096, "PRIV_INSTRUCTION" },
  { 0xC0000025, "NONCONTINUABLE_EXCEPTION" },
  { 0x40000015, "FATAL_APP_EXIT" },
  { 0x80000002, "DATATYPE_MISALIGNMENT" },
  { 0xC0000006, "IN_PAGE_ERROR" },
  { 0xC0000026, "INVALID_DISPOSITION" },
};

static const struct enum_value_name violation_values[] =
{
  { 0, "READ_ACCESS_VIOLATION" },
  { 1, "WRITE_ACCESS_VIOLATION" },
  { 8, "DATA_EXECUTION_PREVENTION_VIOLATION" },
};

static struct type *
windows_get_siginfo_type (struct gdbarch *gdbarch)
{
  struct windows_gdbarch_data *windows_gdbarch_data;
  struct type *uint_type, *void_ptr_type, *code_enum, *violation_enum;
  struct type *violation_type, *para_type, *siginfo_ptr_type, *siginfo_type;

  windows_gdbarch_data = get_windows_gdbarch_data (gdbarch);
  if (windows_gdbarch_data->siginfo_type != NULL)
    return windows_gdbarch_data->siginfo_type;

  uint_type = arch_integer_type (gdbarch, gdbarch_int_bit (gdbarch),
				 1, "unsigned int");
  void_ptr_type = lookup_pointer_type (builtin_type (gdbarch)->builtin_void);

  code_enum = create_enum (gdbarch, gdbarch_int_bit (gdbarch),
			   "ExceptionCode", exception_values,
			   ARRAY_SIZE (exception_values));

  violation_enum = create_enum (gdbarch, gdbarch_ptr_bit (gdbarch),
				"ViolationType", violation_values,
				ARRAY_SIZE (violation_values));

  violation_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_STRUCT);
  append_composite_type_field (violation_type, "Type", violation_enum);
  append_composite_type_field (violation_type, "Address", void_ptr_type);

  para_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_UNION);
  append_composite_type_field (para_type, "ExceptionInformation",
			       lookup_array_range_type (void_ptr_type, 0, 14));
  append_composite_type_field (para_type, "AccessViolationInformation",
			       violation_type);

  siginfo_type = arch_composite_type (gdbarch, NULL, TYPE_CODE_STRUCT);
  TYPE_NAME (siginfo_type) = xstrdup ("EXCEPTION_RECORD");

  siginfo_ptr_type = arch_type (gdbarch, TYPE_CODE_PTR,
				gdbarch_ptr_bit (gdbarch), NULL);
  TYPE_TARGET_TYPE (siginfo_ptr_type) = siginfo_type;

  append_composite_type_field (siginfo_type, "ExceptionCode", code_enum);
  append_composite_type_field (siginfo_type, "ExceptionFlags", uint_type);
  append_composite_type_field (siginfo_type, "ExceptionRecord",
			       siginfo_ptr_type);
  append_composite_type_field (siginfo_type, "ExceptionAddress", void_ptr_type);
  append_composite_type_field (siginfo_type, "NumberParameters", uint_type);
  append_composite_type_field_aligned (siginfo_type, "",
				       para_type, TYPE_LENGTH (void_ptr_type));

  windows_gdbarch_data->siginfo_type = siginfo_type;

  return siginfo_type;
}

struct cpms_data
{
  struct gdbarch *gdbarch;
  struct obstack *obstack;
  int module_count;
};

static void
core_process_module_section (bfd *abfd, asection *sect, void *obj)
{
  struct cpms_data *data = (struct cpms_data *) obj;
  enum bfd_endian byte_order = gdbarch_byte_order (data->gdbarch);

  char *module_name;
  size_t module_name_size;
  CORE_ADDR base_addr;
  int is_module, is_coremodule;

  gdb_byte *buf = NULL;

  is_module = startswith (sect->name, ".module");
  is_coremodule = startswith (sect->name, ".coremodule/");
  if (!is_module && !is_coremodule)
    return;

  buf = (gdb_byte *) xmalloc (bfd_get_section_size (sect) + 1);
  if (!buf)
    {
      printf_unfiltered ("memory allocation failed for %s\n", sect->name);
      goto out;
    }
  if (!bfd_get_section_contents (abfd, sect,
				 buf, 0, bfd_get_section_size (sect)))
    goto out;


  if (is_coremodule)
    {
      auto_obstack host_name;
      convert_between_encodings (target_wide_charset (data->gdbarch),
				 host_charset (),
				 buf, bfd_get_section_size (sect), 2,
				 &host_name, translit_char);
      obstack_grow_str0 (&host_name, "");
      module_name = (char *) obstack_base (&host_name);

      sscanf (sect->name + 12, "%llx", &base_addr);

      if (data->module_count != 0)
	windows_xfer_shared_library (module_name, base_addr,
				     data->gdbarch, data->obstack);
      data->module_count++;

      xfree (buf);
      return;
    }


  /* A DWORD (data_type) followed by struct windows_core_module_info.  */

  base_addr =
    extract_unsigned_integer (buf + 4, 4, byte_order);

  module_name_size =
    extract_unsigned_integer (buf + 8, 4, byte_order);

  if (12 + module_name_size > bfd_get_section_size (sect))
    goto out;
  module_name = (char *) buf + 12;

  /* The first module is the .exe itself.  */
  if (data->module_count != 0)
    windows_xfer_shared_library (module_name, base_addr,
				 data->gdbarch, data->obstack);
  data->module_count++;

out:
  if (buf)
    xfree (buf);
  return;
}

static ULONGEST
windows_core_xfer_shared_libraries (struct gdbarch *gdbarch,
				  gdb_byte *readbuf,
				  ULONGEST offset, ULONGEST len)
{
  struct obstack obstack;
  const char *buf;
  ULONGEST len_avail;
  struct cpms_data data = { gdbarch, &obstack, 0 };

  obstack_init (&obstack);
  obstack_grow_str (&obstack, "<library-list>\n");
  bfd_map_over_sections (core_bfd,
			 core_process_module_section,
			 &data);
  obstack_grow_str0 (&obstack, "</library-list>\n");

  buf = (const char *) obstack_finish (&obstack);
  len_avail = strlen (buf);
  if (offset >= len_avail)
    return 0;

  if (len > len_avail - offset)
    len = len_avail - offset;
  memcpy (readbuf, buf + offset, len);

  obstack_free (&obstack, NULL);
  return len;
}

/* This is how we want PTIDs from core files to be printed.  */

static const char *
i386_windows_core_pid_to_str (struct gdbarch *gdbarch, ptid_t ptid)
{
  static char buf[80];

  if (ptid.lwp () != 0)
    {
      snprintf (buf, sizeof (buf), "Thread 0x%lx", ptid.lwp ());
      return buf;
    }

  return normal_pid_to_str (ptid);
}

static enum gdb_signal
windows_gdb_signal_from_target (struct gdbarch *gdbarch,
				int signal)
{
  unsigned int usignal = signal;
  switch (usignal)
    {
    case 0:
      return GDB_SIGNAL_0;

    case 0xC000005: /* EXCEPTION_ACCESS_VIOLATION */
    case 0xC0000FD: /* STATUS_STACK_OVERFLOW */
      return GDB_SIGNAL_SEGV;

    case 0xC00008C: /* EXCEPTION_ARRAY_BOUNDS_EXCEEDED */
    case 0xC00008D: /* STATUS_FLOAT_DENORMAL_OPERAND */
    case 0xC00008E: /* STATUS_FLOAT_DIVIDE_BY_ZERO */
    case 0xC00008F: /* STATUS_FLOAT_INEXACT_RESULT */
    case 0xC000090: /* STATUS_FLOAT_INVALID_OPERATION */
    case 0xC000091: /* STATUS_FLOAT_OVERFLOW */
    case 0xC000092: /* STATUS_FLOAT_STACK_CHECK */
    case 0xC000093: /* STATUS_FLOAT_UNDERFLOW */
    case 0xC000094: /* STATUS_INTEGER_DIVIDE_BY_ZERO */
    case 0xC000095: /* STATUS_INTEGER_OVERFLOW */
      return GDB_SIGNAL_FPE;

    case 0x8000003: /* EXCEPTION_BREAKPOINT */
    case 0x8000004: /* EXCEPTION_SINGLE_STEP */
      return GDB_SIGNAL_TRAP;

    case 0x4010005: /* DBG_CONTROL_C */
    case 0x4010008: /* DBG_CONTROL_BREAK */
      return GDB_SIGNAL_INT;

    case 0xC00001D: /* EXCEPTION_ILLEGAL_INSTRUCTION */
    case 0xC000096: /* EXCEPTION_PRIV_INSTRUCTION */
    case 0xC000025: /* EXCEPTION_NONCONTINUABLE_EXCEPTION */
      return GDB_SIGNAL_ILL;

    case 0x4000015: /* FATAL_APP_EXIT */
      return GDB_SIGNAL_ABRT;
    }
  return GDB_SIGNAL_UNKNOWN;
}

static LONGEST
windows_core_xfer_siginfo (struct gdbarch *gdbarch, gdb_byte *readbuf,
			   ULONGEST offset, ULONGEST len)
{
  asection *section = bfd_get_section_by_name (core_bfd, ".coreexception");
  if (section == NULL)
    return -1;

  /* The exception record of the minidump file is always in 64bit format.  */
  if (gdbarch_ptr_bit (gdbarch) == 32)
    {
      uint32_t rec[38];
      int r;

#define EXC_SIZE_32 80
#define EXC_SIZE_64 152

      if (offset > EXC_SIZE_32)
	return -1;

      if (bfd_get_section_size (section) != EXC_SIZE_64)
	return -1;

      if (!bfd_get_section_contents (core_bfd, section, rec, 0, EXC_SIZE_64))
	return -1;

      for (r = 2; r < 19; r++)
	rec[r + 1] = rec[r * 2];

      if (len > EXC_SIZE_32 - offset)
	len = EXC_SIZE_32 - offset;

      memcpy (readbuf, (char *) rec + offset, len);

      return len;
    }

  if (!bfd_get_section_contents (core_bfd, section, readbuf, offset, len))
    return -1;

  return len;
}

static const char *
windows_core_thread_name (struct gdbarch *gdbarch, struct thread_info *thr)
{
  if (thr->ptid.lwp () == 0)
    return NULL;

  static char name_buf[80];
  gdb_byte *buf;
  struct bfd_section *sect;
  thread_section_name section_name (".corethread", thr->ptid);

  sect = bfd_get_section_by_name (core_bfd, section_name.c_str ());
  if (sect == NULL || bfd_section_size (core_bfd, sect) ==0)
    return NULL;

  buf = (gdb_byte *) xmalloc (bfd_get_section_size (sect) + 1);
  if (!buf)
  {
    printf_unfiltered ("memory allocation failed for %s\n",
        sect->name);
    return NULL;
  }
  if (bfd_get_section_contents (core_bfd, sect,
        buf, 0, bfd_get_section_size (sect)))
  {
    char *thread_name;
    size_t thread_name_size;
    auto_obstack host_name;

    convert_between_encodings (target_wide_charset (gdbarch),
        host_charset (),
        buf, bfd_get_section_size (sect), 2,
        &host_name, translit_char);
    obstack_grow_str0 (&host_name, "");
    thread_name = (char *) obstack_base (&host_name);
    thread_name_size = strlen (thread_name);
    if (thread_name_size > sizeof name_buf - 1)
      thread_name_size = sizeof name_buf - 1;
    memcpy (name_buf, thread_name, thread_name_size);
    name_buf[thread_name_size] = 0;

    xfree (buf);

    return name_buf;
  }

  xfree (buf);

  return NULL;
}

/* To be called from the various GDB_OSABI_CYGWIN handlers for the
   various Windows architectures and machine types.  */

void
windows_init_abi (struct gdbarch_info info, struct gdbarch *gdbarch)
{
  set_gdbarch_wchar_bit (gdbarch, 16);
  set_gdbarch_wchar_signed (gdbarch, 0);

  /* Canonical paths on this target look like
     `c:\Program Files\Foo App\mydll.dll', for example.  */
  set_gdbarch_has_dos_based_file_system (gdbarch, 1);

  set_gdbarch_iterate_over_objfiles_in_search_order
    (gdbarch, windows_iterate_over_objfiles_in_search_order);

  set_solib_ops (gdbarch, &solib_target_so_ops);

  set_gdbarch_get_siginfo_type (gdbarch, windows_get_siginfo_type);

  /* Core file support.  */
  set_gdbarch_core_xfer_shared_libraries
    (gdbarch, windows_core_xfer_shared_libraries);
  set_gdbarch_core_pid_to_str (gdbarch, i386_windows_core_pid_to_str);
  set_gdbarch_gdb_signal_from_target (gdbarch,
				      windows_gdb_signal_from_target);
  set_gdbarch_core_xfer_siginfo (gdbarch, windows_core_xfer_siginfo);
  set_gdbarch_core_thread_name (gdbarch, windows_core_thread_name);
}

/* Implementation of `tlb' variable.  */

static const struct internalvar_funcs tlb_funcs =
{
  tlb_make_value,
  NULL,
  NULL
};

void
_initialize_windows_tdep (void)
{
  windows_gdbarch_data_handle =
    gdbarch_data_register_post_init (init_windows_gdbarch_data);

  init_w32_command_list ();
  add_cmd ("thread-information-block", class_info, display_tib,
	   _("Display thread information block."),
	   &info_w32_cmdlist);
  add_alias_cmd ("tib", "thread-information-block", class_info, 1,
		 &info_w32_cmdlist);

  add_setshow_boolean_cmd ("show-all-tib", class_maintenance,
			   &maint_display_all_tib, _("\
Set whether to display all non-zero fields of thread information block."), _("\
Show whether to display all non-zero fields of thread information block."), _("\
Use \"on\" to enable, \"off\" to disable.\n\
If enabled, all non-zero fields of thread information block are displayed,\n\
even if their meaning is unknown."),
			   NULL,
			   show_maint_show_all_tib,
			   &maintenance_set_cmdlist,
			   &maintenance_show_cmdlist);

  /* Explicitly create without lookup, since that tries to create a
     value with a void typed value, and when we get here, gdbarch
     isn't initialized yet.  At this point, we're quite sure there
     isn't another convenience variable of the same name.  */
  create_internalvar_type_lazy ("_tlb", &tlb_funcs, NULL);
}

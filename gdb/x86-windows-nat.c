/* Native-dependent code for Windows x86 (i386 and x86-64).

   Copyright (C) 2025 Free Software Foundation, Inc.

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

#include "windows-nat.h"
#include "regcache.h"
#include "gdbarch.h"

#include "x86-nat.h"

#include "i386-tdep.h"
#include "i387-tdep.h"

using namespace windows_nat;

/* If we're not using the old Cygwin header file set, define the
   following which never should have been in the generic Win32 API
   headers in the first place since they were our own invention...  */
#ifndef _GNU_H_WINDOWS_H
enum
  {
    FLAG_TRACE_BIT = 0x100,
  };
#endif

#define DR6_CLEAR_VALUE 0xffff0ff0

#define CHECK(x)	check (x, __FILE__,__LINE__)

static void
check (BOOL ok, const char *file, int line)
{
  if (!ok)
    {
      unsigned err = (unsigned) GetLastError ();
      gdb_printf ("error return %s:%d was %u: %s\n", file, line,
		  err, strwinerror (err));
    }
}

struct x86_windows_per_inferior : public windows_per_inferior
{
  uintptr_t dr[8] {};

  /* The function to use in order to determine whether a register is
     a segment register or not.  */
  segment_register_p_ftype *segment_register_p = nullptr;
};

struct x86_windows_nat_target final : public x86_nat_target<windows_nat_target>
{
  void initialize_windows_arch (bool attaching) override;
  void cleanup_windows_arch () override;

  void fill_thread_context (windows_thread_info *th) override;

  void thread_context_continue (windows_thread_info *th, int killed) override;
  void thread_context_step (windows_thread_info *th) override;

  void fetch_one_register (struct regcache *regcache,
			   windows_thread_info *th, int r) override;
  void store_one_register (const struct regcache *regcache,
			   windows_thread_info *th, int r) override;
};

/* The current process.  */
static x86_windows_per_inferior x86_windows_process;
windows_per_inferior *windows_process = &x86_windows_process;

/* See windows-nat.h.  */

void
x86_windows_nat_target::initialize_windows_arch (bool attaching)
{
  memset (x86_windows_process.dr, 0, sizeof (x86_windows_process.dr));

#ifdef __x86_64__
  x86_windows_process.ignore_first_breakpoint
    = !attaching && x86_windows_process.wow64_process;

  if (!x86_windows_process.wow64_process)
    {
      x86_windows_process.mappings  = amd64_mappings;
      x86_windows_process.segment_register_p = amd64_windows_segment_register_p;
    }
  else
#endif
    {
      x86_windows_process.mappings  = i386_mappings;
      x86_windows_process.segment_register_p = i386_windows_segment_register_p;
    }
}

/* See windows-nat.h.  */

void
x86_windows_nat_target::cleanup_windows_arch ()
{
  x86_cleanup_dregs ();
}

/* See windows-nat.h.  */

void
x86_windows_nat_target::fill_thread_context (windows_thread_info *th)
{
  x86_windows_process.with_context (th, [&] (auto *context)
    {
      context->ContextFlags = WindowsContext<decltype(context)>::all;
      CHECK (get_thread_context (th->h, context));

      /* Copy dr values from that thread.
	 But only if there were not modified since last stop.
	 PR gdb/2388 */
      if (!th->debug_registers_changed)
	{
	  x86_windows_process.dr[0] = context->Dr0;
	  x86_windows_process.dr[1] = context->Dr1;
	  x86_windows_process.dr[2] = context->Dr2;
	  x86_windows_process.dr[3] = context->Dr3;
	  x86_windows_process.dr[6] = context->Dr6;
	  x86_windows_process.dr[7] = context->Dr7;
	}
    });
}

/* See windows-nat.h.  */

void
x86_windows_nat_target::thread_context_continue (windows_thread_info *th,
						 int killed)
{
  x86_windows_process.with_context (th, [&] (auto *context)
    {
      if (th->debug_registers_changed)
	{
	  context->ContextFlags |= WindowsContext<decltype(context)>::debug;
	  context->Dr0 = x86_windows_process.dr[0];
	  context->Dr1 = x86_windows_process.dr[1];
	  context->Dr2 = x86_windows_process.dr[2];
	  context->Dr3 = x86_windows_process.dr[3];
	  context->Dr6 = DR6_CLEAR_VALUE;
	  context->Dr7 = x86_windows_process.dr[7];
	  th->debug_registers_changed = false;
	}

      if (context->ContextFlags)
	{
	  DWORD ec = 0;

	  if (GetExitCodeThread (th->h, &ec)
	      && ec == STILL_ACTIVE)
	    {
	      BOOL status = set_thread_context (th->h, context);

	      if (!killed)
		CHECK (status);
	    }
	  context->ContextFlags = 0;
	}
    });
}

/* See windows-nat.h.  */

void
x86_windows_nat_target::thread_context_step (windows_thread_info *th)
{
  x86_windows_process.with_context (th, [&] (auto *context)
    {
      context->EFlags |= FLAG_TRACE_BIT;
    });
}

/* See windows-nat.h.  */

void
x86_windows_nat_target::fetch_one_register (struct regcache *regcache,
					    windows_thread_info *th, int r)
{
  gdb_assert (r >= 0);
  gdb_assert (!th->reload_context);

  char *context_ptr = x86_windows_process.with_context (th, [] (auto *context)
    {
      return (char *) context;
    });

  char *context_offset = context_ptr + x86_windows_process.mappings[r];
  struct gdbarch *gdbarch = regcache->arch ();
  i386_gdbarch_tdep *tdep = gdbarch_tdep<i386_gdbarch_tdep> (gdbarch);

  gdb_assert (!gdbarch_read_pc_p (gdbarch));
  gdb_assert (gdbarch_pc_regnum (gdbarch) >= 0);
  gdb_assert (!gdbarch_write_pc_p (gdbarch));

  /* GDB treats some registers as 32-bit, where they are in fact only
     16 bits long.  These cases must be handled specially to avoid
     reading extraneous bits from the context.  */
  if (r == I387_FISEG_REGNUM (tdep)
      || x86_windows_process.segment_register_p (r))
    {
      gdb_byte bytes[4] = {};
      memcpy (bytes, context_offset, 2);
      regcache->raw_supply (r, bytes);
    }
  else if (r == I387_FOP_REGNUM (tdep))
    {
      long l = (*((long *) context_offset) >> 16) & ((1 << 11) - 1);
      regcache->raw_supply (r, &l);
    }
  else
    {
      if (th->stopped_at_software_breakpoint
	  && !th->pc_adjusted
	  && r == gdbarch_pc_regnum (gdbarch))
	{
	  int size = register_size (gdbarch, r);
	  if (size == 4)
	    {
	      uint32_t value;
	      memcpy (&value, context_offset, size);
	      value -= gdbarch_decr_pc_after_break (gdbarch);
	      memcpy (context_offset, &value, size);
	    }
	  else
	    {
	      gdb_assert (size == 8);
	      uint64_t value;
	      memcpy (&value, context_offset, size);
	      value -= gdbarch_decr_pc_after_break (gdbarch);
	      memcpy (context_offset, &value, size);
	    }
	  /* Make sure we only rewrite the PC a single time.  */
	  th->pc_adjusted = true;
	}
      regcache->raw_supply (r, context_offset);
    }
}

/* See windows-nat.h.  */

void
x86_windows_nat_target::store_one_register (const struct regcache *regcache,
					    windows_thread_info *th, int r)
{
  gdb_assert (r >= 0);

  char *context_ptr = x86_windows_process.with_context (th, [] (auto *context)
    {
      return (char *) context;
    });

  struct gdbarch *gdbarch = regcache->arch ();
  i386_gdbarch_tdep *tdep = gdbarch_tdep<i386_gdbarch_tdep> (gdbarch);

  /* GDB treats some registers as 32-bit, where they are in fact only
     16 bits long.  These cases must be handled specially to avoid
     overwriting other registers in the context.  */
  if (r == I387_FISEG_REGNUM (tdep)
      || x86_windows_process.segment_register_p (r))
    {
      gdb_byte bytes[4];
      regcache->raw_collect (r, bytes);
      memcpy (context_ptr + x86_windows_process.mappings[r], bytes, 2);
    }
  else if (r == I387_FOP_REGNUM (tdep))
    {
      gdb_byte bytes[4];
      regcache->raw_collect (r, bytes);
      /* The value of FOP occupies the top two bytes in the context,
	 so write the two low-order bytes from the cache into the
	 appropriate spot.  */
      memcpy (context_ptr + x86_windows_process.mappings[r] + 2, bytes, 2);
    }
  else
    regcache->raw_collect (r, context_ptr + x86_windows_process.mappings[r]);
}

/* Hardware watchpoint support, adapted from go32-nat.c code.  */

/* Pass the address ADDR to the inferior in the I'th debug register.
   Here we just store the address in dr array, the registers will be
   actually set up when windows_continue is called.  */
static void
cygwin_set_dr (int i, CORE_ADDR addr)
{
  if (i < 0 || i > 3)
    internal_error (_("Invalid register %d in cygwin_set_dr.\n"), i);
  x86_windows_process.dr[i] = addr;

  for (auto &th : x86_windows_process.thread_list)
    th->debug_registers_changed = true;
}

/* Pass the value VAL to the inferior in the DR7 debug control
   register.  Here we just store the address in D_REGS, the watchpoint
   will be actually set up in windows_wait.  */
static void
cygwin_set_dr7 (unsigned long val)
{
  x86_windows_process.dr[7] = (CORE_ADDR) val;

  for (auto &th : x86_windows_process.thread_list)
    th->debug_registers_changed = true;
}

/* Get the value of debug register I from the inferior.  */

static CORE_ADDR
cygwin_get_dr (int i)
{
  return x86_windows_process.dr[i];
}

/* Get the value of the DR6 debug status register from the inferior.
   Here we just return the value stored in dr[6]
   by the last call to thread_rec for current_event.dwThreadId id.  */
static unsigned long
cygwin_get_dr6 (void)
{
  return (unsigned long) x86_windows_process.dr[6];
}

/* Get the value of the DR7 debug status register from the inferior.
   Here we just return the value stored in dr[7] by the last call to
   thread_rec for current_event.dwThreadId id.  */

static unsigned long
cygwin_get_dr7 (void)
{
  return (unsigned long) x86_windows_process.dr[7];
}

INIT_GDB_FILE (x86_windows_nat)
{
  x86_dr_low.set_control = cygwin_set_dr7;
  x86_dr_low.set_addr = cygwin_set_dr;
  x86_dr_low.get_addr = cygwin_get_dr;
  x86_dr_low.get_status = cygwin_get_dr6;
  x86_dr_low.get_control = cygwin_get_dr7;

  /* x86_dr_low.debug_register_length field is set by
     calling x86_set_debug_register_length function
     in processor windows specific native file.  */

  /* The target is not a global specifically to avoid a C++ "static
     initializer fiasco" situation.  */
  add_inf_child_target (new x86_windows_nat_target);
}

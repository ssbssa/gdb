/* Native-dependent code for Windows x86 (i386 and x86-64).

   Copyright (C) 2025-2026 Free Software Foundation, Inc.

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

#include "windows-tdep.h"
#include "windows-nat.h"
#include "regcache.h"
#include "gdbarch.h"
#include "inferior.h"

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

/* The CONTEXT_DEBUG_REGISTERS define without the arch-specific bit
   (CONTEXT_i386 or CONTEXT_AMD64).  */
#define CONTEXT_DEBUG_REG_FLAG (CONTEXT_DEBUG_REGISTERS & ~CONTEXT_CONTROL)

struct x86_windows_per_inferior : public windows_per_inferior
{
  /* The function to use in order to determine whether a register is
     a segment register or not.  */
  segment_register_p_ftype *segment_register_p = nullptr;

  void fill_thread_context (windows_thread_info *th) override;
  void invalidate_thread_context (windows_thread_info *th) override;
};

struct x86_windows_nat_target final : public x86_nat_target<windows_nat_target>
{
  void initialize_windows_arch (bool attaching) override;
  void cleanup_windows_arch () override;

  void thread_context_continue (windows_thread_info *th, int killed) override;
  void thread_context_step (windows_thread_info *th, bool step) override;

  void fetch_one_register (struct regcache *regcache,
			   windows_thread_info *th, int r) override;
  void store_one_register (const struct regcache *regcache,
			   windows_thread_info *th, int r) override;

  bool is_sw_breakpoint (const EXCEPTION_RECORD *er) const override;
};

/* The current process.  */
static x86_windows_per_inferior x86_windows_process;

/* See windows-nat.h.  */

void
x86_windows_nat_target::initialize_windows_arch (bool attaching)
{
#ifdef __x86_64__
  x86_windows_process.ignore_first_breakpoint
    = !attaching && x86_windows_process.wow64_process;

  if (!x86_windows_process.wow64_process)
    x86_windows_process.segment_register_p = amd64_windows_segment_register_p;
  else
#endif
    x86_windows_process.segment_register_p = i386_windows_segment_register_p;
}

/* See windows-nat.h.  */

void
x86_windows_nat_target::cleanup_windows_arch ()
{
  x86_cleanup_dregs ();
}

/* See nat/windows-nat.h.  */

void
x86_windows_per_inferior::fill_thread_context (windows_thread_info *th)
{
  x86_windows_process.with_context (th, [&] (auto *context)
    {
      if (context->ContextFlags == 0)
	{
	  context->ContextFlags = WindowsContext<decltype(context)>::all;
	  CHECK (get_thread_context (th->h, context));
	}
    });
}

/* See nat/windows-nat.h.  */

void
x86_windows_per_inferior::invalidate_thread_context (windows_thread_info *th)
{
  x86_windows_process.with_context (th, [&] (auto *context)
    {
      context->ContextFlags = 0;
    });
}

/* See windows-nat.h.  */

void
x86_windows_nat_target::thread_context_continue (windows_thread_info *th,
						 int killed)
{
  x86_windows_process.with_context (th, [&] (auto *context)
    {
      struct x86_debug_reg_state *state
	= x86_debug_reg_state (windows_process->process_id);

      if (th->debug_registers_changed)
	{
	  windows_process->fill_thread_context (th);

	  gdb_assert ((context->ContextFlags & CONTEXT_DEBUG_REG_FLAG) != 0);

	  /* Check whether the thread has Dr6 set indicating a
	     watchpoint hit, and we haven't seen the watchpoint event
	     yet (reported as
	     EXCEPTION_SINGLE_STEP/STATUS_WX86_SINGLE_STEP).  In that
	     case, don't change the debug registers.  Changing debug
	     registers, even if to the same values, makes the kernel
	     clear Dr6.  The result would be we would lose the
	     unreported watchpoint hit.  */
	  if ((context->Dr6 & ~DR6_CLEAR_VALUE) != 0)
	    {
	      if (th->last_event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT
		  && (th->last_event.u.Exception.ExceptionRecord.ExceptionCode
		      == EXCEPTION_SINGLE_STEP
		      || (th->last_event.u.Exception.ExceptionRecord.ExceptionCode
			  == STATUS_WX86_SINGLE_STEP)))
		{
		  DEBUG_EVENTS ("0x%x already reported watchpoint", th->tid);
		}
	      else
		{
		  DEBUG_EVENTS ("0x%x last reported something else (0x%x)",
				th->tid,
				th->last_event.dwDebugEventCode);

		  /* Don't touch debug registers.  Let the pending
		     watchpoint event be reported instead.  We will
		     update the debug registers later when the thread
		     is re-resumed by the core after the watchpoint
		     event.  */
		  context->ContextFlags &= ~CONTEXT_DEBUG_REG_FLAG;
		}
	    }
	  else
	    DEBUG_EVENTS ("0x%x has no dr6 set", th->tid);

	  if ((context->ContextFlags & CONTEXT_DEBUG_REG_FLAG) != 0)
	    {
	      DEBUG_EVENTS ("0x%x changing dregs", th->tid);
	      context->Dr0 = state->dr_mirror[0];
	      context->Dr1 = state->dr_mirror[1];
	      context->Dr2 = state->dr_mirror[2];
	      context->Dr3 = state->dr_mirror[3];
	      context->Dr6 = DR6_CLEAR_VALUE;
	      context->Dr7 = state->dr_control_mirror;
	    }

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
x86_windows_nat_target::thread_context_step (windows_thread_info *th,
					     bool enable)
{
  x86_windows_process.with_context (th, [&] (auto *context)
    {
      if (enable)
	context->EFlags |= FLAG_TRACE_BIT;
      else
	context->EFlags &= ~FLAG_TRACE_BIT;
    });
}

/* Get pointer to register R inside CONTEXT.  */

template<typename Context>
static char *
get_context_reg_ptr (Context *context, int r)
{
  const int *mappings;
  int mappings_count;
#ifdef __x86_64__
  if (!x86_windows_process.wow64_process)
    {
      mappings = amd64_mappings;
      mappings_count = amd64_mappings_count;
    }
  else
#endif
    {
      mappings = i386_mappings;
      mappings_count = i386_mappings_count;
    }

  char *context_offset;
  if (r < mappings_count)
    context_offset = (char *) context + mappings[r];
  else
    gdb_assert_not_reached ("invalid register number %d", r);

  return context_offset;
}

/* See windows-nat.h.  */

void
x86_windows_nat_target::fetch_one_register (struct regcache *regcache,
					    windows_thread_info *th, int r)
{
  gdb_assert (r >= 0);

  struct gdbarch *gdbarch = regcache->arch ();
  i386_gdbarch_tdep *tdep = gdbarch_tdep<i386_gdbarch_tdep> (gdbarch);

  char *context_offset
    = x86_windows_process.with_context (th, [&] (auto *context)
    {
      return get_context_reg_ptr (context, r);
    });

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

  struct gdbarch *gdbarch = regcache->arch ();
  i386_gdbarch_tdep *tdep = gdbarch_tdep<i386_gdbarch_tdep> (gdbarch);

  char *context_offset
    = x86_windows_process.with_context (th, [&] (auto *context)
    {
      gdb_assert (context->ContextFlags != 0);
      return get_context_reg_ptr (context, r);
    });

  /* GDB treats some registers as 32-bit, where they are in fact only
     16 bits long.  These cases must be handled specially to avoid
     overwriting other registers in the context.  */
  if (r == I387_FISEG_REGNUM (tdep)
      || x86_windows_process.segment_register_p (r))
    {
      gdb_byte bytes[4];
      regcache->raw_collect (r, bytes);
      memcpy (context_offset, bytes, 2);
    }
  else if (r == I387_FOP_REGNUM (tdep))
    {
      gdb_byte bytes[4];
      regcache->raw_collect (r, bytes);
      /* The value of FOP occupies the top two bytes in the context,
	 so write the two low-order bytes from the cache into the
	 appropriate spot.  */
      memcpy (context_offset + 2, bytes, 2);
    }
  else
    regcache->raw_collect (r, context_offset);
}

/* See windows-nat.h.  */

bool
x86_windows_nat_target::is_sw_breakpoint (const EXCEPTION_RECORD *er) const
{
  return (er->ExceptionCode == EXCEPTION_BREAKPOINT
	  || er->ExceptionCode == STATUS_WX86_BREAKPOINT);
}

/* Hardware watchpoint support, adapted from go32-nat.c code.  */

/* Pass the address ADDR to the inferior in the I'th debug register.
   Here we just store the address in dr array, the registers will be
   actually set up when windows_continue is called.  */
static void
windows_set_dr (int i, CORE_ADDR addr)
{
  if (i < 0 || i > 3)
    internal_error (_("Invalid register %d in windows_set_dr.\n"), i);

  windows_debug_registers_changed_all_threads ();
}

/* Pass the value VAL to the inferior in the DR7 debug control
   register.  Here we just store the address in D_REGS, the watchpoint
   will be actually set up in windows_wait.  */
static void
windows_set_dr7 (unsigned long val)
{
  windows_debug_registers_changed_all_threads ();
}

/* Get the value of debug register I from the inferior.  */

static CORE_ADDR
windows_get_dr (int i)
{
  windows_thread_info *th = windows_process->find_thread (inferior_ptid);

  return windows_process->with_context (th, [&] (auto *context) -> CORE_ADDR
    {
      gdb_assert (context->ContextFlags != 0);
      switch (i)
	{
	case 0:
	  return context->Dr0;
	case 1:
	  return context->Dr1;
	case 2:
	  return context->Dr2;
	case 3:
	  return context->Dr3;
	case 6:
	  return context->Dr6;
	case 7:
	  return context->Dr7;
	};

      gdb_assert_not_reached ("invalid x86 dr register number: %d", i);
    });
}

/* Get the value of the DR6 debug status register from the
   inferior.  */

static unsigned long
windows_get_dr6 (void)
{
  return windows_get_dr (6);
}

/* Get the value of the DR7 debug status register from the
   inferior.  */

static unsigned long
windows_get_dr7 (void)
{
  return windows_get_dr (7);
}

static int
display_selector (HANDLE thread, DWORD sel)
{
  LDT_ENTRY info;
  BOOL ret = windows_process->with_context (nullptr, [&] (auto *context)
    {
      return get_thread_selector_entry (context, thread, sel, &info);
    });
  /* codespell:ignore-begin.  */
  if (ret)
    {
      int base, limit;
      gdb_printf ("0x%03x: ", (unsigned) sel);
      if (!info.HighWord.Bits.Pres)
	{
	  gdb_puts ("Segment not present\n");
	  return 0;
	}
      base = (info.HighWord.Bits.BaseHi << 24) +
	     (info.HighWord.Bits.BaseMid << 16)
	     + info.BaseLow;
      limit = (info.HighWord.Bits.LimitHi << 16) + info.LimitLow;
      if (info.HighWord.Bits.Granularity)
	limit = (limit << 12) | 0xfff;
      gdb_printf ("base=0x%08x limit=0x%08x", base, limit);
      if (info.HighWord.Bits.Default_Big)
	gdb_puts(" 32-bit ");
      else
	gdb_puts(" 16-bit ");
      switch ((info.HighWord.Bits.Type & 0xf) >> 1)
	{
	case 0:
	  gdb_puts ("Data (Read-Only, Exp-up");
	  break;
	case 1:
	  gdb_puts ("Data (Read/Write, Exp-up");
	  break;
	case 2:
	  gdb_puts ("Unused segment (");
	  break;
	case 3:
	  gdb_puts ("Data (Read/Write, Exp-down");
	  break;
	case 4:
	  gdb_puts ("Code (Exec-Only, N.Conf");
	  break;
	case 5:
	  gdb_puts ("Code (Exec/Read, N.Conf");
	  break;
	case 6:
	  gdb_puts ("Code (Exec-Only, Conf");
	  break;
	case 7:
	  gdb_puts ("Code (Exec/Read, Conf");
	  break;
	default:
	  gdb_printf ("Unknown type 0x%lx",
		      (unsigned long) info.HighWord.Bits.Type);
	}
      if ((info.HighWord.Bits.Type & 0x1) == 0)
	gdb_puts(", N.Acc");
      gdb_puts (")\n");
      if ((info.HighWord.Bits.Type & 0x10) == 0)
	gdb_puts("System selector ");
      gdb_printf ("Privilege level = %ld. ",
		  (unsigned long) info.HighWord.Bits.Dpl);
      if (info.HighWord.Bits.Granularity)
	gdb_puts ("Page granular.\n");
      else
	gdb_puts ("Byte granular.\n");
      return 1;
    }
  else
    {
      DWORD err = GetLastError ();
      if (err == ERROR_NOT_SUPPORTED)
	gdb_printf ("Function not supported\n");
      else
	gdb_printf ("Invalid selector 0x%x.\n", (unsigned) sel);
      return 0;
    }
  /* codespell:ignore-end.  */
}

static void
display_selectors (const char * args, int from_tty)
{
  if (inferior_ptid == null_ptid)
    {
      gdb_puts ("Impossible to display selectors now.\n");
      return;
    }

  windows_thread_info *current_windows_thread
    = windows_process->find_thread (inferior_ptid);

  if (!args)
    {
      windows_process->with_context (current_windows_thread, [&] (auto *context)
	{
	  gdb_puts ("Selector $cs\n");
	  display_selector (current_windows_thread->h, context->SegCs);
	  gdb_puts ("Selector $ds\n");
	  display_selector (current_windows_thread->h, context->SegDs);
	  gdb_puts ("Selector $es\n");
	  display_selector (current_windows_thread->h, context->SegEs);
	  gdb_puts ("Selector $ss\n");
	  display_selector (current_windows_thread->h, context->SegSs);
	  gdb_puts ("Selector $fs\n");
	  display_selector (current_windows_thread->h, context->SegFs);
	  gdb_puts ("Selector $gs\n");
	  display_selector (current_windows_thread->h, context->SegGs);
	});
    }
  else
    {
      int sel;
      sel = parse_and_eval_long (args);
      gdb_printf ("Selector \"%s\"\n",args);
      display_selector (current_windows_thread->h, sel);
    }
}

INIT_GDB_FILE (x86_windows_nat)
{
  x86_dr_low.set_control = windows_set_dr7;
  x86_dr_low.set_addr = windows_set_dr;
  x86_dr_low.get_addr = windows_get_dr;
  x86_dr_low.get_status = windows_get_dr6;
  x86_dr_low.get_control = windows_get_dr7;

  /* x86_dr_low.debug_register_length field is set by
     calling x86_set_debug_register_length function
     in processor windows specific native file.  */

  /* The target is not a global specifically to avoid a C++ "static
     initializer fiasco" situation.  */
  add_inf_child_target (new x86_windows_nat_target);

  windows_process = &x86_windows_process;

  add_cmd ("selector", class_info, display_selectors,
	   _("Display selectors infos."),
	   &info_w32_cmdlist);
}

/* Copyright (C) 2025 Free Software Foundation, Inc.

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

#include "win32-low.h"
#include "arch/aarch64.h"
#include "nat/aarch64-hw-point.h"
#include "tdesc.h"

using namespace windows_nat;

#define FLAG_TRACE_BIT 0x200000

static struct aarch64_debug_reg_state debug_reg_state;

static void
update_debug_registers (thread_info *thread)
{
  auto th = static_cast<windows_thread_info *> (thread->target_data ());

  /* The actual update is done later just before resuming the lwp,
     we just mark that the registers need updating.  */
  th->debug_registers_changed = true;
}

void
aarch64_notify_debug_reg_change (ptid_t ptid,
				 int is_watchpoint, unsigned int idx)
{
  /* Only update the threads of this process.  */
  current_process ()->for_each_thread (update_debug_registers);
}

/* Breakpoint/watchpoint support.  */

static int
i386_supports_z_point_type (char z_type)
{
  switch (z_type)
    {
    case Z_PACKET_HW_BP:
    case Z_PACKET_WRITE_WP:
    case Z_PACKET_ACCESS_WP:
      return 1;
    default:
      return 0;
    }
}

static int
i386_insert_point (enum raw_bkpt_type type, CORE_ADDR addr,
		   int len, struct raw_breakpoint *bp)
{
  int ret;
  enum target_hw_bp_type targ_type;

  /* Determine the type from the raw breakpoint type.  */
  targ_type = raw_bkpt_type_to_target_hw_bp_type (type);

  if (targ_type != hw_execute)
    {
      if (aarch64_region_ok_for_watchpoint (addr, len))
	ret = aarch64_handle_watchpoint (targ_type, addr, len,
					 1 /* is_insert */, current_thread->id,
					 &debug_reg_state);
      else
	ret = -1;
    }
  else
    {
      if (len == 3)
	{
	  /* LEN is 3 means the breakpoint is set on a 32-bit thumb
	     instruction.   Set it to 2 to correctly encode length bit
	     mask in hardware/watchpoint control register.  */
	  len = 2;
	}
      ret = aarch64_handle_breakpoint (targ_type, addr, len,
				       1 /* is_insert */, current_thread->id,
				       &debug_reg_state);
    }

  return ret;
}

static int
i386_remove_point (enum raw_bkpt_type type, CORE_ADDR addr,
		   int len, struct raw_breakpoint *bp)
{
  int ret;
  enum target_hw_bp_type targ_type;

  /* Determine the type from the raw breakpoint type.  */
  targ_type = raw_bkpt_type_to_target_hw_bp_type (type);

  /* Set up state pointers.  */
  if (targ_type != hw_execute)
    ret =
      aarch64_handle_watchpoint (targ_type, addr, len, 0 /* is_insert */,
				 current_thread->id, &debug_reg_state);
  else
    {
      if (len == 3)
	{
	  /* LEN is 3 means the breakpoint is set on a 32-bit thumb
	     instruction.   Set it to 2 to correctly encode length bit
	     mask in hardware/watchpoint control register.  */
	  len = 2;
	}
      ret = aarch64_handle_breakpoint (targ_type, addr, len,
				       0 /* is_insert */,  current_thread->id,
				       &debug_reg_state);
    }

  return ret;
}

static CORE_ADDR
x86_stopped_data_address (void)
{
  if (windows_process.siginfo_er.ExceptionCode != EXCEPTION_BREAKPOINT ||
      windows_process.siginfo_er.NumberParameters != 2)
    return 0;

  const CORE_ADDR addr_trap
    = (CORE_ADDR) windows_process.siginfo_er.ExceptionInformation[1];

  CORE_ADDR result;
  if (aarch64_stopped_data_address (&debug_reg_state, addr_trap, &result))
    return result;

  return 0;
}

static int
x86_stopped_by_watchpoint (void)
{
  return x86_stopped_data_address () != 0;
}

static void
i386_initial_stuff (void)
{
  memset (&debug_reg_state, 0, sizeof (debug_reg_state));
}

static void
i386_get_thread_context (windows_thread_info *th)
{
  windows_process.with_context (th, [&] (auto *context)
    {
      /* Requesting the CONTEXT_EXTENDED_REGISTERS register set fails if
	 the system doesn't support extended registers.  */
      static DWORD extended_registers
	= WindowsContext<decltype(context)>::extended;

 again:
      context->ContextFlags = (WindowsContext<decltype(context)>::full
			       | WindowsContext<decltype(context)>::floating
			       | WindowsContext<decltype(context)>::debug
			       | extended_registers);

      BOOL ret = get_thread_context (th->h, context);
      if (!ret)
	{
	  DWORD e = GetLastError ();

	  if (extended_registers && e == ERROR_INVALID_PARAMETER)
	    {
	      extended_registers = 0;
	      goto again;
	    }

	  error ("GetThreadContext failure %ld\n", (long) e);
	}
    });
}

static void
i386_prepare_to_resume (windows_thread_info *th)
{
  if (th->debug_registers_changed)
    {
      win32_require_context (th);

      windows_process.with_context (th, [&] (auto *context)
	{
	  for (int i = 0; i < aarch64_num_bp_regs; i++)
	    {
	      context->Bvr[i] = debug_reg_state.dr_addr_bp[i];
	      context->Bcr[i] = debug_reg_state.dr_ctrl_bp[i];
	    }
	  for (int i = 0; i < aarch64_num_wp_regs; i++)
	    {
	      context->Wvr[i] = debug_reg_state.dr_addr_wp[i];
	      context->Wcr[i] = debug_reg_state.dr_ctrl_wp[i];
	    }
	});

      th->debug_registers_changed = false;
    }
}

static void
i386_thread_added (windows_thread_info *th)
{
  th->debug_registers_changed = true;

  windows_process.initialize_context (th, 0);
}

static void
i386_single_step (windows_thread_info *th)
{
  windows_process.with_context (th, [] (auto *context)
    {
      context->Cpsr |= FLAG_TRACE_BIT;
    });
}

/* An array of offset mappings into a Win32 Context structure.
   This is a one-to-one mapping which is indexed by gdb's register
   numbers.  It retrieves an offset into the context structure where
   the 4 byte register is located.
   An offset value of -1 indicates that Win32 does not provide this
   register in it's CONTEXT structure.  In this case regptr will return
   a pointer into a dummy register.  */
#define context_offset(x) (offsetof (CONTEXT, x))
static const int aarch64_mappings[] = {
  context_offset (X0),
  context_offset (X1),
  context_offset (X2),
  context_offset (X3),
  context_offset (X4),
  context_offset (X5),
  context_offset (X6),
  context_offset (X7),
  context_offset (X8),
  context_offset (X9),
  context_offset (X10),
  context_offset (X11),
  context_offset (X12),
  context_offset (X13),
  context_offset (X14),
  context_offset (X15),
  context_offset (X16),
  context_offset (X17),
  context_offset (X18),
  context_offset (X19),
  context_offset (X20),
  context_offset (X21),
  context_offset (X22),
  context_offset (X23),
  context_offset (X24),
  context_offset (X25),
  context_offset (X26),
  context_offset (X27),
  context_offset (X28),
  context_offset (Fp),
  context_offset (Lr),
  context_offset (Sp),
  context_offset (Pc),
  context_offset (Cpsr),
  context_offset (V[0]),
  context_offset (V[1]),
  context_offset (V[2]),
  context_offset (V[3]),
  context_offset (V[4]),
  context_offset (V[5]),
  context_offset (V[6]),
  context_offset (V[7]),
  context_offset (V[8]),
  context_offset (V[9]),
  context_offset (V[10]),
  context_offset (V[11]),
  context_offset (V[12]),
  context_offset (V[13]),
  context_offset (V[14]),
  context_offset (V[15]),
  context_offset (V[16]),
  context_offset (V[17]),
  context_offset (V[18]),
  context_offset (V[19]),
  context_offset (V[20]),
  context_offset (V[21]),
  context_offset (V[22]),
  context_offset (V[23]),
  context_offset (V[24]),
  context_offset (V[25]),
  context_offset (V[26]),
  context_offset (V[27]),
  context_offset (V[28]),
  context_offset (V[29]),
  context_offset (V[30]),
  context_offset (V[31]),
  context_offset (Fpsr),
  context_offset (Fpcr),
};
#undef context_offset

static inline void
get_mappings (const int *&mappings, int &mappings_count)
{
  mappings = aarch64_mappings;
  mappings_count = sizeof (aarch64_mappings) / sizeof (aarch64_mappings[0]);
}

/* Fetch register from gdbserver regcache data.  */
static void
i386_fetch_inferior_register (struct regcache *regcache,
			      windows_thread_info *th, int r)
{
  const int *mappings;
  int mappings_count;
  get_mappings (mappings, mappings_count);

  char *context_ptr = (char *) th->context;
  char *context_offset;
  if (r < mappings_count)
    context_offset = context_ptr + mappings[r];
  else
    gdb_assert_not_reached ("invalid register number %d", r);

  supply_register (regcache, r, context_offset);
}

/* Store a new register value into the thread context of TH.  */
static void
i386_store_inferior_register (struct regcache *regcache,
			      windows_thread_info *th, int r)
{
  const int *mappings;
  int mappings_count;
  get_mappings (mappings, mappings_count);

  char *context_ptr = (char *) th->context;
  char *context_offset;
  if (r < mappings_count)
    context_offset = context_ptr + mappings[r];
  else
    gdb_assert_not_reached ("invalid register number %d", r);

  collect_register (regcache, r, context_offset);
}

static const unsigned char aarch64_breakpoint[] = {0x00, 0x00, 0x3e, 0xd4};
#define aarch64_breakpoint_len 4

static void
i386_arch_setup (void)
{
  struct target_desc *tdesc;

  aarch64_num_bp_regs = 6;
  aarch64_num_wp_regs = 1;

  tdesc = aarch64_create_target_description ({});

  std::vector<const char *> expedited_registers;
  expedited_registers.push_back ("x29");
  expedited_registers.push_back ("sp");
  expedited_registers.push_back ("pc");
  expedited_registers.push_back (nullptr);

  init_target_desc (tdesc, (const char **) expedited_registers.data (),
		    WINDOWS_OSABI);
  aarch64_tdesc = tdesc;
}

/* Implement win32_target_ops "num_regs" method.  */

static int
i386_win32_num_regs (void)
{
  int num_regs = sizeof (aarch64_mappings) / sizeof (aarch64_mappings[0]);
  return num_regs;
}

/* Implement win32_target_ops "get_pc" method.  */

static CORE_ADDR
i386_win32_get_pc (struct regcache *regcache)
{
  uint64_t pc;

  collect_register_by_name (regcache, "pc", &pc);
  return (CORE_ADDR) pc;
}

/* Implement win32_target_ops "set_pc" method.  */

static void
i386_win32_set_pc (struct regcache *regcache, CORE_ADDR pc)
{
  uint64_t newpc = pc;

  supply_register_by_name (regcache, "pc", &newpc);
}

struct win32_target_ops the_low_target = {
  i386_arch_setup,
  i386_win32_num_regs,
  i386_initial_stuff,
  i386_get_thread_context,
  i386_prepare_to_resume,
  i386_thread_added,
  i386_fetch_inferior_register,
  i386_store_inferior_register,
  i386_single_step,
  aarch64_breakpoint,
  aarch64_breakpoint_len,
  aarch64_breakpoint_len,
  i386_win32_get_pc,
  i386_win32_set_pc,
  i386_supports_z_point_type,
  i386_insert_point,
  i386_remove_point,
  x86_stopped_by_watchpoint,
  x86_stopped_data_address
};

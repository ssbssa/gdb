/* Native-dependent code for Windows AArch64.

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
#include "inferior.h"

#include "aarch64-nat.h"

using namespace windows_nat;

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

struct aarch64_windows_per_inferior : public windows_per_inferior
{
  aarch64_debug_reg_state dr_state;
};

struct aarch64_windows_nat_target final :
  public aarch64_nat_target<windows_nat_target>
{
  bool stopped_data_address (CORE_ADDR *) override;
  bool stopped_by_watchpoint () override;

  void initialize_windows_arch (bool attaching) override;
  void cleanup_windows_arch () override;

  void fill_thread_context (windows_thread_info *th) override;

  void thread_context_continue (windows_thread_info *th, int killed) override;
  void thread_context_step (windows_thread_info *th) override;

  void fetch_one_register (struct regcache *regcache,
			   windows_thread_info *th, int r) override;
  void store_one_register (const struct regcache *regcache,
			   windows_thread_info *th, int r) override;

  bool is_sw_breakpoint (EXCEPTION_RECORD *er) override;
};

/* The current process.  */
static aarch64_windows_per_inferior aarch64_windows_process;
windows_per_inferior *windows_process = &aarch64_windows_process;

#define context_offset(x) (offsetof (CONTEXT, x))
const int aarch64_mappings[] =
{
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

/* Implement the "stopped_data_address" target_ops method.  */

bool
aarch64_windows_nat_target::stopped_data_address (CORE_ADDR *addr_p)
{
  if (aarch64_windows_process.siginfo_er.ExceptionCode != EXCEPTION_BREAKPOINT
      || aarch64_windows_process.siginfo_er.NumberParameters != 2)
    return false;

  const CORE_ADDR addr_trap
    = (CORE_ADDR) aarch64_windows_process.siginfo_er.ExceptionInformation[1];

  struct aarch64_debug_reg_state *state
    = aarch64_get_debug_reg_state (inferior_ptid.pid ());
  return aarch64_stopped_data_address (state, addr_trap, addr_p);
}

/* Implement the "stopped_by_watchpoint" target_ops method.  */

bool
aarch64_windows_nat_target::stopped_by_watchpoint ()
{
  return stopped_data_address (nullptr);
}

/* See windows-nat.h.  */

void
aarch64_windows_nat_target::initialize_windows_arch (bool attaching)
{
  memset (&aarch64_windows_process.dr_state, 0,
	  sizeof (aarch64_windows_process.dr_state));

  aarch64_windows_process.mappings  = aarch64_mappings;
}

/* See windows-nat.h.  */

void
aarch64_windows_nat_target::cleanup_windows_arch ()
{
  aarch64_remove_debug_reg_state (inferior_ptid.pid ());
}

/* See windows-nat.h.  */

void
aarch64_windows_nat_target::fill_thread_context (windows_thread_info *th)
{
  CONTEXT *context = &th->context;

  context->ContextFlags = WindowsContext<decltype(context)>::all;
  CHECK (get_thread_context (th->h, context));
}

/* See windows-nat.h.  */

void
aarch64_windows_nat_target::thread_context_continue (windows_thread_info *th,
						     int killed)
{
  CONTEXT *context = &th->context;

  if (th->debug_registers_changed)
    {
      context->ContextFlags |= WindowsContext<decltype(context)>::debug;
      for (int i = 0; i < aarch64_num_bp_regs; i++)
	{
	  context->Bvr[i] = aarch64_windows_process.dr_state.dr_addr_bp[i];
	  context->Bcr[i] = aarch64_windows_process.dr_state.dr_ctrl_bp[i];
	}
      for (int i = 0; i < aarch64_num_wp_regs; i++)
	{
	  context->Wvr[i] = aarch64_windows_process.dr_state.dr_addr_wp[i];
	  context->Wcr[i] = aarch64_windows_process.dr_state.dr_ctrl_wp[i];
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
}

/* See windows-nat.h.  */

void
aarch64_windows_nat_target::thread_context_step (windows_thread_info *th)
{
  th->context.Cpsr |= 0x200000;
}

/* See windows-nat.h.  */

void
aarch64_windows_nat_target::fetch_one_register (struct regcache *regcache,
						windows_thread_info *th, int r)
{
  gdb_assert (r >= 0);
  gdb_assert (!th->reload_context);

  char *context_ptr = (char *) &th->context;
  char *context_offset = context_ptr + aarch64_windows_process.mappings[r];
  struct gdbarch *gdbarch = regcache->arch ();

  gdb_assert (!gdbarch_read_pc_p (gdbarch));
  gdb_assert (gdbarch_pc_regnum (gdbarch) >= 0);
  gdb_assert (!gdbarch_write_pc_p (gdbarch));

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

/* See windows-nat.h.  */

void
aarch64_windows_nat_target::store_one_register (const struct regcache *regcache,
						windows_thread_info *th, int r)
{
  gdb_assert (r >= 0);

  char *context_ptr = (char *) &th->context;

  regcache->raw_collect (r, context_ptr + aarch64_windows_process.mappings[r]);
}

/* See windows-nat.h.  */

bool
aarch64_windows_nat_target::is_sw_breakpoint (EXCEPTION_RECORD *er)
{
  /* On aarch64, hardware breakpoints also get EXCEPTION_BREAKPOINT,
     but they can be recognized with ExceptionInformation.  */
  return (er->ExceptionCode == EXCEPTION_BREAKPOINT
	  && er->NumberParameters == 1
	  && er->ExceptionInformation[0] == 0);
}

/* Notify all threads that the debug registers changed.  */

void
aarch64_notify_debug_reg_change (ptid_t ptid,
				 int is_watchpoint, unsigned int idx)
{
  struct aarch64_debug_reg_state *state
    = aarch64_get_debug_reg_state (inferior_ptid.pid ());
  aarch64_windows_process.dr_state = *state;

  for (auto &th : aarch64_windows_process.thread_list)
    th->debug_registers_changed = true;
}

INIT_GDB_FILE (x86_windows_nat)
{
  aarch64_initialize_hw_point ();

  /* Get ID_AA64DFR0_EL1 value (CP 4028) from registry.  */
  aarch64_num_bp_regs = 0;
  uint64_t cp4028;
  DWORD cp4028_size = sizeof(cp4028);
  if (RegGetValueA (HKEY_LOCAL_MACHINE,
		    "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		    "CP 4028", RRF_RT_REG_QWORD, NULL, &cp4028, &cp4028_size)
      == ERROR_SUCCESS)
    {
      /* Bits 12-15 are the number of breakpoints, minus 1.  */
      aarch64_num_bp_regs = ((cp4028 & 0xf000) >> 12) + 1;
      if (aarch64_num_bp_regs > ARM64_MAX_BREAKPOINTS)
	aarch64_num_bp_regs = ARM64_MAX_BREAKPOINTS;
    }

  /* ARM64_MAX_WATCHPOINTS is 2, but only 1 works.  */
  aarch64_num_wp_regs = 1;

  /* The target is not a global specifically to avoid a C++ "static
     initializer fiasco" situation.  */
  add_inf_child_target (new aarch64_windows_nat_target);
}

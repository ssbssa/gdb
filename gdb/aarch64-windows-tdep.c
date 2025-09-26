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

#include "gdbarch.h"
#include "aarch64-tdep.h"
#include "arch-utils.h"
#include "regset.h"
#include "windows-tdep.h"

static const struct regcache_map_entry aarch64_windows_gregmap[] =
{
  { 1, REGCACHE_MAP_SKIP, 4 }, /* ContextFlags */
  { 1, AARCH64_CPSR_REGNUM, 4 },
  { 29, AARCH64_X0_REGNUM, 8 },
  { 1, AARCH64_FP_REGNUM, 8 },
  { 1, AARCH64_LR_REGNUM, 8 },
  { 1, AARCH64_SP_REGNUM, 8 },
  { 1, AARCH64_PC_REGNUM, 8 },
  { 32, AARCH64_V0_REGNUM, 16 },
  { 1, AARCH64_FPSR_REGNUM, 4 },
  { 1, AARCH64_FPCR_REGNUM, 4 },
  { 0 }
};

#define AARCH64_WINDOWS_SIZEOF_GREGSET 318

static const struct regset aarch64_windows_gregset =
{
  aarch64_windows_gregmap, regcache_supply_regset, regcache_collect_regset,
  REGSET_VARIABLE_SIZE
};

/* Implement the "iterate_over_regset_sections" gdbarch method.  */

static void
aarch64_win_iterate_over_regset_sections (struct gdbarch *gdbarch,
					  iterate_over_regset_sections_cb *cb,
					  void *cb_data,
					  const struct regcache *regcache)
{
  cb (".reg", AARCH64_WINDOWS_SIZEOF_GREGSET, AARCH64_WINDOWS_SIZEOF_GREGSET,
      &aarch64_windows_gregset, NULL, cb_data);
}

/* Implement the "auto_wide_charset" gdbarch method.  */

static const char *
aarch64_windows_auto_wide_charset (void)
{
  return "UTF-16";
}

constexpr gdb_byte aarch64_windows_breakpoint[] = {0x00, 0x00, 0x3e, 0xd4};

typedef BP_MANIPULATION (aarch64_windows_breakpoint) aarch64_w_breakpoint;

/* gdbarch initialization for Windows on AArch64.  */

static void
aarch64_windows_init_abi (struct gdbarch_info info, struct gdbarch *gdbarch)
{
  set_gdbarch_ps_regnum (gdbarch, AARCH64_CPSR_REGNUM);

  set_gdbarch_auto_wide_charset (gdbarch, aarch64_windows_auto_wide_charset);

  set_gdbarch_breakpoint_kind_from_pc (gdbarch,
				       aarch64_w_breakpoint::kind_from_pc);
  set_gdbarch_sw_breakpoint_from_kind (gdbarch,
				       aarch64_w_breakpoint::bp_from_kind);
  set_gdbarch_decr_pc_after_break (gdbarch, 4);

  set_gdbarch_iterate_over_regset_sections
    (gdbarch, aarch64_win_iterate_over_regset_sections);

  /* Core file support.  */
  set_gdbarch_core_xfer_shared_libraries
    (gdbarch, windows_core_xfer_shared_libraries);
  set_gdbarch_core_pid_to_str (gdbarch, windows_core_pid_to_str);

  windows_init_abi (info, gdbarch);

  /* On Windows, "long"s are only 32bit.  */
  set_gdbarch_long_bit (gdbarch, 32);
}

static gdb_osabi
aarch64_windows_osabi_sniffer (bfd *abfd)
{
  const char *target_name = bfd_get_target (abfd);

  if (!streq (target_name, "pei-aarch64-little"))
    return GDB_OSABI_UNKNOWN;

  return GDB_OSABI_WINDOWS;
}

INIT_GDB_FILE (aarch64_windows_tdep)
{
  gdbarch_register_osabi (bfd_arch_aarch64, 0, GDB_OSABI_WINDOWS,
			  aarch64_windows_init_abi);

  gdbarch_register_osabi_sniffer (bfd_arch_aarch64, bfd_target_coff_flavour,
				  aarch64_windows_osabi_sniffer);
}

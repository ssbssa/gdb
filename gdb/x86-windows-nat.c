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

#include "x86-nat.h"

struct x86_windows_per_inferior : public windows_per_inferior
{
};

struct x86_windows_nat_target final : public x86_nat_target<windows_nat_target>
{
};

/* The current process.  */
static x86_windows_per_inferior x86_windows_process;
windows_per_inferior *windows_process = &x86_windows_process;

INIT_GDB_FILE (x86_windows_nat)
{
  /* The target is not a global specifically to avoid a C++ "static
     initializer fiasco" situation.  */
  add_inf_child_target (new x86_windows_nat_target);
}

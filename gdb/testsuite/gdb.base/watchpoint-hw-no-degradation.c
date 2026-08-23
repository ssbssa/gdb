/* This testcase is part of GDB, the GNU debugger.

   Copyright 2026 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

int
main (void)
{
  size_t len = sysconf(_SC_PAGESIZE);

  /* Map and unmap memory block to get address.  */
  void *p = mmap (0, len, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
  if (p == MAP_FAILED)
    {
      perror ("mmap");
      return 1;
    }
  munmap (p, len);

  /* Now memory block at address P is inaccessible.
     Remap block at same address, so it becomes accessible again.  */
  p = mmap (p, len, PROT_READ|PROT_WRITE,
	    MAP_ANON|MAP_PRIVATE|MAP_FIXED, -1, 0);
  if (p == MAP_FAILED)
    {
      perror ("mmap");
      return 1;
    }

  *(int *) p = 1;

  return 0;
}

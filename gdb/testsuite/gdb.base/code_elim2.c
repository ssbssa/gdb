/* This testcase is part of GDB, the GNU debugger.

   Copyright 2010-2023 Free Software Foundation, Inc.

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

int my_global_symbol = 42;

static int my_static_symbol;

int
my_global_func ()
{
  my_static_symbol = my_global_symbol;
  my_global_symbol = my_static_symbol + my_global_symbol;
  return my_global_symbol;
}

int
main ()
{
  return my_global_func ();
}

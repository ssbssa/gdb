/* This testcase is part of GDB, the GNU debugger.

   Copyright 2020 Free Software Foundation, Inc.

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

int ix;
struct two
{
  int v1, v2;
};
struct t
{
  int i1, i2, i3;
  double d1, d2, d3;
  int *p1, *p2, *p3;
  struct two t1, t2, t3;
  int ia[10];
  int ea[3];
  int *ipa[5];
} t1 = {0, 0, 1, 0, 2.5, 0, &ix, 0, 0, {0, 0}, {3, 0}, {4, 5},
	{0, 1, 2, 0, 0, 3, 4, 5, 0, 6}, {0, 0, 0},
	{0, &ix, 0, 0, &t1.i1}};

int
main ()
{
  return 0;
}

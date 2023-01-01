--  Copyright 2007-2023 Free Software Foundation, Inc.
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 3 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program.  If not, see <http://www.gnu.org/licenses/>.

procedure Foo is
begin

   begin
      raise Constraint_Error with "ignore C_E";  -- SPOT1
   exception
      when others =>
         null;
   end;

   begin
      raise Program_Error;  -- SPOT2
   exception
      when others =>
         null;
   end;

   begin
      pragma Assert (False);  -- SPOT3
      null;
   exception
      when others =>
         null;
   end;

   raise Constraint_Error;  -- SPOT4

end Foo;

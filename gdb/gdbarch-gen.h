/* THIS FILE IS GENERATED -*- buffer-read-only: t -*- */
/* vi:set ro: */

/* Dynamic architecture support for GDB, the GNU debugger.

   Copyright (C) 1998-2026 Free Software Foundation, Inc.

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

/* To regenerate this file, run:
   ./gdbarch.py
*/



/* The following are pre-initialized by GDBARCH.  */

const struct bfd_arch_info *gdbarch_bfd_arch_info (struct gdbarch *gdbarch);
/* set_gdbarch_bfd_arch_info() - not applicable - pre-initialized.  */

enum bfd_endian gdbarch_byte_order (struct gdbarch *gdbarch);
/* set_gdbarch_byte_order() - not applicable - pre-initialized.  */

enum bfd_endian gdbarch_byte_order_for_code (struct gdbarch *gdbarch);
/* set_gdbarch_byte_order_for_code() - not applicable - pre-initialized.  */

enum gdb_osabi gdbarch_osabi (struct gdbarch *gdbarch);
/* set_gdbarch_osabi() - not applicable - pre-initialized.  */

const struct target_desc *gdbarch_target_desc (struct gdbarch *gdbarch);
/* set_gdbarch_target_desc() - not applicable - pre-initialized.  */


/* The following are initialized by the target dependent code.  */

/* Number of bits in a short or unsigned short for the target machine. */

int gdbarch_short_bit (struct gdbarch *gdbarch);
void set_gdbarch_short_bit (struct gdbarch *gdbarch, int short_bit);

/* Number of bits in an int or unsigned int for the target machine. */

int gdbarch_int_bit (struct gdbarch *gdbarch);
void set_gdbarch_int_bit (struct gdbarch *gdbarch, int int_bit);

/* Number of bits in a long or unsigned long for the target machine. */

int gdbarch_long_bit (struct gdbarch *gdbarch);
void set_gdbarch_long_bit (struct gdbarch *gdbarch, int long_bit);

/* Number of bits in a long long or unsigned long long for the target
   machine. */

int gdbarch_long_long_bit (struct gdbarch *gdbarch);
void set_gdbarch_long_long_bit (struct gdbarch *gdbarch, int long_long_bit);

/* The ABI default bit-size and format for "bfloat16", "half", "float", "double", and
   "long double".  These bit/format pairs should eventually be combined
   into a single object.  For the moment, just initialize them as a pair.
   Each format describes both the big and little endian layouts (if
   useful). */

int gdbarch_bfloat16_bit (struct gdbarch *gdbarch);
void set_gdbarch_bfloat16_bit (struct gdbarch *gdbarch, int bfloat16_bit);

const struct floatformat **gdbarch_bfloat16_format (struct gdbarch *gdbarch);
void set_gdbarch_bfloat16_format (struct gdbarch *gdbarch, const struct floatformat **bfloat16_format);

int gdbarch_half_bit (struct gdbarch *gdbarch);
void set_gdbarch_half_bit (struct gdbarch *gdbarch, int half_bit);

const struct floatformat **gdbarch_half_format (struct gdbarch *gdbarch);
void set_gdbarch_half_format (struct gdbarch *gdbarch, const struct floatformat **half_format);

int gdbarch_float_bit (struct gdbarch *gdbarch);
void set_gdbarch_float_bit (struct gdbarch *gdbarch, int float_bit);

const struct floatformat **gdbarch_float_format (struct gdbarch *gdbarch);
void set_gdbarch_float_format (struct gdbarch *gdbarch, const struct floatformat **float_format);

int gdbarch_double_bit (struct gdbarch *gdbarch);
void set_gdbarch_double_bit (struct gdbarch *gdbarch, int double_bit);

const struct floatformat **gdbarch_double_format (struct gdbarch *gdbarch);
void set_gdbarch_double_format (struct gdbarch *gdbarch, const struct floatformat **double_format);

int gdbarch_long_double_bit (struct gdbarch *gdbarch);
void set_gdbarch_long_double_bit (struct gdbarch *gdbarch, int long_double_bit);

const struct floatformat **gdbarch_long_double_format (struct gdbarch *gdbarch);
void set_gdbarch_long_double_format (struct gdbarch *gdbarch, const struct floatformat **long_double_format);

/* The ABI default bit-size for "wchar_t".  wchar_t is a built-in type
   starting with C++11. */

int gdbarch_wchar_bit (struct gdbarch *gdbarch);
void set_gdbarch_wchar_bit (struct gdbarch *gdbarch, int wchar_bit);

/* True if `wchar_t' is signed, false if unsigned.

   The default value is true (signed). */

bool gdbarch_wchar_signed (struct gdbarch *gdbarch);
void set_gdbarch_wchar_signed (struct gdbarch *gdbarch, bool wchar_signed);

/* Returns the floating-point format to be used for values of length LENGTH.
   NAME, if non-NULL, is the type name, which may be used to distinguish
   different target formats of the same length. */

using gdbarch_floatformat_for_type_ftype = const struct floatformat **(struct gdbarch *gdbarch, const char *name, int length);
const struct floatformat **gdbarch_floatformat_for_type (struct gdbarch *gdbarch, const char *name, int length);
void set_gdbarch_floatformat_for_type (struct gdbarch *gdbarch, gdbarch_floatformat_for_type_ftype *floatformat_for_type);

/* For most targets, a pointer on the target and its representation as an
   address in GDB have the same size and "look the same".  For such a
   target, you need only set gdbarch_ptr_bit and gdbarch_addr_bit
   / addr_bit will be set from it.

   If gdbarch_ptr_bit and gdbarch_addr_bit are different, you'll probably
   also need to set gdbarch_dwarf2_addr_size, gdbarch_pointer_to_address and
   gdbarch_address_to_pointer as well.

   ptr_bit is the size of a pointer on the target */

int gdbarch_ptr_bit (struct gdbarch *gdbarch);
void set_gdbarch_ptr_bit (struct gdbarch *gdbarch, int ptr_bit);

/* addr_bit is the size of a target address as represented in gdb */

int gdbarch_addr_bit (struct gdbarch *gdbarch);
void set_gdbarch_addr_bit (struct gdbarch *gdbarch, int addr_bit);

/* dwarf2_addr_size is the target address size as used in the Dwarf debug
   info.  For .debug_frame FDEs, this is supposed to be the target address
   size from the associated CU header, and which is equivalent to the
   DWARF2_ADDR_SIZE as defined by the target specific GCC back-end.
   Unfortunately there is no good way to determine this value.  Therefore
   dwarf2_addr_size simply defaults to the target pointer size.

   dwarf2_addr_size is not used for .eh_frame FDEs, which are generally
   defined using the target's pointer size so far.

   Note that dwarf2_addr_size only needs to be redefined by a target if the
   GCC back-end defines a DWARF2_ADDR_SIZE other than the target pointer size,
   and if Dwarf versions < 4 need to be supported. */

int gdbarch_dwarf2_addr_size (struct gdbarch *gdbarch);
void set_gdbarch_dwarf2_addr_size (struct gdbarch *gdbarch, int dwarf2_addr_size);

/* True if `char' acts like `signed char', false if `unsigned char'.

   The default value is true (signed). */

bool gdbarch_char_signed (struct gdbarch *gdbarch);
void set_gdbarch_char_signed (struct gdbarch *gdbarch, bool char_signed);

bool gdbarch_read_pc_p (struct gdbarch *gdbarch);

using gdbarch_read_pc_ftype = CORE_ADDR (readable_regcache *regcache);
CORE_ADDR gdbarch_read_pc (struct gdbarch *gdbarch, readable_regcache *regcache);
void set_gdbarch_read_pc (struct gdbarch *gdbarch, gdbarch_read_pc_ftype *read_pc);

bool gdbarch_write_pc_p (struct gdbarch *gdbarch);

using gdbarch_write_pc_ftype = void (struct regcache *regcache, CORE_ADDR val);
void gdbarch_write_pc (struct gdbarch *gdbarch, struct regcache *regcache, CORE_ADDR val);
void set_gdbarch_write_pc (struct gdbarch *gdbarch, gdbarch_write_pc_ftype *write_pc);

/* Function for getting target's idea of a frame pointer.  FIXME: GDB's
   whole scheme for dealing with "frames" and "frame pointers" needs a
   serious shakedown. */

using gdbarch_virtual_frame_pointer_ftype = void (struct gdbarch *gdbarch, CORE_ADDR pc, int *frame_regnum, LONGEST *frame_offset);
void gdbarch_virtual_frame_pointer (struct gdbarch *gdbarch, CORE_ADDR pc, int *frame_regnum, LONGEST *frame_offset);
void set_gdbarch_virtual_frame_pointer (struct gdbarch *gdbarch, gdbarch_virtual_frame_pointer_ftype *virtual_frame_pointer);

bool gdbarch_pseudo_register_read_p (struct gdbarch *gdbarch);

using gdbarch_pseudo_register_read_ftype = enum register_status (struct gdbarch *gdbarch, readable_regcache *regcache, int cookednum, gdb_byte *buf);
enum register_status gdbarch_pseudo_register_read (struct gdbarch *gdbarch, readable_regcache *regcache, int cookednum, gdb_byte *buf);
void set_gdbarch_pseudo_register_read (struct gdbarch *gdbarch, gdbarch_pseudo_register_read_ftype *pseudo_register_read);

/* Read a register into a new struct value.  If the register is wholly
   or partly unavailable, this should call mark_value_bytes_unavailable
   as appropriate.  If this is defined, then pseudo_register_read will
   never be called. */

bool gdbarch_pseudo_register_read_value_p (struct gdbarch *gdbarch);

using gdbarch_pseudo_register_read_value_ftype = struct value *(struct gdbarch *gdbarch, const frame_info_ptr &next_frame, int cookednum);
struct value *gdbarch_pseudo_register_read_value (struct gdbarch *gdbarch, const frame_info_ptr &next_frame, int cookednum);
void set_gdbarch_pseudo_register_read_value (struct gdbarch *gdbarch, gdbarch_pseudo_register_read_value_ftype *pseudo_register_read_value);

/* Write bytes in BUF to pseudo register with number PSEUDO_REG_NUM.

   Raw registers backing the pseudo register should be written to using
   NEXT_FRAME. */

bool gdbarch_pseudo_register_write_p (struct gdbarch *gdbarch);

using gdbarch_pseudo_register_write_ftype = void (struct gdbarch *gdbarch, const frame_info_ptr &next_frame, int pseudo_reg_num, gdb::array_view<const gdb_byte> buf);
void gdbarch_pseudo_register_write (struct gdbarch *gdbarch, const frame_info_ptr &next_frame, int pseudo_reg_num, gdb::array_view<const gdb_byte> buf);
void set_gdbarch_pseudo_register_write (struct gdbarch *gdbarch, gdbarch_pseudo_register_write_ftype *pseudo_register_write);

/* Write bytes to a pseudo register.

   This is marked as deprecated because it gets passed a regcache for
   implementations to write raw registers in.  This doesn't work for unwound
   frames, where the raw registers backing the pseudo registers may have been
   saved elsewhere.

   Implementations should be migrated to implement pseudo_register_write instead. */

using gdbarch_deprecated_pseudo_register_write_ftype = void (struct gdbarch *gdbarch, struct regcache *regcache, int cookednum, const gdb_byte *buf);
void gdbarch_deprecated_pseudo_register_write (struct gdbarch *gdbarch, struct regcache *regcache, int cookednum, const gdb_byte *buf);
void set_gdbarch_deprecated_pseudo_register_write (struct gdbarch *gdbarch, gdbarch_deprecated_pseudo_register_write_ftype *deprecated_pseudo_register_write);

int gdbarch_num_regs (struct gdbarch *gdbarch);
void set_gdbarch_num_regs (struct gdbarch *gdbarch, int num_regs);

/* This macro gives the number of pseudo-registers that live in the
   register namespace but do not get fetched or stored on the target.
   These pseudo-registers may be aliases for other registers,
   combinations of other registers, or they may be computed by GDB. */

int gdbarch_num_pseudo_regs (struct gdbarch *gdbarch);
void set_gdbarch_num_pseudo_regs (struct gdbarch *gdbarch, int num_pseudo_regs);

/* Assemble agent expression bytecode to collect pseudo-register REG.
   REG must be a valid register number. */

bool gdbarch_ax_pseudo_register_collect_p (struct gdbarch *gdbarch);

using gdbarch_ax_pseudo_register_collect_ftype = void (struct gdbarch *gdbarch, struct agent_expr *ax, int reg);
void gdbarch_ax_pseudo_register_collect (struct gdbarch *gdbarch, struct agent_expr *ax, int reg);
void set_gdbarch_ax_pseudo_register_collect (struct gdbarch *gdbarch, gdbarch_ax_pseudo_register_collect_ftype *ax_pseudo_register_collect);

/* Assemble agent expression bytecode to push the value of pseudo-register
   REG on the interpreter stack.
   REG must be a valid register number.
   Return false if something goes wrong, true otherwise. */

bool gdbarch_ax_pseudo_register_push_stack_p (struct gdbarch *gdbarch);

using gdbarch_ax_pseudo_register_push_stack_ftype = bool (struct gdbarch *gdbarch, struct agent_expr *ax, int reg);
bool gdbarch_ax_pseudo_register_push_stack (struct gdbarch *gdbarch, struct agent_expr *ax, int reg);
void set_gdbarch_ax_pseudo_register_push_stack (struct gdbarch *gdbarch, gdbarch_ax_pseudo_register_push_stack_ftype *ax_pseudo_register_push_stack);

/* Some architectures can display additional information for specific
   signals.
   UIOUT is the output stream where the handler will place information. */

bool gdbarch_report_signal_info_p (struct gdbarch *gdbarch);

using gdbarch_report_signal_info_ftype = void (struct gdbarch *gdbarch, struct ui_out *uiout, enum gdb_signal siggnal);
void gdbarch_report_signal_info (struct gdbarch *gdbarch, struct ui_out *uiout, enum gdb_signal siggnal);
void set_gdbarch_report_signal_info (struct gdbarch *gdbarch, gdbarch_report_signal_info_ftype *report_signal_info);

/* GDB's standard (or well known) register numbers.  These can map onto
   a real register or a pseudo (computed) register or not be defined at
   all (-1).
   gdbarch_sp_regnum will hopefully be replaced by UNWIND_SP. */

int gdbarch_sp_regnum (struct gdbarch *gdbarch);
void set_gdbarch_sp_regnum (struct gdbarch *gdbarch, int sp_regnum);

int gdbarch_pc_regnum (struct gdbarch *gdbarch);
void set_gdbarch_pc_regnum (struct gdbarch *gdbarch, int pc_regnum);

int gdbarch_ps_regnum (struct gdbarch *gdbarch);
void set_gdbarch_ps_regnum (struct gdbarch *gdbarch, int ps_regnum);

int gdbarch_fp0_regnum (struct gdbarch *gdbarch);
void set_gdbarch_fp0_regnum (struct gdbarch *gdbarch, int fp0_regnum);

/* Provide a default mapping from a DWARF2 register number to a gdb REGNUM.
   Return -1 for bad REGNUM.  Note: Several targets get this wrong. */

using gdbarch_dwarf2_reg_to_regnum_ftype = int (struct gdbarch *gdbarch, int dwarf2_regnr);
int gdbarch_dwarf2_reg_to_regnum (struct gdbarch *gdbarch, int dwarf2_regnr);
void set_gdbarch_dwarf2_reg_to_regnum (struct gdbarch *gdbarch, gdbarch_dwarf2_reg_to_regnum_ftype *dwarf2_reg_to_regnum);

/* Return the name of register REGNR for the specified architecture.
   REGNR can be any value greater than, or equal to zero, and less than
   'gdbarch_num_cooked_regs (GDBARCH)'.  If REGNR is not supported for
   GDBARCH, then this function will return an empty string, this function
   should never return nullptr. */

using gdbarch_register_name_ftype = const char *(struct gdbarch *gdbarch, int regnr);
const char *gdbarch_register_name (struct gdbarch *gdbarch, int regnr);
void set_gdbarch_register_name (struct gdbarch *gdbarch, gdbarch_register_name_ftype *register_name);

/* Return the type of a register specified by the architecture.  Only
   the register cache should call this function directly; others should
   use "register_type". */

using gdbarch_register_type_ftype = struct type *(struct gdbarch *gdbarch, int reg_nr);
struct type *gdbarch_register_type (struct gdbarch *gdbarch, int reg_nr);
void set_gdbarch_register_type (struct gdbarch *gdbarch, gdbarch_register_type_ftype *register_type);

/* Generate a dummy frame_id for THIS_FRAME assuming that the frame is
   a dummy frame.  A dummy frame is created before an inferior call,
   the frame_id returned here must match the frame_id that was built
   for the inferior call.  Usually this means the returned frame_id's
   stack address should match the address returned by
   gdbarch_push_dummy_call, and the returned frame_id's code address
   should match the address at which the breakpoint was set in the dummy
   frame. */

using gdbarch_dummy_id_ftype = struct frame_id (struct gdbarch *gdbarch, const frame_info_ptr &this_frame);
struct frame_id gdbarch_dummy_id (struct gdbarch *gdbarch, const frame_info_ptr &this_frame);
void set_gdbarch_dummy_id (struct gdbarch *gdbarch, gdbarch_dummy_id_ftype *dummy_id);

/* Implement DUMMY_ID and PUSH_DUMMY_CALL, then delete
   deprecated_fp_regnum. */

int gdbarch_deprecated_fp_regnum (struct gdbarch *gdbarch);
void set_gdbarch_deprecated_fp_regnum (struct gdbarch *gdbarch, int deprecated_fp_regnum);

bool gdbarch_push_dummy_call_p (struct gdbarch *gdbarch);

using gdbarch_push_dummy_call_ftype = CORE_ADDR (struct gdbarch *gdbarch, struct value *function, struct regcache *regcache, CORE_ADDR bp_addr, int nargs, struct value **args, CORE_ADDR sp, function_call_return_method return_method, CORE_ADDR struct_addr);
CORE_ADDR gdbarch_push_dummy_call (struct gdbarch *gdbarch, struct value *function, struct regcache *regcache, CORE_ADDR bp_addr, int nargs, struct value **args, CORE_ADDR sp, function_call_return_method return_method, CORE_ADDR struct_addr);
void set_gdbarch_push_dummy_call (struct gdbarch *gdbarch, gdbarch_push_dummy_call_ftype *push_dummy_call);

enum call_dummy_location_type gdbarch_call_dummy_location (struct gdbarch *gdbarch);
void set_gdbarch_call_dummy_location (struct gdbarch *gdbarch, enum call_dummy_location_type call_dummy_location);

bool gdbarch_push_dummy_code_p (struct gdbarch *gdbarch);

using gdbarch_push_dummy_code_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR sp, CORE_ADDR funaddr, struct value **args, int nargs, struct type *value_type, CORE_ADDR *real_pc, CORE_ADDR *bp_addr, struct regcache *regcache);
CORE_ADDR gdbarch_push_dummy_code (struct gdbarch *gdbarch, CORE_ADDR sp, CORE_ADDR funaddr, struct value **args, int nargs, struct type *value_type, CORE_ADDR *real_pc, CORE_ADDR *bp_addr, struct regcache *regcache);
void set_gdbarch_push_dummy_code (struct gdbarch *gdbarch, gdbarch_push_dummy_code_ftype *push_dummy_code);

/* Return true if the code of FRAME is writable. */

using gdbarch_code_of_frame_writable_ftype = bool (struct gdbarch *gdbarch, const frame_info_ptr &frame);
bool gdbarch_code_of_frame_writable (struct gdbarch *gdbarch, const frame_info_ptr &frame);
void set_gdbarch_code_of_frame_writable (struct gdbarch *gdbarch, gdbarch_code_of_frame_writable_ftype *code_of_frame_writable);

using gdbarch_print_registers_info_ftype = void (struct gdbarch *gdbarch, struct ui_file *file, const frame_info_ptr &frame, int regnum, bool all);
void gdbarch_print_registers_info (struct gdbarch *gdbarch, struct ui_file *file, const frame_info_ptr &frame, int regnum, bool all);
void set_gdbarch_print_registers_info (struct gdbarch *gdbarch, gdbarch_print_registers_info_ftype *print_registers_info);

using gdbarch_print_float_info_ftype = void (struct gdbarch *gdbarch, struct ui_file *file, const frame_info_ptr &frame, const char *args);
void gdbarch_print_float_info (struct gdbarch *gdbarch, struct ui_file *file, const frame_info_ptr &frame, const char *args);
void set_gdbarch_print_float_info (struct gdbarch *gdbarch, gdbarch_print_float_info_ftype *print_float_info);

/* MAP a GDB RAW register number onto a simulator register number.  See
   also include/...-sim.h. */

using gdbarch_register_sim_regno_ftype = int (struct gdbarch *gdbarch, int reg_nr);
int gdbarch_register_sim_regno (struct gdbarch *gdbarch, int reg_nr);
void set_gdbarch_register_sim_regno (struct gdbarch *gdbarch, gdbarch_register_sim_regno_ftype *register_sim_regno);

using gdbarch_cannot_fetch_register_ftype = bool (struct gdbarch *gdbarch, int regnum);
bool gdbarch_cannot_fetch_register (struct gdbarch *gdbarch, int regnum);
void set_gdbarch_cannot_fetch_register (struct gdbarch *gdbarch, gdbarch_cannot_fetch_register_ftype *cannot_fetch_register);

using gdbarch_cannot_store_register_ftype = bool (struct gdbarch *gdbarch, int regnum);
bool gdbarch_cannot_store_register (struct gdbarch *gdbarch, int regnum);
void set_gdbarch_cannot_store_register (struct gdbarch *gdbarch, gdbarch_cannot_store_register_ftype *cannot_store_register);

/* Determine the address where a longjmp will land and save this address
   in PC.  Return true on success.

   FRAME corresponds to the longjmp frame. */

bool gdbarch_get_longjmp_target_p (struct gdbarch *gdbarch);

using gdbarch_get_longjmp_target_ftype = bool (const frame_info_ptr &frame, CORE_ADDR *pc);
bool gdbarch_get_longjmp_target (struct gdbarch *gdbarch, const frame_info_ptr &frame, CORE_ADDR *pc);
void set_gdbarch_get_longjmp_target (struct gdbarch *gdbarch, gdbarch_get_longjmp_target_ftype *get_longjmp_target);

using gdbarch_convert_register_p_ftype = bool (struct gdbarch *gdbarch, int regnum, struct type *type);
bool gdbarch_convert_register_p (struct gdbarch *gdbarch, int regnum, struct type *type);
void set_gdbarch_convert_register_p (struct gdbarch *gdbarch, gdbarch_convert_register_p_ftype *convert_register_p);

using gdbarch_register_to_value_ftype = bool (const frame_info_ptr &frame, int regnum, struct type *type, gdb_byte *buf, bool *optimizedp, bool *unavailablep);
bool gdbarch_register_to_value (struct gdbarch *gdbarch, const frame_info_ptr &frame, int regnum, struct type *type, gdb_byte *buf, bool *optimizedp, bool *unavailablep);
void set_gdbarch_register_to_value (struct gdbarch *gdbarch, gdbarch_register_to_value_ftype *register_to_value);

using gdbarch_value_to_register_ftype = void (const frame_info_ptr &frame, int regnum, struct type *type, const gdb_byte *buf);
void gdbarch_value_to_register (struct gdbarch *gdbarch, const frame_info_ptr &frame, int regnum, struct type *type, const gdb_byte *buf);
void set_gdbarch_value_to_register (struct gdbarch *gdbarch, gdbarch_value_to_register_ftype *value_to_register);

/* Construct a value representing the contents of register REGNUM in
   frame THIS_FRAME, interpreted as type TYPE.  The routine needs to
   allocate and return a struct value with all value attributes
   (but not the value contents) filled in. */

using gdbarch_value_from_register_ftype = struct value *(struct gdbarch *gdbarch, struct type *type, int regnum, const frame_info_ptr &this_frame);
struct value *gdbarch_value_from_register (struct gdbarch *gdbarch, struct type *type, int regnum, const frame_info_ptr &this_frame);
void set_gdbarch_value_from_register (struct gdbarch *gdbarch, gdbarch_value_from_register_ftype *value_from_register);

/* For a DW_OP_piece located in a register, but not occupying the
   entire register, return the placement of the piece within that
   register as defined by the ABI. */

using gdbarch_dwarf2_reg_piece_offset_ftype = ULONGEST (struct gdbarch *gdbarch, int regnum, ULONGEST size);
ULONGEST gdbarch_dwarf2_reg_piece_offset (struct gdbarch *gdbarch, int regnum, ULONGEST size);
void set_gdbarch_dwarf2_reg_piece_offset (struct gdbarch *gdbarch, gdbarch_dwarf2_reg_piece_offset_ftype *dwarf2_reg_piece_offset);

using gdbarch_pointer_to_address_ftype = CORE_ADDR (struct gdbarch *gdbarch, struct type *type, const gdb_byte *buf);
CORE_ADDR gdbarch_pointer_to_address (struct gdbarch *gdbarch, struct type *type, const gdb_byte *buf);
void set_gdbarch_pointer_to_address (struct gdbarch *gdbarch, gdbarch_pointer_to_address_ftype *pointer_to_address);

using gdbarch_address_to_pointer_ftype = void (struct gdbarch *gdbarch, struct type *type, gdb_byte *buf, CORE_ADDR addr);
void gdbarch_address_to_pointer (struct gdbarch *gdbarch, struct type *type, gdb_byte *buf, CORE_ADDR addr);
void set_gdbarch_address_to_pointer (struct gdbarch *gdbarch, gdbarch_address_to_pointer_ftype *address_to_pointer);

bool gdbarch_integer_to_address_p (struct gdbarch *gdbarch);

using gdbarch_integer_to_address_ftype = CORE_ADDR (struct gdbarch *gdbarch, struct type *type, const gdb_byte *buf);
CORE_ADDR gdbarch_integer_to_address (struct gdbarch *gdbarch, struct type *type, const gdb_byte *buf);
void set_gdbarch_integer_to_address (struct gdbarch *gdbarch, gdbarch_integer_to_address_ftype *integer_to_address);

/* Return the return-value convention that will be used by FUNCTION
   to return a value of type VALTYPE.  FUNCTION may be NULL in which
   case the return convention is computed based only on VALTYPE.

   If READBUF is not NULL, extract the return value and save it in this buffer.

   If WRITEBUF is not NULL, it contains a return value which will be
   stored into the appropriate register.  This can be used when we want
   to force the value returned by a function (see the "return" command
   for instance).

   NOTE: it is better to implement return_value_as_value instead, as that
   method can properly handle variably-sized types. */

using gdbarch_return_value_ftype = enum return_value_convention (struct gdbarch *gdbarch, struct value *function, struct type *valtype, struct regcache *regcache, gdb_byte *readbuf, const gdb_byte *writebuf);
void set_gdbarch_return_value (struct gdbarch *gdbarch, gdbarch_return_value_ftype *return_value);

/* Return the return-value convention that will be used by FUNCTION
   to return a value of type VALTYPE.  FUNCTION may be NULL in which
   case the return convention is computed based only on VALTYPE.

   If READ_VALUE is not NULL, extract the return value and save it in
   this pointer.

   If WRITEBUF is not NULL, it contains a return value which will be
   stored into the appropriate register.  This can be used when we want
   to force the value returned by a function (see the "return" command
   for instance). */

using gdbarch_return_value_as_value_ftype = enum return_value_convention (struct gdbarch *gdbarch, struct value *function, struct type *valtype, struct regcache *regcache, struct value **read_value, const gdb_byte *writebuf);
enum return_value_convention gdbarch_return_value_as_value (struct gdbarch *gdbarch, struct value *function, struct type *valtype, struct regcache *regcache, struct value **read_value, const gdb_byte *writebuf);
void set_gdbarch_return_value_as_value (struct gdbarch *gdbarch, gdbarch_return_value_as_value_ftype *return_value_as_value);

/* Return the address at which the value being returned from
   the current function will be stored.  This routine is only
   called if the current function uses the "struct return
   convention".

   May return 0 when unable to determine that address. */

using gdbarch_get_return_buf_addr_ftype = CORE_ADDR (struct type *val_type, const frame_info_ptr &cur_frame);
CORE_ADDR gdbarch_get_return_buf_addr (struct gdbarch *gdbarch, struct type *val_type, const frame_info_ptr &cur_frame);
void set_gdbarch_get_return_buf_addr (struct gdbarch *gdbarch, gdbarch_get_return_buf_addr_ftype *get_return_buf_addr);

/* Return true if the typedef record needs to be replaced.".

   Return 0 by default */

using gdbarch_dwarf2_omit_typedef_p_ftype = bool (struct type *target_type, const char *producer, const char *name);
bool gdbarch_dwarf2_omit_typedef_p (struct gdbarch *gdbarch, struct type *target_type, const char *producer, const char *name);
void set_gdbarch_dwarf2_omit_typedef_p (struct gdbarch *gdbarch, gdbarch_dwarf2_omit_typedef_p_ftype *dwarf2_omit_typedef_p);

/* Update PC when trying to find a call site.  This is useful on
   architectures where the call site PC, as reported in the DWARF, can be
   incorrect for some reason.

   The passed-in PC will be an address in the inferior.  GDB will have
   already failed to find a call site at this PC.  This function may
   simply return its parameter if it thinks that should be the correct
   address. */

using gdbarch_update_call_site_pc_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR pc);
CORE_ADDR gdbarch_update_call_site_pc (struct gdbarch *gdbarch, CORE_ADDR pc);
void set_gdbarch_update_call_site_pc (struct gdbarch *gdbarch, gdbarch_update_call_site_pc_ftype *update_call_site_pc);

/* Return true if the return value of function is stored in the first hidden
   parameter.  In theory, this feature should be language-dependent, specified
   by language and its ABI, such as C++.  Unfortunately, compiler may
   implement it to a target-dependent feature.  So that we need such hook here
   to be aware of this in GDB. */

using gdbarch_return_in_first_hidden_param_p_ftype = bool (struct gdbarch *gdbarch, struct type *type);
bool gdbarch_return_in_first_hidden_param_p (struct gdbarch *gdbarch, struct type *type);
void set_gdbarch_return_in_first_hidden_param_p (struct gdbarch *gdbarch, gdbarch_return_in_first_hidden_param_p_ftype *return_in_first_hidden_param_p);

using gdbarch_skip_prologue_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR ip);
CORE_ADDR gdbarch_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR ip);
void set_gdbarch_skip_prologue (struct gdbarch *gdbarch, gdbarch_skip_prologue_ftype *skip_prologue);

bool gdbarch_skip_main_prologue_p (struct gdbarch *gdbarch);

using gdbarch_skip_main_prologue_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR ip);
CORE_ADDR gdbarch_skip_main_prologue (struct gdbarch *gdbarch, CORE_ADDR ip);
void set_gdbarch_skip_main_prologue (struct gdbarch *gdbarch, gdbarch_skip_main_prologue_ftype *skip_main_prologue);

/* On some platforms, a single function may provide multiple entry points,
   e.g. one that is used for function-pointer calls and a different one
   that is used for direct function calls.
   In order to ensure that breakpoints set on the function will trigger
   no matter via which entry point the function is entered, a platform
   may provide the skip_entrypoint callback.  It is called with IP set
   to the main entry point of a function (as determined by the symbol table),
   and should return the address of the innermost entry point, where the
   actual breakpoint needs to be set.  Note that skip_entrypoint is used
   by GDB common code even when debugging optimized code, where skip_prologue
   is not used. */

bool gdbarch_skip_entrypoint_p (struct gdbarch *gdbarch);

using gdbarch_skip_entrypoint_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR ip);
CORE_ADDR gdbarch_skip_entrypoint (struct gdbarch *gdbarch, CORE_ADDR ip);
void set_gdbarch_skip_entrypoint (struct gdbarch *gdbarch, gdbarch_skip_entrypoint_ftype *skip_entrypoint);

using gdbarch_inner_than_ftype = bool (CORE_ADDR lhs, CORE_ADDR rhs);
bool gdbarch_inner_than (struct gdbarch *gdbarch, CORE_ADDR lhs, CORE_ADDR rhs);
void set_gdbarch_inner_than (struct gdbarch *gdbarch, gdbarch_inner_than_ftype *inner_than);

using gdbarch_breakpoint_from_pc_ftype = const gdb_byte *(struct gdbarch *gdbarch, CORE_ADDR *pcptr, int *lenptr);
const gdb_byte *gdbarch_breakpoint_from_pc (struct gdbarch *gdbarch, CORE_ADDR *pcptr, int *lenptr);
void set_gdbarch_breakpoint_from_pc (struct gdbarch *gdbarch, gdbarch_breakpoint_from_pc_ftype *breakpoint_from_pc);

/* Return the breakpoint kind for this target based on *PCPTR. */

using gdbarch_breakpoint_kind_from_pc_ftype = int (struct gdbarch *gdbarch, CORE_ADDR *pcptr);
int gdbarch_breakpoint_kind_from_pc (struct gdbarch *gdbarch, CORE_ADDR *pcptr);
void set_gdbarch_breakpoint_kind_from_pc (struct gdbarch *gdbarch, gdbarch_breakpoint_kind_from_pc_ftype *breakpoint_kind_from_pc);

/* Return the software breakpoint from KIND.  KIND can have target
   specific meaning like the Z0 kind parameter.
   SIZE is set to the software breakpoint's length in memory. */

using gdbarch_sw_breakpoint_from_kind_ftype = const gdb_byte *(struct gdbarch *gdbarch, int kind, int *size);
const gdb_byte *gdbarch_sw_breakpoint_from_kind (struct gdbarch *gdbarch, int kind, int *size);
void set_gdbarch_sw_breakpoint_from_kind (struct gdbarch *gdbarch, gdbarch_sw_breakpoint_from_kind_ftype *sw_breakpoint_from_kind);

/* Return the breakpoint kind for this target based on the current
   processor state (e.g. the current instruction mode on ARM) and the
   *PCPTR.  In default, it is gdbarch->breakpoint_kind_from_pc. */

using gdbarch_breakpoint_kind_from_current_state_ftype = int (struct gdbarch *gdbarch, struct regcache *regcache, CORE_ADDR *pcptr);
int gdbarch_breakpoint_kind_from_current_state (struct gdbarch *gdbarch, struct regcache *regcache, CORE_ADDR *pcptr);
void set_gdbarch_breakpoint_kind_from_current_state (struct gdbarch *gdbarch, gdbarch_breakpoint_kind_from_current_state_ftype *breakpoint_kind_from_current_state);

bool gdbarch_adjust_breakpoint_address_p (struct gdbarch *gdbarch);

using gdbarch_adjust_breakpoint_address_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR bpaddr);
CORE_ADDR gdbarch_adjust_breakpoint_address (struct gdbarch *gdbarch, CORE_ADDR bpaddr);
void set_gdbarch_adjust_breakpoint_address (struct gdbarch *gdbarch, gdbarch_adjust_breakpoint_address_ftype *adjust_breakpoint_address);

using gdbarch_memory_insert_breakpoint_ftype = int (struct gdbarch *gdbarch, struct bp_target_info *bp_tgt);
int gdbarch_memory_insert_breakpoint (struct gdbarch *gdbarch, struct bp_target_info *bp_tgt);
void set_gdbarch_memory_insert_breakpoint (struct gdbarch *gdbarch, gdbarch_memory_insert_breakpoint_ftype *memory_insert_breakpoint);

using gdbarch_memory_remove_breakpoint_ftype = int (struct gdbarch *gdbarch, struct bp_target_info *bp_tgt);
int gdbarch_memory_remove_breakpoint (struct gdbarch *gdbarch, struct bp_target_info *bp_tgt);
void set_gdbarch_memory_remove_breakpoint (struct gdbarch *gdbarch, gdbarch_memory_remove_breakpoint_ftype *memory_remove_breakpoint);

CORE_ADDR gdbarch_decr_pc_after_break (struct gdbarch *gdbarch);
void set_gdbarch_decr_pc_after_break (struct gdbarch *gdbarch, CORE_ADDR decr_pc_after_break);

/* A function can be addressed by either its "pointer" (possibly a
   descriptor address) or "entry point" (first executable instruction).
   The method "convert_from_func_ptr_addr" converting the former to the
   latter.  gdbarch_deprecated_function_start_offset is being used to implement
   a simplified subset of that functionality - the function's address
   corresponds to the "function pointer" and the function's start
   corresponds to the "function entry point" - and hence is redundant. */

CORE_ADDR gdbarch_deprecated_function_start_offset (struct gdbarch *gdbarch);
void set_gdbarch_deprecated_function_start_offset (struct gdbarch *gdbarch, CORE_ADDR deprecated_function_start_offset);

/* Return the remote protocol register number associated with this
   register.  Normally the identity mapping. */

using gdbarch_remote_register_number_ftype = int (struct gdbarch *gdbarch, int regno);
int gdbarch_remote_register_number (struct gdbarch *gdbarch, int regno);
void set_gdbarch_remote_register_number (struct gdbarch *gdbarch, gdbarch_remote_register_number_ftype *remote_register_number);

/* Fetch the target specific address used to represent a load module. */

bool gdbarch_fetch_tls_load_module_address_p (struct gdbarch *gdbarch);

using gdbarch_fetch_tls_load_module_address_ftype = CORE_ADDR (struct objfile *objfile);
CORE_ADDR gdbarch_fetch_tls_load_module_address (struct gdbarch *gdbarch, struct objfile *objfile);
void set_gdbarch_fetch_tls_load_module_address (struct gdbarch *gdbarch, gdbarch_fetch_tls_load_module_address_ftype *fetch_tls_load_module_address);

/* Return the thread-local address at OFFSET in the thread-local
   storage for the thread PTID and the shared library or executable
   file given by LM_ADDR.  If that block of thread-local storage hasn't
   been allocated yet, this function may throw an error.  LM_ADDR may
   be zero for statically linked multithreaded inferiors. */

bool gdbarch_get_thread_local_address_p (struct gdbarch *gdbarch);

using gdbarch_get_thread_local_address_ftype = CORE_ADDR (struct gdbarch *gdbarch, ptid_t ptid, CORE_ADDR lm_addr, CORE_ADDR offset);
CORE_ADDR gdbarch_get_thread_local_address (struct gdbarch *gdbarch, ptid_t ptid, CORE_ADDR lm_addr, CORE_ADDR offset);
void set_gdbarch_get_thread_local_address (struct gdbarch *gdbarch, gdbarch_get_thread_local_address_ftype *get_thread_local_address);

CORE_ADDR gdbarch_frame_args_skip (struct gdbarch *gdbarch);
void set_gdbarch_frame_args_skip (struct gdbarch *gdbarch, CORE_ADDR frame_args_skip);

using gdbarch_unwind_pc_ftype = CORE_ADDR (struct gdbarch *gdbarch, const frame_info_ptr &next_frame);
CORE_ADDR gdbarch_unwind_pc (struct gdbarch *gdbarch, const frame_info_ptr &next_frame);
void set_gdbarch_unwind_pc (struct gdbarch *gdbarch, gdbarch_unwind_pc_ftype *unwind_pc);

using gdbarch_unwind_sp_ftype = CORE_ADDR (struct gdbarch *gdbarch, const frame_info_ptr &next_frame);
CORE_ADDR gdbarch_unwind_sp (struct gdbarch *gdbarch, const frame_info_ptr &next_frame);
void set_gdbarch_unwind_sp (struct gdbarch *gdbarch, gdbarch_unwind_sp_ftype *unwind_sp);

/* DEPRECATED_FRAME_LOCALS_ADDRESS as been replaced by the per-frame
   frame-base.  Enable frame-base before frame-unwind. */

bool gdbarch_frame_num_args_p (struct gdbarch *gdbarch);

using gdbarch_frame_num_args_ftype = int (const frame_info_ptr &frame);
int gdbarch_frame_num_args (struct gdbarch *gdbarch, const frame_info_ptr &frame);
void set_gdbarch_frame_num_args (struct gdbarch *gdbarch, gdbarch_frame_num_args_ftype *frame_num_args);

bool gdbarch_frame_align_p (struct gdbarch *gdbarch);

using gdbarch_frame_align_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR address);
CORE_ADDR gdbarch_frame_align (struct gdbarch *gdbarch, CORE_ADDR address);
void set_gdbarch_frame_align (struct gdbarch *gdbarch, gdbarch_frame_align_ftype *frame_align);

int gdbarch_frame_red_zone_size (struct gdbarch *gdbarch);
void set_gdbarch_frame_red_zone_size (struct gdbarch *gdbarch, int frame_red_zone_size);

using gdbarch_convert_from_func_ptr_addr_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR addr, struct target_ops *targ);
CORE_ADDR gdbarch_convert_from_func_ptr_addr (struct gdbarch *gdbarch, CORE_ADDR addr, struct target_ops *targ);
void set_gdbarch_convert_from_func_ptr_addr (struct gdbarch *gdbarch, gdbarch_convert_from_func_ptr_addr_ftype *convert_from_func_ptr_addr);

/* On some machines there are bits in addresses which are not really
   part of the address, but are used by the kernel, the hardware, etc.
   for special purposes.  gdbarch_addr_bits_remove takes out any such bits so
   we get a "real" address such as one would find in a symbol table.
   This is used only for addresses of instructions, and even then I'm
   not sure it's used in all contexts.  It exists to deal with there
   being a few stray bits in the PC which would mislead us, not as some
   sort of generic thing to handle alignment or segmentation (it's
   possible it should be in TARGET_READ_PC instead). */

using gdbarch_addr_bits_remove_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR addr);
CORE_ADDR gdbarch_addr_bits_remove (struct gdbarch *gdbarch, CORE_ADDR addr);
void set_gdbarch_addr_bits_remove (struct gdbarch *gdbarch, gdbarch_addr_bits_remove_ftype *addr_bits_remove);

/* On some architectures, not all bits of a pointer are significant.
   On AArch64 and amd64, for example, the top bits of a pointer may carry a
   "tag", which can be ignored by the kernel and the hardware.  The "tag" can be
   regarded as additional data associated with the pointer, but it is not part
   of the address.

   Given a pointer for the architecture, this hook removes all the
   non-significant bits and sign-extends things as needed.  It gets used to
   remove non-address bits from pointers used for watchpoints. */

using gdbarch_remove_non_address_bits_watchpoint_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR pointer);
CORE_ADDR gdbarch_remove_non_address_bits_watchpoint (struct gdbarch *gdbarch, CORE_ADDR pointer);
void set_gdbarch_remove_non_address_bits_watchpoint (struct gdbarch *gdbarch, gdbarch_remove_non_address_bits_watchpoint_ftype *remove_non_address_bits_watchpoint);

/* On some architectures, not all bits of a pointer are significant.
   On AArch64 and amd64, for example, the top bits of a pointer may carry a
   "tag", which can be ignored by the kernel and the hardware.  The "tag" can be
   regarded as additional data associated with the pointer, but it is not part
   of the address.

   Given a pointer for the architecture, this hook removes all the
   non-significant bits and sign-extends things as needed.  It gets used to
   remove non-address bits from pointers used for breakpoints. */

using gdbarch_remove_non_address_bits_breakpoint_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR pointer);
CORE_ADDR gdbarch_remove_non_address_bits_breakpoint (struct gdbarch *gdbarch, CORE_ADDR pointer);
void set_gdbarch_remove_non_address_bits_breakpoint (struct gdbarch *gdbarch, gdbarch_remove_non_address_bits_breakpoint_ftype *remove_non_address_bits_breakpoint);

/* On some architectures, not all bits of a pointer are significant.
   On AArch64 and amd64, for example, the top bits of a pointer may carry a
   "tag", which can be ignored by the kernel and the hardware.  The "tag" can be
   regarded as additional data associated with the pointer, but it is not part
   of the address.

   Given a pointer for the architecture, this hook removes all the
   non-significant bits and sign-extends things as needed.  It gets used to
   remove non-address bits from any pointer used to access memory. */

using gdbarch_remove_non_address_bits_memory_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR pointer);
CORE_ADDR gdbarch_remove_non_address_bits_memory (struct gdbarch *gdbarch, CORE_ADDR pointer);
void set_gdbarch_remove_non_address_bits_memory (struct gdbarch *gdbarch, gdbarch_remove_non_address_bits_memory_ftype *remove_non_address_bits_memory);

/* Return a string representation of the memory tag TAG. */

using gdbarch_memtag_to_string_ftype = std::string (struct gdbarch *gdbarch, struct value *tag);
std::string gdbarch_memtag_to_string (struct gdbarch *gdbarch, struct value *tag);
void set_gdbarch_memtag_to_string (struct gdbarch *gdbarch, gdbarch_memtag_to_string_ftype *memtag_to_string);

/* Return true if ADDRESS contains a tag and false otherwise.  ADDRESS
   must be either a pointer or a reference type. */

using gdbarch_tagged_address_p_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR address);
bool gdbarch_tagged_address_p (struct gdbarch *gdbarch, CORE_ADDR address);
void set_gdbarch_tagged_address_p (struct gdbarch *gdbarch, gdbarch_tagged_address_p_ftype *tagged_address_p);

/* Return true if the tag from ADDRESS matches the memory tag for that
   particular address.  Return false otherwise. */

using gdbarch_memtag_matches_p_ftype = bool (struct gdbarch *gdbarch, struct value *address);
bool gdbarch_memtag_matches_p (struct gdbarch *gdbarch, struct value *address);
void set_gdbarch_memtag_matches_p (struct gdbarch *gdbarch, gdbarch_memtag_matches_p_ftype *memtag_matches_p);

/* Set the tags of type TAG_TYPE, for the memory address range
   [ADDRESS, ADDRESS + LENGTH) to TAGS.
   Return true if successful and false otherwise. */

using gdbarch_set_memtags_ftype = bool (struct gdbarch *gdbarch, struct value *address, size_t length, const gdb::byte_vector &tags, memtag_type tag_type);
bool gdbarch_set_memtags (struct gdbarch *gdbarch, struct value *address, size_t length, const gdb::byte_vector &tags, memtag_type tag_type);
void set_gdbarch_set_memtags (struct gdbarch *gdbarch, gdbarch_set_memtags_ftype *set_memtags);

/* Return the tag of type TAG_TYPE associated with the memory address ADDRESS,
   assuming ADDRESS is tagged. */

using gdbarch_get_memtag_ftype = struct value *(struct gdbarch *gdbarch, struct value *address, memtag_type tag_type);
struct value *gdbarch_get_memtag (struct gdbarch *gdbarch, struct value *address, memtag_type tag_type);
void set_gdbarch_get_memtag (struct gdbarch *gdbarch, gdbarch_get_memtag_ftype *get_memtag);

/* memtag_granule_size is the size of the allocation tag granule, for
   architectures that support memory tagging.
   This is 0 for architectures that do not support memory tagging.
   For a non-zero value, this represents the number of bytes of memory per tag. */

CORE_ADDR gdbarch_memtag_granule_size (struct gdbarch *gdbarch);
void set_gdbarch_memtag_granule_size (struct gdbarch *gdbarch, CORE_ADDR memtag_granule_size);

/* FIXME/cagney/2001-01-18: This should be split in two.  A target method that
   indicates if the target needs software single step.  An ISA method to
   implement it.

   FIXME/cagney/2001-01-18: The logic is backwards.  It should be asking if the
   target can single step.  If not, then implement single step using breakpoints.

   Return a vector of addresses on which the software single step
   breakpoints should be inserted.  NULL means software single step is
   not used.
   Multiple breakpoints may be inserted for some instructions such as
   conditional branch.  However, each implementation must always evaluate
   the condition and only put the breakpoint at the branch destination if
   the condition is true, so that we ensure forward progress when stepping
   past a conditional branch to self. */

bool gdbarch_get_next_pcs_p (struct gdbarch *gdbarch);

using gdbarch_get_next_pcs_ftype = std::vector<CORE_ADDR> (struct regcache *regcache);
std::vector<CORE_ADDR> gdbarch_get_next_pcs (struct gdbarch *gdbarch, struct regcache *regcache);
void set_gdbarch_get_next_pcs (struct gdbarch *gdbarch, gdbarch_get_next_pcs_ftype *get_next_pcs);

/* Return true if the processor is executing a delay slot and a
   further single-step is needed before the instruction finishes. */

bool gdbarch_single_step_through_delay_p (struct gdbarch *gdbarch);

using gdbarch_single_step_through_delay_ftype = bool (struct gdbarch *gdbarch, const frame_info_ptr &frame);
bool gdbarch_single_step_through_delay (struct gdbarch *gdbarch, const frame_info_ptr &frame);
void set_gdbarch_single_step_through_delay (struct gdbarch *gdbarch, gdbarch_single_step_through_delay_ftype *single_step_through_delay);

/* FIXME: cagney/2003-08-28: Need to find a better way of selecting the
   disassembler.  Perhaps objdump can handle it? */

using gdbarch_print_insn_ftype = int (bfd_vma vma, struct disassemble_info *info);
int gdbarch_print_insn (struct gdbarch *gdbarch, bfd_vma vma, struct disassemble_info *info);
void set_gdbarch_print_insn (struct gdbarch *gdbarch, gdbarch_print_insn_ftype *print_insn);

using gdbarch_skip_trampoline_code_ftype = CORE_ADDR (const frame_info_ptr &frame, CORE_ADDR pc);
CORE_ADDR gdbarch_skip_trampoline_code (struct gdbarch *gdbarch, const frame_info_ptr &frame, CORE_ADDR pc);
void set_gdbarch_skip_trampoline_code (struct gdbarch *gdbarch, gdbarch_skip_trampoline_code_ftype *skip_trampoline_code);

/* Return a newly-allocated solib_ops object capable of providing the solibs for this architecture. */

using gdbarch_make_solib_ops_ftype = solib_ops_up (program_space *pspace);
solib_ops_up gdbarch_make_solib_ops (struct gdbarch *gdbarch, program_space *pspace);
void set_gdbarch_make_solib_ops (struct gdbarch *gdbarch, gdbarch_make_solib_ops_ftype *make_solib_ops);

/* If in_solib_dynsym_resolve_code() returns true, and SKIP_SOLIB_RESOLVER
   evaluates non-zero, this is the address where the debugger will place
   a step-resume breakpoint to get us past the dynamic linker. */

using gdbarch_skip_solib_resolver_ftype = CORE_ADDR (struct gdbarch *gdbarch, CORE_ADDR pc);
CORE_ADDR gdbarch_skip_solib_resolver (struct gdbarch *gdbarch, CORE_ADDR pc);
void set_gdbarch_skip_solib_resolver (struct gdbarch *gdbarch, gdbarch_skip_solib_resolver_ftype *skip_solib_resolver);

/* Some systems also have trampoline code for returning from shared libs. */

using gdbarch_in_solib_return_trampoline_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR pc, const char *name);
bool gdbarch_in_solib_return_trampoline (struct gdbarch *gdbarch, CORE_ADDR pc, const char *name);
void set_gdbarch_in_solib_return_trampoline (struct gdbarch *gdbarch, gdbarch_in_solib_return_trampoline_ftype *in_solib_return_trampoline);

/* Return true if PC lies inside an indirect branch thunk. */

using gdbarch_in_indirect_branch_thunk_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR pc);
bool gdbarch_in_indirect_branch_thunk (struct gdbarch *gdbarch, CORE_ADDR pc);
void set_gdbarch_in_indirect_branch_thunk (struct gdbarch *gdbarch, gdbarch_in_indirect_branch_thunk_ftype *in_indirect_branch_thunk);

/* A target might have problems with watchpoints as soon as the stack
   frame of the current function has been destroyed.  This mostly happens
   as the first action in a function's epilogue.  stack_frame_destroyed_p()
   is defined to return true if either the given addr is one
   instruction after the stack destroying instruction up to the trailing
   return instruction or if we can figure out that the stack frame has
   already been invalidated regardless of the value of addr.  Targets
   which don't suffer from that problem could just let this functionality
   untouched. */

using gdbarch_stack_frame_destroyed_p_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR addr);
bool gdbarch_stack_frame_destroyed_p (struct gdbarch *gdbarch, CORE_ADDR addr);
void set_gdbarch_stack_frame_destroyed_p (struct gdbarch *gdbarch, gdbarch_stack_frame_destroyed_p_ftype *stack_frame_destroyed_p);

/* Process an ELF symbol in the minimal symbol table in a backend-specific
   way.  Normally this hook is supposed to do nothing, however if required,
   then this hook can be used to apply transformations to symbols that are
   considered special in some way.  For example the MIPS backend uses it
   to interpret `st_other' information to mark compressed code symbols so
   that they can be treated in the appropriate manner in the processing of
   the main symbol table and DWARF-2 records. */

bool gdbarch_elf_make_msymbol_special_p (struct gdbarch *gdbarch);

using gdbarch_elf_make_msymbol_special_ftype = void (const asymbol *sym, struct minimal_symbol *msym);
void gdbarch_elf_make_msymbol_special (struct gdbarch *gdbarch, const asymbol *sym, struct minimal_symbol *msym);
void set_gdbarch_elf_make_msymbol_special (struct gdbarch *gdbarch, gdbarch_elf_make_msymbol_special_ftype *elf_make_msymbol_special);

using gdbarch_coff_make_msymbol_special_ftype = void (int val, struct minimal_symbol *msym);
void gdbarch_coff_make_msymbol_special (struct gdbarch *gdbarch, int val, struct minimal_symbol *msym);
void set_gdbarch_coff_make_msymbol_special (struct gdbarch *gdbarch, gdbarch_coff_make_msymbol_special_ftype *coff_make_msymbol_special);

/* Process a symbol in the main symbol table in a backend-specific way.
   Normally this hook is supposed to do nothing, however if required,
   then this hook can be used to apply transformations to symbols that
   are considered special in some way.  This is currently used by the
   MIPS backend to make sure compressed code symbols have the ISA bit
   set.  This in turn is needed for symbol values seen in GDB to match
   the values used at the runtime by the program itself, for function
   and label references. */

using gdbarch_make_symbol_special_ftype = void (struct symbol *sym, struct objfile *objfile);
void gdbarch_make_symbol_special (struct gdbarch *gdbarch, struct symbol *sym, struct objfile *objfile);
void set_gdbarch_make_symbol_special (struct gdbarch *gdbarch, gdbarch_make_symbol_special_ftype *make_symbol_special);

/* Adjust the address retrieved from a DWARF-2 record other than a line
   entry in a backend-specific way.  Normally this hook is supposed to
   return the address passed unchanged, however if that is incorrect for
   any reason, then this hook can be used to fix the address up in the
   required manner.  This is currently used by the MIPS backend to make
   sure addresses in FDE, range records, etc. referring to compressed
   code have the ISA bit set, matching line information and the symbol
   table. */

using gdbarch_adjust_dwarf2_addr_ftype = CORE_ADDR (CORE_ADDR pc);
CORE_ADDR gdbarch_adjust_dwarf2_addr (struct gdbarch *gdbarch, CORE_ADDR pc);
void set_gdbarch_adjust_dwarf2_addr (struct gdbarch *gdbarch, gdbarch_adjust_dwarf2_addr_ftype *adjust_dwarf2_addr);

/* Adjust the address updated by a line entry in a backend-specific way.
   Normally this hook is supposed to return the address passed unchanged,
   however in the case of inconsistencies in these records, this hook can
   be used to fix them up in the required manner.  This is currently used
   by the MIPS backend to make sure all line addresses in compressed code
   are presented with the ISA bit set, which is not always the case.  This
   in turn ensures breakpoint addresses are correctly matched against the
   stop PC. */

using gdbarch_adjust_dwarf2_line_ftype = CORE_ADDR (CORE_ADDR addr, bool rel);
CORE_ADDR gdbarch_adjust_dwarf2_line (struct gdbarch *gdbarch, CORE_ADDR addr, bool rel);
void set_gdbarch_adjust_dwarf2_line (struct gdbarch *gdbarch, gdbarch_adjust_dwarf2_line_ftype *adjust_dwarf2_line);

bool gdbarch_cannot_step_breakpoint (struct gdbarch *gdbarch);
void set_gdbarch_cannot_step_breakpoint (struct gdbarch *gdbarch, bool cannot_step_breakpoint);

/* See comment in target.h about continuable, steppable and
   non-steppable watchpoints. */

bool gdbarch_have_nonsteppable_watchpoint (struct gdbarch *gdbarch);
void set_gdbarch_have_nonsteppable_watchpoint (struct gdbarch *gdbarch, bool have_nonsteppable_watchpoint);

/* Given the DWARF identifier for an architecture-specific address class,
   return the id of that address class. */

bool gdbarch_address_class_dwarf_to_id_p (struct gdbarch *gdbarch);

using gdbarch_address_class_dwarf_to_id_ftype = unsigned int (int byte_size, int dwarf2_addr_class);
unsigned int gdbarch_address_class_dwarf_to_id (struct gdbarch *gdbarch, int byte_size, int dwarf2_addr_class);
void set_gdbarch_address_class_dwarf_to_id (struct gdbarch *gdbarch, gdbarch_address_class_dwarf_to_id_ftype *address_class_dwarf_to_id);

/* Given an architecture-specific address class identifier, return the
   name of that address class. */

bool gdbarch_address_class_id_to_name_p (struct gdbarch *gdbarch);

using gdbarch_address_class_id_to_name_ftype = const char *(struct gdbarch *gdbarch, unsigned int address_class);
const char *gdbarch_address_class_id_to_name (struct gdbarch *gdbarch, unsigned int address_class);
void set_gdbarch_address_class_id_to_name (struct gdbarch *gdbarch, gdbarch_address_class_id_to_name_ftype *address_class_id_to_name);

/* Execute vendor-specific DWARF Call Frame Instruction.  OP is the instruction.
   FS are passed from the generic execute_cfa_program function. */

using gdbarch_execute_dwarf_cfa_vendor_op_ftype = bool (struct gdbarch *gdbarch, gdb_byte op, struct dwarf2_frame_state *fs);
bool gdbarch_execute_dwarf_cfa_vendor_op (struct gdbarch *gdbarch, gdb_byte op, struct dwarf2_frame_state *fs);
void set_gdbarch_execute_dwarf_cfa_vendor_op (struct gdbarch *gdbarch, gdbarch_execute_dwarf_cfa_vendor_op_ftype *execute_dwarf_cfa_vendor_op);

/* Return the appropriate address class id for the supplied address class name.
   This function should return true if the address class was recognized and
   address_class was set, false otherwise. */

bool gdbarch_address_class_name_to_id_p (struct gdbarch *gdbarch);

using gdbarch_address_class_name_to_id_ftype = bool (struct gdbarch *gdbarch, const char *name, unsigned int &address_class);
bool gdbarch_address_class_name_to_id (struct gdbarch *gdbarch, const char *name, unsigned int &address_class);
void set_gdbarch_address_class_name_to_id (struct gdbarch *gdbarch, gdbarch_address_class_name_to_id_ftype *address_class_name_to_id);

/* Is a register in a group */

using gdbarch_register_reggroup_p_ftype = bool (struct gdbarch *gdbarch, int regnum, const struct reggroup *reggroup);
bool gdbarch_register_reggroup_p (struct gdbarch *gdbarch, int regnum, const struct reggroup *reggroup);
void set_gdbarch_register_reggroup_p (struct gdbarch *gdbarch, gdbarch_register_reggroup_p_ftype *register_reggroup_p);

/* Fetch the pointer to the ith function argument. */

using gdbarch_fetch_pointer_argument_ftype = CORE_ADDR (const frame_info_ptr &frame, int argi, struct type *type);
CORE_ADDR gdbarch_fetch_pointer_argument (struct gdbarch *gdbarch, const frame_info_ptr &frame, int argi, struct type *type);
void set_gdbarch_fetch_pointer_argument (struct gdbarch *gdbarch, gdbarch_fetch_pointer_argument_ftype *fetch_pointer_argument);

/* Iterate over all supported register notes in a core file.  For each
   supported register note section, the iterator must call CB and pass
   CB_DATA unchanged.  If REGCACHE is not NULL, the iterator can limit
   the supported register note sections based on the current register
   values.  Otherwise it should enumerate all supported register note
   sections. */

bool gdbarch_iterate_over_regset_sections_p (struct gdbarch *gdbarch);

using gdbarch_iterate_over_regset_sections_ftype = void (struct gdbarch *gdbarch, iterate_over_regset_sections_cb *cb, void *cb_data, const struct regcache *regcache);
void gdbarch_iterate_over_regset_sections (struct gdbarch *gdbarch, iterate_over_regset_sections_cb *cb, void *cb_data, const struct regcache *regcache);
void set_gdbarch_iterate_over_regset_sections (struct gdbarch *gdbarch, gdbarch_iterate_over_regset_sections_ftype *iterate_over_regset_sections);

/* Create core file notes */

bool gdbarch_make_corefile_notes_p (struct gdbarch *gdbarch);

using gdbarch_make_corefile_notes_ftype = gdb::unique_xmalloc_ptr<char> (struct gdbarch *gdbarch, bfd *obfd, int *note_size);
gdb::unique_xmalloc_ptr<char> gdbarch_make_corefile_notes (struct gdbarch *gdbarch, bfd *obfd, int *note_size);
void set_gdbarch_make_corefile_notes (struct gdbarch *gdbarch, gdbarch_make_corefile_notes_ftype *make_corefile_notes);

/* Find core file memory regions */

bool gdbarch_find_memory_regions_p (struct gdbarch *gdbarch);

using gdbarch_find_memory_regions_ftype = bool (struct gdbarch *gdbarch, find_memory_region_ftype func);
bool gdbarch_find_memory_regions (struct gdbarch *gdbarch, find_memory_region_ftype func);
void set_gdbarch_find_memory_regions (struct gdbarch *gdbarch, gdbarch_find_memory_regions_ftype *find_memory_regions);

/* Given a bfd OBFD, segment ADDRESS and SIZE, create a memory tag section to be dumped to a core file */

using gdbarch_create_memtag_section_ftype = asection *(struct gdbarch *gdbarch, bfd *obfd, CORE_ADDR address, size_t size);
asection *gdbarch_create_memtag_section (struct gdbarch *gdbarch, bfd *obfd, CORE_ADDR address, size_t size);
void set_gdbarch_create_memtag_section (struct gdbarch *gdbarch, gdbarch_create_memtag_section_ftype *create_memtag_section);

/* Given a memory tag section OSEC, fill OSEC's contents with the appropriate tag data */

using gdbarch_fill_memtag_section_ftype = bool (struct gdbarch *gdbarch, asection *osec);
bool gdbarch_fill_memtag_section (struct gdbarch *gdbarch, asection *osec);
void set_gdbarch_fill_memtag_section (struct gdbarch *gdbarch, gdbarch_fill_memtag_section_ftype *fill_memtag_section);

/* Decode a memory tag SECTION and return the tags of type TYPE contained in
   the memory range [ADDRESS, ADDRESS + LENGTH).
   If no tags were found, return an empty vector. */

bool gdbarch_decode_memtag_section_p (struct gdbarch *gdbarch);

using gdbarch_decode_memtag_section_ftype = gdb::byte_vector (struct gdbarch *gdbarch, bfd_section *section, int type, CORE_ADDR address, size_t length);
gdb::byte_vector gdbarch_decode_memtag_section (struct gdbarch *gdbarch, bfd_section *section, int type, CORE_ADDR address, size_t length);
void set_gdbarch_decode_memtag_section (struct gdbarch *gdbarch, gdbarch_decode_memtag_section_ftype *decode_memtag_section);

/* Read offset OFFSET of TARGET_OBJECT_LIBRARIES formatted shared libraries list from
   core file into buffer READBUF with length LEN.  Return the number of bytes read
   (zero indicates failure).
   failed, otherwise, return the red length of READBUF. */

bool gdbarch_core_xfer_shared_libraries_p (struct gdbarch *gdbarch);

using gdbarch_core_xfer_shared_libraries_ftype = ULONGEST (struct gdbarch *gdbarch, struct bfd &cbfd, gdb_byte *readbuf, ULONGEST offset, ULONGEST len);
ULONGEST gdbarch_core_xfer_shared_libraries (struct gdbarch *gdbarch, struct bfd &cbfd, gdb_byte *readbuf, ULONGEST offset, ULONGEST len);
void set_gdbarch_core_xfer_shared_libraries (struct gdbarch *gdbarch, gdbarch_core_xfer_shared_libraries_ftype *core_xfer_shared_libraries);

/* Read offset OFFSET of TARGET_OBJECT_LIBRARIES_AIX formatted shared
   libraries list from core file into buffer READBUF with length LEN.
   Return the number of bytes read (zero indicates failure). */

bool gdbarch_core_xfer_shared_libraries_aix_p (struct gdbarch *gdbarch);

using gdbarch_core_xfer_shared_libraries_aix_ftype = ULONGEST (struct gdbarch *gdbarch, struct bfd &cbfd, gdb_byte *readbuf, ULONGEST offset, ULONGEST len);
ULONGEST gdbarch_core_xfer_shared_libraries_aix (struct gdbarch *gdbarch, struct bfd &cbfd, gdb_byte *readbuf, ULONGEST offset, ULONGEST len);
void set_gdbarch_core_xfer_shared_libraries_aix (struct gdbarch *gdbarch, gdbarch_core_xfer_shared_libraries_aix_ftype *core_xfer_shared_libraries_aix);

/* How the core target converts a PTID from a core file to a string. */

bool gdbarch_core_pid_to_str_p (struct gdbarch *gdbarch);

using gdbarch_core_pid_to_str_ftype = std::string (struct gdbarch *gdbarch, ptid_t ptid);
std::string gdbarch_core_pid_to_str (struct gdbarch *gdbarch, ptid_t ptid);
void set_gdbarch_core_pid_to_str (struct gdbarch *gdbarch, gdbarch_core_pid_to_str_ftype *core_pid_to_str);

/* How the core target extracts the name of a thread from core file CBFD. */

bool gdbarch_core_thread_name_p (struct gdbarch *gdbarch);

using gdbarch_core_thread_name_ftype = const char *(struct gdbarch *gdbarch, struct bfd &cbfd, struct thread_info *thr);
const char *gdbarch_core_thread_name (struct gdbarch *gdbarch, struct bfd &cbfd, struct thread_info *thr);
void set_gdbarch_core_thread_name (struct gdbarch *gdbarch, gdbarch_core_thread_name_ftype *core_thread_name);

/* Read offset OFFSET of TARGET_OBJECT_SIGNAL_INFO signal information
   from core file CBFD into buffer READBUF with length LEN.  Return the number
   of bytes read (zero indicates EOF, a negative value indicates failure). */

bool gdbarch_core_xfer_siginfo_p (struct gdbarch *gdbarch);

using gdbarch_core_xfer_siginfo_ftype = LONGEST (struct gdbarch *gdbarch, struct bfd &cbfd, gdb_byte *readbuf, ULONGEST offset, ULONGEST len);
LONGEST gdbarch_core_xfer_siginfo (struct gdbarch *gdbarch, struct bfd &cbfd, gdb_byte *readbuf, ULONGEST offset, ULONGEST len);
void set_gdbarch_core_xfer_siginfo (struct gdbarch *gdbarch, gdbarch_core_xfer_siginfo_ftype *core_xfer_siginfo);

/* Read x86 XSAVE layout information from core file CBFD into XSAVE_LAYOUT.
   Returns true if the layout was read successfully. */

bool gdbarch_core_read_x86_xsave_layout_p (struct gdbarch *gdbarch);

using gdbarch_core_read_x86_xsave_layout_ftype = bool (struct gdbarch *gdbarch, struct bfd &cbfd, x86_xsave_layout &xsave_layout);
bool gdbarch_core_read_x86_xsave_layout (struct gdbarch *gdbarch, struct bfd &cbfd, x86_xsave_layout &xsave_layout);
void set_gdbarch_core_read_x86_xsave_layout (struct gdbarch *gdbarch, gdbarch_core_read_x86_xsave_layout_ftype *core_read_x86_xsave_layout);

/* Return the executable path of a core file. */

bool gdbarch_core_load_executable_p (struct gdbarch *gdbarch);

using gdbarch_core_load_executable_ftype = char *(struct gdbarch *gdbarch);
char *gdbarch_core_load_executable (struct gdbarch *gdbarch);
void set_gdbarch_core_load_executable (struct gdbarch *gdbarch, gdbarch_core_load_executable_ftype *core_load_executable);

/* BFD target to use when generating a core file. */

bool gdbarch_gcore_bfd_target_p (struct gdbarch *gdbarch);

const char *gdbarch_gcore_bfd_target (struct gdbarch *gdbarch);
void set_gdbarch_gcore_bfd_target (struct gdbarch *gdbarch, const char *gcore_bfd_target);

/* If the elements of C++ vtables are in-place function descriptors rather
   than normal function pointers (which may point to code or a descriptor),
   set this to true. */

bool gdbarch_vtable_function_descriptors (struct gdbarch *gdbarch);
void set_gdbarch_vtable_function_descriptors (struct gdbarch *gdbarch, bool vtable_function_descriptors);

/* Set if the least significant bit of the delta is used instead of the least
   significant bit of the pfn for pointers to virtual member functions. */

bool gdbarch_vbit_in_delta (struct gdbarch *gdbarch);
void set_gdbarch_vbit_in_delta (struct gdbarch *gdbarch, bool vbit_in_delta);

/* The maximum length of an instruction on this architecture in bytes. */

bool gdbarch_max_insn_length_p (struct gdbarch *gdbarch);

ULONGEST gdbarch_max_insn_length (struct gdbarch *gdbarch);
void set_gdbarch_max_insn_length (struct gdbarch *gdbarch, ULONGEST max_insn_length);

/* Copy the instruction at FROM to TO, and make any adjustments
   necessary to single-step it at that address.

   REGS holds the state the thread's registers will have before
   executing the copied instruction; the PC in REGS will refer to FROM,
   not the copy at TO.  The caller should update it to point at TO later.

   Return a pointer to data of the architecture's choice to be passed
   to gdbarch_displaced_step_fixup.

   For a general explanation of displaced stepping and how GDB uses it,
   see the comments in infrun.c.

   The TO area is only guaranteed to have space for
   gdbarch_displaced_step_buffer_length (arch) octets, so this
   function must not write more octets than that to this area.

   If you do not provide this function, GDB assumes that the
   architecture does not support displaced stepping.

   If the instruction cannot execute out of line, return NULL.  The
   core falls back to stepping past the instruction in-line instead in
   that case. */

using gdbarch_displaced_step_copy_insn_ftype = displaced_step_copy_insn_closure_up (struct gdbarch *gdbarch, CORE_ADDR from, CORE_ADDR to, struct regcache *regs);
displaced_step_copy_insn_closure_up gdbarch_displaced_step_copy_insn (struct gdbarch *gdbarch, CORE_ADDR from, CORE_ADDR to, struct regcache *regs);
void set_gdbarch_displaced_step_copy_insn (struct gdbarch *gdbarch, gdbarch_displaced_step_copy_insn_ftype *displaced_step_copy_insn);

/* Return true if GDB should use hardware single-stepping to execute a displaced
   step instruction.  If false, GDB will simply restart execution at the
   displaced instruction location, and it is up to the target to ensure GDB will
   receive control again (e.g. by placing a software breakpoint instruction into
   the displaced instruction buffer).

   The default implementation returns false on all targets that provide a
   gdbarch_get_next_pcs routine, and true otherwise. */

using gdbarch_displaced_step_hw_singlestep_ftype = bool (struct gdbarch *gdbarch);
bool gdbarch_displaced_step_hw_singlestep (struct gdbarch *gdbarch);
void set_gdbarch_displaced_step_hw_singlestep (struct gdbarch *gdbarch, gdbarch_displaced_step_hw_singlestep_ftype *displaced_step_hw_singlestep);

/* Fix up the state after attempting to single-step a displaced
   instruction, to give the result we would have gotten from stepping the
   instruction in its original location.

   REGS is the register state resulting from single-stepping the
   displaced instruction.

   CLOSURE is the result from the matching call to
   gdbarch_displaced_step_copy_insn.

   FROM is the address where the instruction was original located, TO is
   the address of the displaced buffer where the instruction was copied
   to for stepping.

   COMPLETED_P is true if GDB stopped as a result of the requested step
   having completed (e.g. the inferior stopped with SIGTRAP), otherwise
   COMPLETED_P is false and GDB stopped for some other reason.  In the
   case where a single instruction is expanded to multiple replacement
   instructions for stepping then it may be necessary to read the current
   program counter from REGS in order to decide how far through the
   series of replacement instructions the inferior got before stopping,
   this may impact what will need fixing up in this function.

   For a general explanation of displaced stepping and how GDB uses it,
   see the comments in infrun.c. */

using gdbarch_displaced_step_fixup_ftype = void (struct gdbarch *gdbarch, struct displaced_step_copy_insn_closure *closure, CORE_ADDR from, CORE_ADDR to, struct regcache *regs, bool completed_p);
void gdbarch_displaced_step_fixup (struct gdbarch *gdbarch, struct displaced_step_copy_insn_closure *closure, CORE_ADDR from, CORE_ADDR to, struct regcache *regs, bool completed_p);
void set_gdbarch_displaced_step_fixup (struct gdbarch *gdbarch, gdbarch_displaced_step_fixup_ftype *displaced_step_fixup);

/* Prepare THREAD for it to displaced step the instruction at its current PC.

   Throw an exception if any unexpected error happens. */

bool gdbarch_displaced_step_prepare_p (struct gdbarch *gdbarch);

using gdbarch_displaced_step_prepare_ftype = displaced_step_prepare_status (struct gdbarch *gdbarch, thread_info *thread, CORE_ADDR &displaced_pc);
displaced_step_prepare_status gdbarch_displaced_step_prepare (struct gdbarch *gdbarch, thread_info *thread, CORE_ADDR &displaced_pc);
void set_gdbarch_displaced_step_prepare (struct gdbarch *gdbarch, gdbarch_displaced_step_prepare_ftype *displaced_step_prepare);

/* Clean up after a displaced step of THREAD.

   It is possible for the displaced-stepped instruction to have caused
   the thread to exit.  The implementation can detect this case by
   checking if WS.kind is TARGET_WAITKIND_THREAD_EXITED. */

using gdbarch_displaced_step_finish_ftype = displaced_step_finish_status (struct gdbarch *gdbarch, thread_info *thread, const target_waitstatus &ws);
displaced_step_finish_status gdbarch_displaced_step_finish (struct gdbarch *gdbarch, thread_info *thread, const target_waitstatus &ws);
void set_gdbarch_displaced_step_finish (struct gdbarch *gdbarch, gdbarch_displaced_step_finish_ftype *displaced_step_finish);

/* Return the closure associated to the displaced step buffer that is at ADDR. */

bool gdbarch_displaced_step_copy_insn_closure_by_addr_p (struct gdbarch *gdbarch);

using gdbarch_displaced_step_copy_insn_closure_by_addr_ftype = const displaced_step_copy_insn_closure *(inferior *inf, CORE_ADDR addr);
const displaced_step_copy_insn_closure *gdbarch_displaced_step_copy_insn_closure_by_addr (struct gdbarch *gdbarch, inferior *inf, CORE_ADDR addr);
void set_gdbarch_displaced_step_copy_insn_closure_by_addr (struct gdbarch *gdbarch, gdbarch_displaced_step_copy_insn_closure_by_addr_ftype *displaced_step_copy_insn_closure_by_addr);

/* PARENT_INF has forked and CHILD_PTID is the ptid of the child.  Restore the
   contents of all displaced step buffers in the child's address space. */

using gdbarch_displaced_step_restore_all_in_ptid_ftype = void (inferior *parent_inf, ptid_t child_ptid);
void gdbarch_displaced_step_restore_all_in_ptid (struct gdbarch *gdbarch, inferior *parent_inf, ptid_t child_ptid);
void set_gdbarch_displaced_step_restore_all_in_ptid (struct gdbarch *gdbarch, gdbarch_displaced_step_restore_all_in_ptid_ftype *displaced_step_restore_all_in_ptid);

/* The maximum length in octets required for a displaced-step instruction
   buffer.  By default this will be the same as gdbarch::max_insn_length,
   but should be overridden for architectures that might expand a
   displaced-step instruction to multiple replacement instructions. */

ULONGEST gdbarch_displaced_step_buffer_length (struct gdbarch *gdbarch);
void set_gdbarch_displaced_step_buffer_length (struct gdbarch *gdbarch, ULONGEST displaced_step_buffer_length);

/* Relocate an instruction to execute at a different address.  OLDLOC
   is the address in the inferior memory where the instruction to
   relocate is currently at.  On input, TO points to the destination
   where we want the instruction to be copied (and possibly adjusted)
   to.  On output, it points to one past the end of the resulting
   instruction(s).  The effect of executing the instruction at TO shall
   be the same as if executing it at FROM.  For example, call
   instructions that implicitly push the return address on the stack
   should be adjusted to return to the instruction after OLDLOC;
   relative branches, and other PC-relative instructions need the
   offset adjusted; etc. */

using gdbarch_relocate_instruction_ftype = void (struct gdbarch *gdbarch, CORE_ADDR *to, CORE_ADDR from);
void gdbarch_relocate_instruction (struct gdbarch *gdbarch, CORE_ADDR *to, CORE_ADDR from);
void set_gdbarch_relocate_instruction (struct gdbarch *gdbarch, gdbarch_relocate_instruction_ftype *relocate_instruction);

/* Refresh overlay mapped state for section OSECT. */

bool gdbarch_overlay_update_p (struct gdbarch *gdbarch);

using gdbarch_overlay_update_ftype = void (struct obj_section *osect);
void gdbarch_overlay_update (struct gdbarch *gdbarch, struct obj_section *osect);
void set_gdbarch_overlay_update (struct gdbarch *gdbarch, gdbarch_overlay_update_ftype *overlay_update);

bool gdbarch_core_read_description_p (struct gdbarch *gdbarch);

using gdbarch_core_read_description_ftype = const struct target_desc *(struct gdbarch *gdbarch, struct target_ops *target, bfd *abfd);
const struct target_desc *gdbarch_core_read_description (struct gdbarch *gdbarch, struct target_ops *target, bfd *abfd);
void set_gdbarch_core_read_description (struct gdbarch *gdbarch, gdbarch_core_read_description_ftype *core_read_description);

/* Parse the instruction at ADDR storing in the record execution log
   the registers REGCACHE and memory ranges that will be affected when
   the instruction executes, along with their current values.
   Return -1 if something goes wrong, 0 otherwise. */

bool gdbarch_process_record_p (struct gdbarch *gdbarch);

using gdbarch_process_record_ftype = int (struct gdbarch *gdbarch, struct regcache *regcache, CORE_ADDR addr);
int gdbarch_process_record (struct gdbarch *gdbarch, struct regcache *regcache, CORE_ADDR addr);
void set_gdbarch_process_record (struct gdbarch *gdbarch, gdbarch_process_record_ftype *process_record);

/* Save process state after a signal.
   Return -1 if something goes wrong, 0 otherwise. */

bool gdbarch_process_record_signal_p (struct gdbarch *gdbarch);

using gdbarch_process_record_signal_ftype = int (struct gdbarch *gdbarch, struct regcache *regcache, enum gdb_signal signal);
int gdbarch_process_record_signal (struct gdbarch *gdbarch, struct regcache *regcache, enum gdb_signal signal);
void set_gdbarch_process_record_signal (struct gdbarch *gdbarch, gdbarch_process_record_signal_ftype *process_record_signal);

/* Signal translation: translate inferior's signal (target's) number
   into GDB's representation.  The implementation of this method must
   be host independent.  IOW, don't rely on symbols of the NAT_FILE
   header (the nm-*.h files), the host <signal.h> header, or similar
   headers.  This is mainly used when cross-debugging core files ---
   "Live" targets hide the translation behind the target interface
   (target_wait, target_resume, etc.). */

bool gdbarch_gdb_signal_from_target_p (struct gdbarch *gdbarch);

using gdbarch_gdb_signal_from_target_ftype = enum gdb_signal (struct gdbarch *gdbarch, int signo);
enum gdb_signal gdbarch_gdb_signal_from_target (struct gdbarch *gdbarch, int signo);
void set_gdbarch_gdb_signal_from_target (struct gdbarch *gdbarch, gdbarch_gdb_signal_from_target_ftype *gdb_signal_from_target);

/* Signal translation: translate the GDB's internal signal number into
   the inferior's signal (target's) representation.  The implementation
   of this method must be host independent.  IOW, don't rely on symbols
   of the NAT_FILE header (the nm-*.h files), the host <signal.h>
   header, or similar headers.
   Return the target signal number if found, or -1 if the GDB internal
   signal number is invalid. */

bool gdbarch_gdb_signal_to_target_p (struct gdbarch *gdbarch);

using gdbarch_gdb_signal_to_target_ftype = int (struct gdbarch *gdbarch, enum gdb_signal signal);
int gdbarch_gdb_signal_to_target (struct gdbarch *gdbarch, enum gdb_signal signal);
void set_gdbarch_gdb_signal_to_target (struct gdbarch *gdbarch, gdbarch_gdb_signal_to_target_ftype *gdb_signal_to_target);

/* Extra signal info inspection.

   Return a type suitable to inspect extra signal information. */

bool gdbarch_get_siginfo_type_p (struct gdbarch *gdbarch);

using gdbarch_get_siginfo_type_ftype = struct type *(struct gdbarch *gdbarch);
struct type *gdbarch_get_siginfo_type (struct gdbarch *gdbarch);
void set_gdbarch_get_siginfo_type (struct gdbarch *gdbarch, gdbarch_get_siginfo_type_ftype *get_siginfo_type);

/* Record architecture-specific information from the symbol table. */

bool gdbarch_record_special_symbol_p (struct gdbarch *gdbarch);

using gdbarch_record_special_symbol_ftype = void (struct gdbarch *gdbarch, struct objfile *objfile, const asymbol *sym);
void gdbarch_record_special_symbol (struct gdbarch *gdbarch, struct objfile *objfile, const asymbol *sym);
void set_gdbarch_record_special_symbol (struct gdbarch *gdbarch, gdbarch_record_special_symbol_ftype *record_special_symbol);

/* Function for the 'catch syscall' feature.
   Get architecture-specific system calls information from registers. */

bool gdbarch_get_syscall_number_p (struct gdbarch *gdbarch);

using gdbarch_get_syscall_number_ftype = LONGEST (struct gdbarch *gdbarch, thread_info *thread);
LONGEST gdbarch_get_syscall_number (struct gdbarch *gdbarch, thread_info *thread);
void set_gdbarch_get_syscall_number (struct gdbarch *gdbarch, gdbarch_get_syscall_number_ftype *get_syscall_number);

/* The filename of the XML syscall for this architecture. */

const char *gdbarch_xml_syscall_file (struct gdbarch *gdbarch);
void set_gdbarch_xml_syscall_file (struct gdbarch *gdbarch, const char *xml_syscall_file);

/* Information about system calls from this architecture */

struct syscalls_info *gdbarch_syscalls_info (struct gdbarch *gdbarch);
void set_gdbarch_syscalls_info (struct gdbarch *gdbarch, struct syscalls_info *syscalls_info);

/* SystemTap related fields and functions.
   A NULL-terminated array of prefixes used to mark an integer constant
   on the architecture's assembly.
   For example, on x86 integer constants are written as:

   $10 ;; integer constant 10

   in this case, this prefix would be the character `$'. */

const char *const *gdbarch_stap_integer_prefixes (struct gdbarch *gdbarch);
void set_gdbarch_stap_integer_prefixes (struct gdbarch *gdbarch, const char *const *stap_integer_prefixes);

/* A NULL-terminated array of suffixes used to mark an integer constant
   on the architecture's assembly. */

const char *const *gdbarch_stap_integer_suffixes (struct gdbarch *gdbarch);
void set_gdbarch_stap_integer_suffixes (struct gdbarch *gdbarch, const char *const *stap_integer_suffixes);

/* A NULL-terminated array of prefixes used to mark a register name on
   the architecture's assembly.
   For example, on x86 the register name is written as:

   %eax ;; register eax

   in this case, this prefix would be the character `%'. */

const char *const *gdbarch_stap_register_prefixes (struct gdbarch *gdbarch);
void set_gdbarch_stap_register_prefixes (struct gdbarch *gdbarch, const char *const *stap_register_prefixes);

/* A NULL-terminated array of suffixes used to mark a register name on
   the architecture's assembly. */

const char *const *gdbarch_stap_register_suffixes (struct gdbarch *gdbarch);
void set_gdbarch_stap_register_suffixes (struct gdbarch *gdbarch, const char *const *stap_register_suffixes);

/* A NULL-terminated array of prefixes used to mark a register
   indirection on the architecture's assembly.
   For example, on x86 the register indirection is written as:

   (%eax) ;; indirecting eax

   in this case, this prefix would be the character `('.

   Please note that we use the indirection prefix also for register
   displacement, e.g., `4(%eax)' on x86. */

const char *const *gdbarch_stap_register_indirection_prefixes (struct gdbarch *gdbarch);
void set_gdbarch_stap_register_indirection_prefixes (struct gdbarch *gdbarch, const char *const *stap_register_indirection_prefixes);

/* A NULL-terminated array of suffixes used to mark a register
   indirection on the architecture's assembly.
   For example, on x86 the register indirection is written as:

   (%eax) ;; indirecting eax

   in this case, this prefix would be the character `)'.

   Please note that we use the indirection suffix also for register
   displacement, e.g., `4(%eax)' on x86. */

const char *const *gdbarch_stap_register_indirection_suffixes (struct gdbarch *gdbarch);
void set_gdbarch_stap_register_indirection_suffixes (struct gdbarch *gdbarch, const char *const *stap_register_indirection_suffixes);

/* Prefix(es) used to name a register using GDB's nomenclature.

   For example, on PPC a register is represented by a number in the assembly
   language (e.g., `10' is the 10th general-purpose register).  However,
   inside GDB this same register has an `r' appended to its name, so the 10th
   register would be represented as `r10' internally. */

const char *gdbarch_stap_gdb_register_prefix (struct gdbarch *gdbarch);
void set_gdbarch_stap_gdb_register_prefix (struct gdbarch *gdbarch, const char *stap_gdb_register_prefix);

/* Suffix used to name a register using GDB's nomenclature. */

const char *gdbarch_stap_gdb_register_suffix (struct gdbarch *gdbarch);
void set_gdbarch_stap_gdb_register_suffix (struct gdbarch *gdbarch, const char *stap_gdb_register_suffix);

/* Check if S is a single operand.

   Single operands can be:
   - Literal integers, e.g. `$10' on x86
   - Register access, e.g. `%eax' on x86
   - Register indirection, e.g. `(%eax)' on x86
   - Register displacement, e.g. `4(%eax)' on x86

   This function should check for these patterns on the string
   and return true if some were found, or false otherwise.  Please try to match
   as much info as you can from the string, i.e., if you have to match
   something like `(%', do not match just the `('. */

bool gdbarch_stap_is_single_operand_p (struct gdbarch *gdbarch);

using gdbarch_stap_is_single_operand_ftype = bool (struct gdbarch *gdbarch, const char *s);
bool gdbarch_stap_is_single_operand (struct gdbarch *gdbarch, const char *s);
void set_gdbarch_stap_is_single_operand (struct gdbarch *gdbarch, gdbarch_stap_is_single_operand_ftype *stap_is_single_operand);

/* Function used to handle a "special case" in the parser.

   A "special case" is considered to be an unknown token, i.e., a token
   that the parser does not know how to parse.  A good example of special
   case would be ARM's register displacement syntax:

   [R0, #4]  ;; displacing R0 by 4

   Since the parser assumes that a register displacement is of the form:

   <number> <indirection_prefix> <register_name> <indirection_suffix>

   it means that it will not be able to recognize and parse this odd syntax.
   Therefore, we should add a special case function that will handle this token.

   This function should generate the proper expression form of the expression
   using GDB's internal expression mechanism (e.g., `write_exp_elt_opcode'
   and so on).  It should also return 1 if the parsing was successful, or zero
   if the token was not recognized as a special token (in this case, returning
   zero means that the special parser is deferring the parsing to the generic
   parser), and should advance the buffer pointer (p->arg). */

bool gdbarch_stap_parse_special_token_p (struct gdbarch *gdbarch);

using gdbarch_stap_parse_special_token_ftype = expr::operation_up (struct gdbarch *gdbarch, struct stap_parse_info *p);
expr::operation_up gdbarch_stap_parse_special_token (struct gdbarch *gdbarch, struct stap_parse_info *p);
void set_gdbarch_stap_parse_special_token (struct gdbarch *gdbarch, gdbarch_stap_parse_special_token_ftype *stap_parse_special_token);

/* Perform arch-dependent adjustments to a register name.

   In very specific situations, it may be necessary for the register
   name present in a SystemTap probe's argument to be handled in a
   special way.  For example, on i386, GCC may over-optimize the
   register allocation and use smaller registers than necessary.  In
   such cases, the client that is reading and evaluating the SystemTap
   probe (ourselves) will need to actually fetch values from the wider
   version of the register in question.

   To illustrate the example, consider the following probe argument
   (i386):

   4@%ax

   This argument says that its value can be found at the %ax register,
   which is a 16-bit register.  However, the argument's prefix says
   that its type is "uint32_t", which is 32-bit in size.  Therefore, in
   this case, GDB should actually fetch the probe's value from register
   %eax, not %ax.  In this scenario, this function would actually
   replace the register name from %ax to %eax.

   The rationale for this can be found at PR breakpoints/24541. */

bool gdbarch_stap_adjust_register_p (struct gdbarch *gdbarch);

using gdbarch_stap_adjust_register_ftype = std::string (struct gdbarch *gdbarch, struct stap_parse_info *p, const std::string &regname, int regnum);
std::string gdbarch_stap_adjust_register (struct gdbarch *gdbarch, struct stap_parse_info *p, const std::string &regname, int regnum);
void set_gdbarch_stap_adjust_register (struct gdbarch *gdbarch, gdbarch_stap_adjust_register_ftype *stap_adjust_register);

/* DTrace related functions.
   The expression to compute the NARTGth+1 argument to a DTrace USDT probe.
   NARG must be >= 0. */

bool gdbarch_dtrace_parse_probe_argument_p (struct gdbarch *gdbarch);

using gdbarch_dtrace_parse_probe_argument_ftype = expr::operation_up (struct gdbarch *gdbarch, int narg);
expr::operation_up gdbarch_dtrace_parse_probe_argument (struct gdbarch *gdbarch, int narg);
void set_gdbarch_dtrace_parse_probe_argument (struct gdbarch *gdbarch, gdbarch_dtrace_parse_probe_argument_ftype *dtrace_parse_probe_argument);

/* True if the given ADDR does not contain the instruction sequence
   corresponding to a disabled DTrace is-enabled probe. */

bool gdbarch_dtrace_probe_is_enabled_p (struct gdbarch *gdbarch);

using gdbarch_dtrace_probe_is_enabled_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR addr);
bool gdbarch_dtrace_probe_is_enabled (struct gdbarch *gdbarch, CORE_ADDR addr);
void set_gdbarch_dtrace_probe_is_enabled (struct gdbarch *gdbarch, gdbarch_dtrace_probe_is_enabled_ftype *dtrace_probe_is_enabled);

/* Enable a DTrace is-enabled probe at ADDR. */

bool gdbarch_dtrace_enable_probe_p (struct gdbarch *gdbarch);

using gdbarch_dtrace_enable_probe_ftype = void (struct gdbarch *gdbarch, CORE_ADDR addr);
void gdbarch_dtrace_enable_probe (struct gdbarch *gdbarch, CORE_ADDR addr);
void set_gdbarch_dtrace_enable_probe (struct gdbarch *gdbarch, gdbarch_dtrace_enable_probe_ftype *dtrace_enable_probe);

/* Disable a DTrace is-enabled probe at ADDR. */

bool gdbarch_dtrace_disable_probe_p (struct gdbarch *gdbarch);

using gdbarch_dtrace_disable_probe_ftype = void (struct gdbarch *gdbarch, CORE_ADDR addr);
void gdbarch_dtrace_disable_probe (struct gdbarch *gdbarch, CORE_ADDR addr);
void set_gdbarch_dtrace_disable_probe (struct gdbarch *gdbarch, gdbarch_dtrace_disable_probe_ftype *dtrace_disable_probe);

/* True if the list of shared libraries is one and only for all
   processes, as opposed to a list of shared libraries per inferior.
   This usually means that all processes, although may or may not share
   an address space, will see the same set of symbols at the same
   addresses. */

bool gdbarch_has_global_solist (struct gdbarch *gdbarch);
void set_gdbarch_has_global_solist (struct gdbarch *gdbarch, bool has_global_solist);

/* On some targets, even though each inferior has its own private
   address space, the debug interface takes care of making breakpoints
   visible to all address spaces automatically.  For such cases,
   this property should be set to true. */

bool gdbarch_has_global_breakpoints (struct gdbarch *gdbarch);
void set_gdbarch_has_global_breakpoints (struct gdbarch *gdbarch, bool has_global_breakpoints);

/* True if inferiors share an address space (e.g., uClinux). */

using gdbarch_has_shared_address_space_ftype = bool (struct gdbarch *gdbarch);
bool gdbarch_has_shared_address_space (struct gdbarch *gdbarch);
void set_gdbarch_has_shared_address_space (struct gdbarch *gdbarch, gdbarch_has_shared_address_space_ftype *has_shared_address_space);

/* True if a fast tracepoint can be set at an address. */

using gdbarch_fast_tracepoint_valid_at_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR addr, std::string *msg);
bool gdbarch_fast_tracepoint_valid_at (struct gdbarch *gdbarch, CORE_ADDR addr, std::string *msg);
void set_gdbarch_fast_tracepoint_valid_at (struct gdbarch *gdbarch, gdbarch_fast_tracepoint_valid_at_ftype *fast_tracepoint_valid_at);

/* Guess register state based on tracepoint location.  Used for tracepoints
   where no registers have been collected, but there's only one location,
   allowing us to guess the PC value, and perhaps some other registers.
   On entry, regcache has all registers marked as unavailable. */

using gdbarch_guess_tracepoint_registers_ftype = void (struct gdbarch *gdbarch, struct regcache *regcache, CORE_ADDR addr);
void gdbarch_guess_tracepoint_registers (struct gdbarch *gdbarch, struct regcache *regcache, CORE_ADDR addr);
void set_gdbarch_guess_tracepoint_registers (struct gdbarch *gdbarch, gdbarch_guess_tracepoint_registers_ftype *guess_tracepoint_registers);

/* Return the "auto" target wide charset. */

using gdbarch_auto_wide_charset_ftype = const char *();
const char *gdbarch_auto_wide_charset (struct gdbarch *gdbarch);
void set_gdbarch_auto_wide_charset (struct gdbarch *gdbarch, gdbarch_auto_wide_charset_ftype *auto_wide_charset);

/* If true, the target OS has DOS-based file system semantics.  That
   is, absolute paths include a drive name, and the backslash is
   considered a directory separator. */

bool gdbarch_has_dos_based_file_system (struct gdbarch *gdbarch);
void set_gdbarch_has_dos_based_file_system (struct gdbarch *gdbarch, bool has_dos_based_file_system);

/* Generate bytecodes to collect the return address in a frame.
   Since the bytecodes run on the target, possibly with GDB not even
   connected, the full unwinding machinery is not available, and
   typically this function will issue bytecodes for one or more likely
   places that the return address may be found. */

using gdbarch_gen_return_address_ftype = void (struct gdbarch *gdbarch, struct agent_expr *ax, struct axs_value *value, CORE_ADDR scope);
void gdbarch_gen_return_address (struct gdbarch *gdbarch, struct agent_expr *ax, struct axs_value *value, CORE_ADDR scope);
void set_gdbarch_gen_return_address (struct gdbarch *gdbarch, gdbarch_gen_return_address_ftype *gen_return_address);

/* Implement the "info proc" command. */

bool gdbarch_info_proc_p (struct gdbarch *gdbarch);

using gdbarch_info_proc_ftype = void (struct gdbarch *gdbarch, const char *args, enum info_proc_what what);
void gdbarch_info_proc (struct gdbarch *gdbarch, const char *args, enum info_proc_what what);
void set_gdbarch_info_proc (struct gdbarch *gdbarch, gdbarch_info_proc_ftype *info_proc);

/* Implement the "info proc" command for core files.  Note that there
   are two "info_proc"-like methods on gdbarch -- one for core files,
   one for live targets.  CBFD is the core file being read from. */

bool gdbarch_core_info_proc_p (struct gdbarch *gdbarch);

using gdbarch_core_info_proc_ftype = void (struct gdbarch *gdbarch, struct bfd *cbfd, const char *args, enum info_proc_what what);
void gdbarch_core_info_proc (struct gdbarch *gdbarch, struct bfd *cbfd, const char *args, enum info_proc_what what);
void set_gdbarch_core_info_proc (struct gdbarch *gdbarch, gdbarch_core_info_proc_ftype *core_info_proc);

/* Ravenscar arch-dependent ops. */

struct ravenscar_arch_ops *gdbarch_ravenscar_ops (struct gdbarch *gdbarch);
void set_gdbarch_ravenscar_ops (struct gdbarch *gdbarch, struct ravenscar_arch_ops *ravenscar_ops);

/* Return true if the instruction at ADDR is a call; false otherwise. */

using gdbarch_insn_is_call_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR addr);
bool gdbarch_insn_is_call (struct gdbarch *gdbarch, CORE_ADDR addr);
void set_gdbarch_insn_is_call (struct gdbarch *gdbarch, gdbarch_insn_is_call_ftype *insn_is_call);

/* Return true if the instruction at ADDR is a return; false otherwise. */

using gdbarch_insn_is_ret_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR addr);
bool gdbarch_insn_is_ret (struct gdbarch *gdbarch, CORE_ADDR addr);
void set_gdbarch_insn_is_ret (struct gdbarch *gdbarch, gdbarch_insn_is_ret_ftype *insn_is_ret);

/* Return true if the instruction at ADDR is a jump; false otherwise. */

using gdbarch_insn_is_jump_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR addr);
bool gdbarch_insn_is_jump (struct gdbarch *gdbarch, CORE_ADDR addr);
void set_gdbarch_insn_is_jump (struct gdbarch *gdbarch, gdbarch_insn_is_jump_ftype *insn_is_jump);

/* Return true if there's a program/permanent breakpoint planted in
   memory at ADDRESS, return false otherwise. */

using gdbarch_program_breakpoint_here_p_ftype = bool (struct gdbarch *gdbarch, CORE_ADDR address);
bool gdbarch_program_breakpoint_here_p (struct gdbarch *gdbarch, CORE_ADDR address);
void set_gdbarch_program_breakpoint_here_p (struct gdbarch *gdbarch, gdbarch_program_breakpoint_here_p_ftype *program_breakpoint_here_p);

/* Read one auxv entry from *READPTR, not reading locations >= ENDPTR.
   Return 0 if *READPTR is already at the end of the buffer.
   Return -1 if there is insufficient buffer for a whole entry.
   Return 1 if an entry was read into *TYPEP and *VALP. */

bool gdbarch_auxv_parse_p (struct gdbarch *gdbarch);

using gdbarch_auxv_parse_ftype = int (struct gdbarch *gdbarch, const gdb_byte **readptr, const gdb_byte *endptr, CORE_ADDR *typep, CORE_ADDR *valp);
int gdbarch_auxv_parse (struct gdbarch *gdbarch, const gdb_byte **readptr, const gdb_byte *endptr, CORE_ADDR *typep, CORE_ADDR *valp);
void set_gdbarch_auxv_parse (struct gdbarch *gdbarch, gdbarch_auxv_parse_ftype *auxv_parse);

/* Print the description of a single auxv entry described by TYPE and VAL
   to FILE. */

using gdbarch_print_auxv_entry_ftype = void (struct gdbarch *gdbarch, struct ui_file *file, CORE_ADDR type, CORE_ADDR val);
void gdbarch_print_auxv_entry (struct gdbarch *gdbarch, struct ui_file *file, CORE_ADDR type, CORE_ADDR val);
void set_gdbarch_print_auxv_entry (struct gdbarch *gdbarch, gdbarch_print_auxv_entry_ftype *print_auxv_entry);

/* Find the address range of the current inferior's vsyscall/vDSO, and
   write it to *RANGE.  If the vsyscall's length can't be determined, a
   range with zero length is returned.  Returns true if the vsyscall is
   found, false otherwise. */

using gdbarch_vsyscall_range_ftype = bool (struct gdbarch *gdbarch, struct mem_range *range);
bool gdbarch_vsyscall_range (struct gdbarch *gdbarch, struct mem_range *range);
void set_gdbarch_vsyscall_range (struct gdbarch *gdbarch, gdbarch_vsyscall_range_ftype *vsyscall_range);

/* Allocate SIZE bytes of PROT protected page aligned memory in inferior.
   PROT has GDB_MMAP_PROT_* bitmask format.
   Throw an error if it is not possible.  Returned address is always valid. */

using gdbarch_infcall_mmap_ftype = CORE_ADDR (CORE_ADDR size, unsigned prot);
CORE_ADDR gdbarch_infcall_mmap (struct gdbarch *gdbarch, CORE_ADDR size, unsigned prot);
void set_gdbarch_infcall_mmap (struct gdbarch *gdbarch, gdbarch_infcall_mmap_ftype *infcall_mmap);

/* Deallocate SIZE bytes of memory at ADDR in inferior from gdbarch_infcall_mmap.
   Print a warning if it is not possible. */

using gdbarch_infcall_munmap_ftype = void (CORE_ADDR addr, CORE_ADDR size);
void gdbarch_infcall_munmap (struct gdbarch *gdbarch, CORE_ADDR addr, CORE_ADDR size);
void set_gdbarch_infcall_munmap (struct gdbarch *gdbarch, gdbarch_infcall_munmap_ftype *infcall_munmap);

/* Return string (caller has to use xfree for it) with options for GCC
   to produce code for this target, typically "-m64", "-m32" or "-m31".
   These options are put before CU's DW_AT_producer compilation options so that
   they can override it. */

using gdbarch_gcc_target_options_ftype = std::string (struct gdbarch *gdbarch);
std::string gdbarch_gcc_target_options (struct gdbarch *gdbarch);
void set_gdbarch_gcc_target_options (struct gdbarch *gdbarch, gdbarch_gcc_target_options_ftype *gcc_target_options);

/* Return a regular expression that matches names used by this
   architecture in GNU configury triplets.  The result is statically
   allocated and must not be freed.  The default implementation simply
   returns the BFD architecture name, which is correct in nearly every
   case. */

using gdbarch_gnu_triplet_regexp_ftype = const char *(struct gdbarch *gdbarch);
const char *gdbarch_gnu_triplet_regexp (struct gdbarch *gdbarch);
void set_gdbarch_gnu_triplet_regexp (struct gdbarch *gdbarch, gdbarch_gnu_triplet_regexp_ftype *gnu_triplet_regexp);

/* Return the size in 8-bit bytes of an addressable memory unit on this
   architecture.  This corresponds to the number of 8-bit bytes associated to
   each address in memory. */

using gdbarch_addressable_memory_unit_size_ftype = int (struct gdbarch *gdbarch);
int gdbarch_addressable_memory_unit_size (struct gdbarch *gdbarch);
void set_gdbarch_addressable_memory_unit_size (struct gdbarch *gdbarch, gdbarch_addressable_memory_unit_size_ftype *addressable_memory_unit_size);

/* Functions for allowing a target to modify its disassembler options. */

const char *gdbarch_disassembler_options_implicit (struct gdbarch *gdbarch);
void set_gdbarch_disassembler_options_implicit (struct gdbarch *gdbarch, const char *disassembler_options_implicit);

std::string *gdbarch_disassembler_options (struct gdbarch *gdbarch);
void set_gdbarch_disassembler_options (struct gdbarch *gdbarch, std::string *disassembler_options);

const disasm_options_and_args_t *gdbarch_valid_disassembler_options (struct gdbarch *gdbarch);
void set_gdbarch_valid_disassembler_options (struct gdbarch *gdbarch, const disasm_options_and_args_t *valid_disassembler_options);

/* Type alignment override method.  Return the architecture specific
   alignment required for TYPE.  If there is no special handling
   required for TYPE then return the value 0, GDB will then apply the
   default rules as laid out in gdbtypes.c:type_align. */

using gdbarch_type_align_ftype = ULONGEST (struct gdbarch *gdbarch, struct type *type);
ULONGEST gdbarch_type_align (struct gdbarch *gdbarch, struct type *type);
void set_gdbarch_type_align (struct gdbarch *gdbarch, gdbarch_type_align_ftype *type_align);

/* Return a string containing any flags for the given PC in the given FRAME. */

using gdbarch_get_pc_address_flags_ftype = std::string (const frame_info_ptr &frame, CORE_ADDR pc);
std::string gdbarch_get_pc_address_flags (struct gdbarch *gdbarch, const frame_info_ptr &frame, CORE_ADDR pc);
void set_gdbarch_get_pc_address_flags (struct gdbarch *gdbarch, gdbarch_get_pc_address_flags_ftype *get_pc_address_flags);

/* Read core file mappings */

using gdbarch_read_core_file_mappings_ftype = void (struct gdbarch *gdbarch, struct bfd *cbfd, read_core_file_mappings_loop_ftype loop_cb);
void gdbarch_read_core_file_mappings (struct gdbarch *gdbarch, struct bfd *cbfd, read_core_file_mappings_loop_ftype loop_cb);
void set_gdbarch_read_core_file_mappings (struct gdbarch *gdbarch, gdbarch_read_core_file_mappings_ftype *read_core_file_mappings);

/* Return true if the target description for all threads should be read from the
   target description core file note(s).  Return false if the target description
   for all threads should be inferred from the core file contents/sections.

   The corefile's bfd is passed through COREFILE_BFD. */

using gdbarch_use_target_description_from_corefile_notes_ftype = bool (struct gdbarch *gdbarch, struct bfd *corefile_bfd);
bool gdbarch_use_target_description_from_corefile_notes (struct gdbarch *gdbarch, struct bfd *corefile_bfd);
void set_gdbarch_use_target_description_from_corefile_notes (struct gdbarch *gdbarch, gdbarch_use_target_description_from_corefile_notes_ftype *use_target_description_from_corefile_notes);

/* Examine the core file bfd object CBFD and try to extract the name of
   the current executable and the argument list, which are return in a
   core_file_exec_context object.

   If for any reason the details can't be extracted from CBFD then an
   empty context is returned.

   It is required that the current inferior be the one associated with
   CBFD, strings are read from the current inferior using target methods
   which all assume current_inferior() is the one to read from. */

using gdbarch_core_parse_exec_context_ftype = core_file_exec_context (struct gdbarch *gdbarch, bfd *cbfd);
core_file_exec_context gdbarch_core_parse_exec_context (struct gdbarch *gdbarch, bfd *cbfd);
void set_gdbarch_core_parse_exec_context (struct gdbarch *gdbarch, gdbarch_core_parse_exec_context_ftype *core_parse_exec_context);

/* Some targets support special hardware-assisted control-flow protection
   technologies.  For example, the Intel Control-Flow Enforcement Technology
   (Intel CET) on x86 provides a shadow stack and indirect branch tracking.
   To enable shadow stack support for inferior calls the shadow_stack_push
   gdbarch hook has to be provided.  The get_shadow_stack_pointer gdbarch
   hook has to be provided to enable displaced stepping.

   Push NEW_ADDR to the shadow stack and update the shadow stack pointer. */

bool gdbarch_shadow_stack_push_p (struct gdbarch *gdbarch);

using gdbarch_shadow_stack_push_ftype = void (struct gdbarch *gdbarch, CORE_ADDR new_addr, regcache *regcache);
void gdbarch_shadow_stack_push (struct gdbarch *gdbarch, CORE_ADDR new_addr, regcache *regcache);
void set_gdbarch_shadow_stack_push (struct gdbarch *gdbarch, gdbarch_shadow_stack_push_ftype *shadow_stack_push);

/* If possible, return the shadow stack pointer.  If the shadow stack
   feature is enabled then set SHADOW_STACK_ENABLED to true, otherwise
   set SHADOW_STACK_ENABLED to false.  This hook has to be provided to enable
   displaced stepping for shadow stack enabled programs.
   On some architectures, the shadow stack pointer is available even if the
   feature is disabled.  So dependent on the target, an implementation of
   this function may return a valid shadow stack pointer, but set
   SHADOW_STACK_ENABLED to false. */

using gdbarch_get_shadow_stack_pointer_ftype = std::optional<CORE_ADDR> (struct gdbarch *gdbarch, regcache *regcache, bool &shadow_stack_enabled);
std::optional<CORE_ADDR> gdbarch_get_shadow_stack_pointer (struct gdbarch *gdbarch, regcache *regcache, bool &shadow_stack_enabled);
void set_gdbarch_get_shadow_stack_pointer (struct gdbarch *gdbarch, gdbarch_get_shadow_stack_pointer_ftype *get_shadow_stack_pointer);

/* Copyright (C) 2008-2025 Free Software Foundation, Inc.

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

#ifndef GDB_WINDOWS_NAT_H
#define GDB_WINDOWS_NAT_H

#include <atomic>
#include <queue>
#include <vector>

#include "inf-child.h"
#include "nat/windows-nat.h"
#include "ser-event.h"

using windows_nat::windows_thread_info;
using windows_nat::thread_disposition_type;

/* A pointer to a function that should return non-zero iff REGNUM
   corresponds to one of the segment registers.  */
typedef int (segment_register_p_ftype) (int regnum);

/* Maintain a linked list of "so" information.  */
struct windows_solib
{
  LPVOID load_addr = 0;
  CORE_ADDR text_offset = 0;

  /* Original name.  */
  std::string original_name;
  /* Expanded form of the name.  */
  std::string name;
};

struct windows_per_inferior : public windows_nat::windows_process_info
{
  windows_thread_info *thread_rec (ptid_t ptid,
				   thread_disposition_type disposition) override;
  int handle_output_debug_string (struct target_waitstatus *ourstatus) override;
  void handle_load_dll (const char *dll_name, LPVOID base) override;
  void handle_unload_dll () override;
  bool handle_access_violation (const EXCEPTION_RECORD *rec) override;

  int windows_initialization_done = 0;

  std::vector<std::unique_ptr<windows_thread_info>> thread_list;

  /* Counts of things.  */
  int saw_create = 0;
  int open_process_used = 0;
#ifdef __x86_64__
  void *wow64_dbgbreak = nullptr;
#endif

  /* This vector maps GDB's idea of a register's number into an offset
     in the windows exception context vector.

     It also contains the bit mask needed to load the register in question.

     The contents of this table can only be computed by the units
     that provide CPU-specific support for Windows native debugging.

     One day we could read a reg, we could inspect the context we
     already have loaded, if it doesn't have the bit set that we need,
     we read that set of registers in using GetThreadContext.  If the
     context already contains what we need, we just unpack it.  Then to
     write a register, first we have to ensure that the context contains
     the other regs of the group, and then we copy the info in and set
     out bit.  */

  const int *mappings = nullptr;

  /* The function to use in order to determine whether a register is
     a segment register or not.  */
  segment_register_p_ftype *segment_register_p = nullptr;

  std::vector<windows_solib> solibs;

#ifdef __CYGWIN__
  /* The starting and ending address of the cygwin1.dll text segment.  */
  CORE_ADDR cygwin_load_start = 0;
  CORE_ADDR cygwin_load_end = 0;
#endif /* __CYGWIN__ */
};

struct windows_nat_target : public inf_child_target
{
  windows_nat_target ();

  void close () override;

  void attach (const char *, int) override;

  bool attach_no_wait () override
  { return true; }

  void detach (inferior *, int) override;

  void resume (ptid_t, int , enum gdb_signal) override;

  ptid_t wait (ptid_t, struct target_waitstatus *, target_wait_flags) override;

  void fetch_registers (struct regcache *, int) override;
  void store_registers (struct regcache *, int) override;

  bool stopped_by_sw_breakpoint () override;

  bool supports_stopped_by_sw_breakpoint () override
  {
    return true;
  }

  enum target_xfer_status xfer_partial (enum target_object object,
					const char *annex,
					gdb_byte *readbuf,
					const gdb_byte *writebuf,
					ULONGEST offset, ULONGEST len,
					ULONGEST *xfered_len) override;

  void files_info () override;

  void kill () override;

  void create_inferior (const char *, const std::string &,
			char **, int) override;

  void mourn_inferior () override;

  bool thread_alive (ptid_t ptid) override;

  std::string pid_to_str (ptid_t) override;

  void interrupt () override;
  void pass_ctrlc () override;

  const char *pid_to_exec_file (int pid) override;

  ptid_t get_ada_task_ptid (long lwp, ULONGEST thread) override;

  bool get_tib_address (ptid_t ptid, CORE_ADDR *addr) override;

  const char *thread_name (struct thread_info *) override;

  ptid_t get_windows_debug_event (int pid, struct target_waitstatus *ourstatus,
				  target_wait_flags options);

  void do_initial_windows_stuff (DWORD pid, bool attaching);

  bool supports_disable_randomization () override
  {
    return windows_nat::disable_randomization_available ();
  }

  bool can_async_p () override
  {
    return true;
  }

  bool is_async_p () override
  {
    return m_is_async;
  }

  void async (bool enable) override;

  int async_wait_fd () override
  {
    return serial_event_fd (m_wait_event);
  }

  /* Initialize arch-specific data.  */
  virtual void initialize_windows_arch () = 0;
  /* Cleanup arch-specific data.  */
  virtual void cleanup_windows_arch () = 0;

  /* Fill in the threads context.  */
  virtual void fill_thread_context (windows_thread_info *th) = 0;

  /* Prepare the thread context for continuing.  */
  virtual void thread_context_continue (windows_thread_info *th,
					int killed) = 0;
  /* Set the stepping bit in the thread context.  */
  virtual void thread_context_step (windows_thread_info *th) = 0;

private:

  windows_thread_info *add_thread (ptid_t ptid, HANDLE h, void *tlb,
				   bool main_thread_p);
  void delete_thread (ptid_t ptid, DWORD exit_code, bool main_thread_p);
  DWORD fake_create_process ();

  BOOL windows_continue (DWORD continue_status, int id, int killed,
			 bool last_call = false);

  /* Helper function to start process_thread.  */
  static DWORD WINAPI process_thread_starter (LPVOID self);

  /* This function implements the background thread that starts
     inferiors and waits for events.  */
  void process_thread ();

  /* Push FUNC onto the queue of requests for process_thread, and wait
     until it has been called.  On Windows, certain debugging
     functions can only be called by the thread that started (or
     attached to) the inferior.  These are all done in the worker
     thread, via calls to this method.  If FUNC returns true,
     process_thread will wait for debug events when FUNC returns.  */
  void do_synchronously (gdb::function_view<bool ()> func);

  /* This waits for a debug event, dispatching to the worker thread as
     needed.  */
  void wait_for_debug_event_main_thread (DEBUG_EVENT *event);

  /* Force the process_thread thread to return from WaitForDebugEvent.
     PROCESS_ALIVE is set to false if the inferior process exits while
     we're trying to break out the process_thread thread.  This can
     happen because this is called while all threads are running free,
     while we're trying to detach.  */
  void break_out_process_thread (bool &process_alive);

  /* Queue used to send requests to process_thread.  This is
     implicitly locked.  */
  std::queue<gdb::function_view<bool ()>> m_queue;

  /* Event used to signal process_thread that an item has been
     pushed.  */
  HANDLE m_pushed_event;
  /* Event used by process_thread to indicate that it has processed a
     single function call.  */
  HANDLE m_response_event;

  /* Serial event used to communicate wait event availability to the
     main loop.  */
  serial_event *m_wait_event;

  /* The last debug event, when M_WAIT_EVENT has been set.  */
  DEBUG_EVENT m_last_debug_event {};
  /* True if a debug event is pending.  */
  std::atomic<bool> m_debug_event_pending { false };

  /* True if currently in async mode.  */
  bool m_is_async = false;

  /* True if we last called ContinueDebugEvent and the process_thread
     thread is now waiting for events.  False if WaitForDebugEvent
     already returned an event, and we need to ContinueDebugEvent
     again to restart the inferior.  */
  bool m_continued = false;
};

/* The current process.  */
extern windows_per_inferior *windows_process;

/* segment_register_p_ftype implementation for x86.  */
int i386_windows_segment_register_p (int regnum);

/* context register offsets for x86.  */
extern const int i386_mappings[];

#ifdef __x86_64__
/* segment_register_p_ftype implementation for amd64.  */
int amd64_windows_segment_register_p (int regnum);

/* context register offsets for amd64.  */
extern const int amd64_mappings[];
#endif

#endif /* GDB_WINDOWS_NAT_H */

/* Copyright (C) 2008-2026 Free Software Foundation, Inc.

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

extern bool debug_exec;		/* show execution */
extern bool debug_events;	/* show events from kernel */
extern bool debug_memory;	/* show target memory accesses */
extern bool debug_exceptions;	/* show target exceptions */

#define DEBUG_EXEC(fmt, ...) \
  debug_prefixed_printf_cond (debug_exec, "windows exec", fmt, ## __VA_ARGS__)
#define DEBUG_EVENTS(fmt, ...) \
  debug_prefixed_printf_cond (debug_events, "windows events", fmt, \
			      ## __VA_ARGS__)
#define DEBUG_MEM(fmt, ...) \
  debug_prefixed_printf_cond (debug_memory, "windows mem", fmt, \
			      ## __VA_ARGS__)
#define DEBUG_EXCEPT(fmt, ...) \
  debug_prefixed_printf_cond (debug_exceptions, "windows except", fmt, \
			      ## __VA_ARGS__)

using windows_nat::windows_thread_info;

/* A pointer to a function that should return non-zero iff REGNUM
   corresponds to one of the segment registers.  */
using segment_register_p_ftype = int (int regnum);

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

/* Flags that can be passed to windows_continue.  */

enum windows_continue_flag
  {
    /* This means we have killed the inferior, so windows_continue
       should ignore weird errors due to threads shutting down.  */
    WCONT_KILLED = 1,

    /* This means we expect this windows_continue call to be the last
       call to continue the inferior -- we are either mourning it or
       detaching.  */
    WCONT_LAST_CALL = 2,

    /* By default, windows_continue only calls ContinueDebugEvent in
       all-stop mode.  This flag indicates that windows_continue
       should call ContinueDebugEvent even in non-stop mode.  */
    WCONT_CONTINUE_DEBUG_EVENT = 4,

    /* Skip calling ContinueDebugEvent even in all-stop mode.  This is
       the default in non-stop mode.  */
    WCONT_DONT_CONTINUE_DEBUG_EVENT = 8,
  };

DEF_ENUM_FLAGS_TYPE (windows_continue_flag, windows_continue_flags);

/* We want to register windows_thread_info as struct thread_info
   private data.  thread_info::priv must point to a class that
   inherits from private_thread_info.  But we can't make
   windows_thread_info inherit private_thread_info, because
   windows_thread_info is shared with GDBserver.  So we make a new
   class that inherits from both private_thread_info,
   windows_thread_info, and register that one as thread_info::private.
   This multiple inheritance is benign, because private_thread_info is
   a java-style interface class with no data.  */
struct windows_private_thread_info : private_thread_info, windows_thread_info
{
  windows_private_thread_info (windows_nat::windows_process_info *proc,
			       DWORD tid, HANDLE h, CORE_ADDR tlb)
    : windows_thread_info (proc, tid, h, tlb)
  {}

  ~windows_private_thread_info () override
  {}
};

/* Get the windows_thread_info object associated with THR.  */

static inline windows_thread_info *
as_windows_thread_info (thread_info *thr)
{
  /* Cast to windows_private_thread_info, which inherits from
     private_thread_info, and is implicitly convertible to
     windows_thread_info, the return type.  */
  private_thread_info *priv = thr->priv.get ();
  return gdb::checked_static_cast<windows_private_thread_info *> (priv);
}

struct windows_per_inferior : public windows_nat::windows_process_info
{
  windows_thread_info *find_thread (ptid_t ptid) override;
  bool handle_output_debug_string (const DEBUG_EVENT &current_event,
				   struct target_waitstatus *ourstatus) override;
  void handle_load_dll (const char *dll_name, LPVOID base) override;
  void handle_unload_dll (const DEBUG_EVENT &current_event) override;
  bool handle_access_violation (const EXCEPTION_RECORD *rec) override;

  /* Invalidate the thread context.  */
  virtual void invalidate_thread_context (windows_thread_info *th) = 0;

  int windows_initialization_done = 0;

  /* Counts of things.  */
  int saw_create = 0;
  int open_process_used = 0;
#ifdef __x86_64__
  void *wow64_dbgbreak = nullptr;
#endif

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

  thread_control_capabilities get_thread_control_capabilities () override
  { return tc_schedlock; }

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

  const char *extra_thread_info (thread_info *info) override;

  std::string pid_to_str (ptid_t) override;

  void interrupt () override;
  void pass_ctrlc () override;
  void stop (ptid_t ptid) override;

  void thread_events (bool enable) override;

  bool any_resumed_thread ();

  const char *pid_to_exec_file (int pid) override;

  ptid_t get_ada_task_ptid (long lwp, ULONGEST thread) override;

  bool get_tib_address (ptid_t ptid, CORE_ADDR *addr) override;

  const char *thread_name (struct thread_info *) override;

  ptid_t get_windows_debug_event (int pid, struct target_waitstatus *ourstatus,
				  target_wait_flags options,
				  DEBUG_EVENT *current_event);

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

  bool supports_non_stop () override;
  bool always_non_stop_p () override;

  void async (bool enable) override;

  int async_wait_fd () override
  {
    return serial_event_fd (m_wait_event);
  }

  void debug_registers_changed_all_threads ();

protected:

  /* Initialize arch-specific data for a new inferior (debug registers,
     register mappings).  If ATTACHING is true, we're attaching to an
     already-running process.  */
  virtual void initialize_windows_arch (bool attaching) = 0;
  /* Cleanup arch-specific data after inferior exit.  */
  virtual void cleanup_windows_arch () = 0;

  /* Prepare the thread context for continuing.  */
  virtual void thread_context_continue (windows_thread_info *th,
					int killed) = 0;
  /* Set or clear the stepping bit in the thread context.  */
  virtual void thread_context_step (windows_thread_info *th, bool enable) = 0;

  /* Fetches register number R from the given windows_thread_info,
     and supplies its value to the given regcache.

     This function assumes that R is non-negative.  A failed assertion
     is raised if that is not true.

     This function assumes that TH->RELOAD_CONTEXT is not set, meaning
     that the windows_thread_info has an up-to-date context.  A failed
     assertion is raised if that assumption is violated.  */
  virtual void fetch_one_register (struct regcache *regcache,
				   windows_thread_info *th, int r) = 0;

  /* Collect the register number R from the given regcache, and store
     its value into the corresponding area of the given thread's context.

     This function assumes that R is non-negative.  A failed assertion
     assertion is raised if that is not true.  */
  virtual void store_one_register (const struct regcache *regcache,
				   windows_thread_info *th, int r) = 0;

  /* Determine if ER contains a software-breakpoint.  */
  virtual bool is_sw_breakpoint (const EXCEPTION_RECORD *er) const = 0;

private:

  windows_thread_info *add_thread (ptid_t ptid, HANDLE h, void *tlb,
				   bool main_thread_p);
  void delete_thread (ptid_t ptid, DWORD exit_code, bool main_thread_p);
  DWORD fake_create_process (const DEBUG_EVENT &current_event);

  bool stop_one_thread (windows_thread_info *th,
			enum windows_nat::stopping_kind stopping_kind);

  void stop_interrupt (ptid_t ptid, bool stop_on_first_match);
  bool dbg_break_process ();

  DWORD continue_status_for_event_detaching
    (const DEBUG_EVENT &event, size_t *reply_later_events_left = nullptr);

  DWORD prepare_resume (windows_thread_info *th,
			thread_info *tp,
			int step, gdb_signal sig);

  void continue_one_thread (windows_thread_info *th,
			    windows_continue_flags cont_flags);

  BOOL windows_continue (DWORD continue_status, int id,
			 windows_continue_flags cont_flags = 0);

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

  /* This continues the last debug event, dispatching to the worker
     thread as needed.  */
  void continue_last_debug_event_main_thread (const char *context_str,
					      DWORD continue_status,
					      bool last_call = false);

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

  /* Whether target_thread_events is in effect.  */
  bool m_report_thread_events = false;
};

/* Check if Windows API call succeeds, and otherwise print error code
   and description.  */
void check (BOOL ok, const char *file, int line);
#define CHECK(x)	check (x, __FILE__,__LINE__)

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

/* Creates an iterator that works like all_matching_threads_iterator,
   but that returns windows_thread_info pointers instead of
   thread_info.  This could be replaced with a std::range::transform
   when we require C++20.  */
class all_windows_threads_iterator
{
public:
  using self_type = all_windows_threads_iterator;
  using value_type = windows_thread_info;
  using reference = windows_thread_info *&;
  using pointer = windows_thread_info **;
  using iterator_category = std::forward_iterator_tag;
  using difference_type = int;

  explicit all_windows_threads_iterator (all_non_exited_threads_iterator base_iter)
    : m_base_iter (base_iter)
  {}

  windows_thread_info * operator* () const { return as_windows_thread_info (&*m_base_iter); }

  all_windows_threads_iterator &operator++ ()
  {
    ++m_base_iter;
    return *this;
  }

  bool operator== (const all_windows_threads_iterator &other) const
  { return m_base_iter == other.m_base_iter; }

  bool operator!= (const all_windows_threads_iterator &other) const
  { return !(*this == other); }

private:
  all_non_exited_threads_iterator m_base_iter;
};

/* The range for all_windows_threads, below.  */

class all_windows_threads_range : public all_non_exited_threads_range
{
public:
  all_windows_threads_range (all_non_exited_threads_range base_range)
    : m_base_range (base_range)
  {}

  all_windows_threads_iterator begin () const
  { return all_windows_threads_iterator (m_base_range.begin ()); }
  all_windows_threads_iterator end () const
  { return all_windows_threads_iterator (m_base_range.end ()); }

private:
  all_non_exited_threads_range m_base_range;
};

/* Return a range that can be used to walk over all non-exited Windows
   threads of all inferiors, with range-for.  */

static inline all_windows_threads_range
all_windows_threads ()
{
  auto *win_tgt = static_cast<windows_nat_target *> (get_native_target ());
  return (all_windows_threads_range
	  (all_non_exited_threads_range (win_tgt, minus_one_ptid)));
}

extern void windows_debug_registers_changed_all_threads ();

#endif /* GDB_WINDOWS_NAT_H */

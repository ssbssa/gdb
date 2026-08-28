/* Internal interfaces for the Windows code
   Copyright (C) 1995-2026 Free Software Foundation, Inc.

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

#ifndef GDB_NAT_WINDOWS_NAT_H
#define GDB_NAT_WINDOWS_NAT_H

#include <windows.h>
#include <psapi.h>
#include <vector>

#include <optional>
#include "target/waitstatus.h"

#define STATUS_WX86_BREAKPOINT 0x4000001F
#define STATUS_WX86_SINGLE_STEP 0x4000001E

#ifndef CONTEXT_EXTENDED_REGISTERS
/* This macro is only defined on ia32.  It only makes sense on this target,
   so define it as zero if not already defined.  */
#define CONTEXT_EXTENDED_REGISTERS 0
#endif

#define CONTEXT_EXTENDED_REGISTERS_FLAG	0x20
#define CONTEXT_XSTATE_FLAG		0x40

namespace windows_nat
{

struct windows_process_info;

/* The reason for explicitly stopping a thread.  Note the enumerators
   are ordered such that when comparing two stopping_kind's numerical
   value, the highest should prevail.  */
enum stopping_kind
  {
    /* Not really stopping the thread.  */
    SK_NOT_STOPPING = 0,

    /* We're stopping the thread for internal reasons, the stop should
       not be reported as an event to the core.  */
    SK_INTERNAL = 1,

    /* We're stopping the thread for external reasons, meaning, the
       core/user asked us to stop the thread, so we must report a stop
       event to the core.  */
    SK_EXTERNAL = 2,
  };

/* Thread information structure used to track extra information about
   each thread.  */
struct windows_thread_info
{
  windows_thread_info (windows_process_info *proc_,
		       DWORD tid_, HANDLE h_, CORE_ADDR tlb);

  DISABLE_COPY_AND_ASSIGN (windows_thread_info);

  /* Ensure that this thread has been suspended.  */
  void suspend ();

  /* Resume the thread if it has been suspended.  */
  void resume ();

  /* Return the thread's name, or nullptr if not known.  The name is
     stored in this thread and is guaranteed to live until at least
     the next call.  */
  const char *thread_name ();

  /* Read LEN bytes of the thread's $_siginfo into READBUF, starting
     at OFFSET.  Store the number of actually-read bytes in
     XFERED_LEN.  Returns true on success, false on failure.  Passing
     READBUF as NULL indicates that the caller is trying to write to
     $_siginfo, which is a failure case.  */
  bool xfer_siginfo (gdb_byte *readbuf,
		     ULONGEST offset, ULONGEST len,
		     ULONGEST *xfered_len);

#if defined __i386__ || defined __x86_64__
  /* Zero all XState feature areas that were not provided by
     GetThreadContext.  */
  void zero_xstate_features ();
#endif

  /* The process this thread belongs to.  */
  windows_process_info *const proc;

  /* The Win32 thread identifier.  */
  DWORD tid;

  /* The handle to the thread.  */
  HANDLE h;

  /* Thread Information Block address.  */
  CORE_ADDR thread_local_base;

#ifdef __CYGWIN__
  /* These two fields are used to handle Cygwin signals.  When a
     thread is signaled, the "sig" thread inside the Cygwin runtime
     reports the fact to us via a special OutputDebugString message.
     In order to make stepping into a signal handler work, we can only
     resume the "sig" thread when we also resume the target signaled
     thread.  When we intercept a Cygwin signal, we set up a cross
     link between the two threads using the two fields below, so we
     can always identify one from the other.  See the "Cygwin signals"
     description in gdb/windows-nat.c for more.  */

  /* If this thread received a signal, then 'cygwin_sig_thread' points
     to the "sig" thread within the Cygwin runtime.  */
  windows_thread_info *cygwin_sig_thread = nullptr;

  /* If this thread is the Cygwin runtime's "sig" thread, then
     'signaled_thread' points at the thread that received a
     signal.  */
  windows_thread_info *signaled_thread = nullptr;
#endif

  /* If the thread had its event postponed with DBG_REPLY_LATER, when
     we later ResumeThread this thread, WaitForDebugEvent will
     re-report the postponed event.  This field holds the continue
     status value to be automatically passed to ContinueDebugEvent
     when we encounter this re-reported event.  0 if the thread has
     not had its event postponed with DBG_REPLY_LATER. */
  DWORD reply_later = 0;

  /* This keeps track of whether SuspendThread was called on this
     thread.  -1 means there was a failure or that the thread was
     explicitly not suspended, 1 means it was called, and 0 means it
     was not.  */
  int suspended = 0;

  /* This flag indicates whether we are explicitly stopping this
     thread in response to a target_stop request or for
     backend-internal reasons.  This allows distinguishing between
     threads that are explicitly stopped by the debugger and threads
     that are stopped due to other reasons.

     Typically, when we want to stop a thread, we suspend it, enqueue
     a pending GDB_SIGNAL_0 stop status on the thread, and then set
     this flag to true.  However, if the thread has had its event
     previously postponed with DBG_REPLY_LATER, it means that it
     already has an event to report.  In such case, we simply set the
     'stopping' flag without suspending the thread or enqueueing a
     pending stop.  See stop_one_thread.  */
  stopping_kind stopping = SK_NOT_STOPPING;

/* Info about a potential pending stop.

   Sometimes, Windows will report a stop on a thread that has been
   ostensibly suspended.  We believe what happens here is that two
   threads hit a breakpoint simultaneously, and the Windows kernel
   queues the stop events.  However, this can result in the strange
   effect of trying to single step thread A -- leaving all other
   threads suspended -- and then seeing a stop in thread B.  To handle
   this scenario, we queue the "pending" stop here, and then
   process it once the step has completed.  See PR gdb/22992.
   If we do have a pending event, its Windows event info is in
   LAST_EVENT.

   TARGET_WAITKIND_IGNORE if the thread does not have a pending
   stop.  */
  target_waitstatus pending_status;

  /* The last Windows event returned by WaitForDebugEvent for this
     thread.  */
  DEBUG_EVENT last_event {};

  /* The last signal reported for this thread, extracted out of
     last_event.  */
  enum gdb_signal last_sig = GDB_SIGNAL_0;

  /* The context of the thread, including any manipulations.  */
  union
  {
    CONTEXT *context = nullptr;
#ifdef __x86_64__
    WOW64_CONTEXT *wow64_context;
#endif
  };

  /* Whether debug registers changed since we last set CONTEXT back to
     the thread.  */
  bool debug_registers_changed = false;

  /* True if this thread is currently stopped at a software
     breakpoint.  This is used to offset the PC when needed.  */
  bool stopped_at_software_breakpoint = false;

  /* True if we've adjusted the PC after hitting a software
     breakpoint, false otherwise.  This lets us avoid multiple
     adjustments if the registers are read multiple times.  */
  bool pc_adjusted = false;

  /* The name of the thread.  */
  gdb::unique_xmalloc_ptr<char> name;

  /* The buffer for the thread context, including any XState registers if
     available.  */
  gdb::unique_xmalloc_ptr<void> context_buffer;
};

enum handle_exception_result
{
  HANDLE_EXCEPTION_UNHANDLED = 0,
  HANDLE_EXCEPTION_HANDLED,
  HANDLE_EXCEPTION_IGNORED
};

/* A single Windows process.  An object of this type (or subclass) is
   created by the client.  Some methods must be provided by the client
   as well.  */

struct windows_process_info
{
  /* The process handle */
  HANDLE handle = 0;
  DWORD process_id = 0;
  DWORD main_thread_id = 0;

#ifdef __CYGWIN__
  /* True if the inferior was created through Cygwin's spawn path
     (i.e., its Cygwin pinfo has PID_CYGPARENT set).  We need this at
     exit time, but we cache it early when we start debugging the
     inferior, because by exit time the inferior's Cygwin pinfo may
     have been torn down (CW_GETPINFO returns NULL).  */
  bool started_by_cygwin = false;

  /* True if cygwin1.dll is loaded into the inferior.  */
  bool cygwin1_dll_loaded = false;

  /* If DLL_PATH is cygwin1.dll, set cygwin1_dll_loaded to true.  */
  void maybe_note_cygwin1_dll (const char *dll_path);
#else
  void maybe_note_cygwin1_dll (const char *) {}
#endif

#ifdef __x86_64__
  /* The target is a WOW64 process */
  bool wow64_process = false;
  /* Ignore first breakpoint exception of WOW64 process */
  bool ignore_first_breakpoint = false;
#endif

  /* Find a thread record given a thread id.

     This function must be supplied by the embedding application.  */
  virtual windows_thread_info *find_thread (ptid_t ptid) = 0;

  /* Fill in the thread's CONTEXT/WOW64_CONTEXT, if it wasn't filled
     in yet.

     This function must be supplied by the embedding application.  */
  virtual void fill_thread_context (windows_thread_info *th) = 0;

  /* Handle OUTPUT_DEBUG_STRING_EVENT from child process.  Updates
     OURSTATUS and returns true if this represents a Cygwin signal,
     otherwise false.

     Cygwin prepends its messages with a "cygwin:".  Interpret this as
     a Cygwin signal.  Otherwise just print the string as a warning.

     This function must be supplied by the embedding application.  */
  virtual bool handle_output_debug_string (const DEBUG_EVENT &current_event,
					   struct target_waitstatus *ourstatus) = 0;

  /* Handle a DLL load event.

     This function assumes that the current event did not occur during
     inferior initialization.

     DLL_NAME is the name of the library.  BASE is the base load
     address.

     This function must be supplied by the embedding application.  */

  virtual void handle_load_dll (const char *dll_name, LPVOID base) = 0;

  /* Handle a DLL unload event.

     This function assumes that this event did not occur during inferior
     initialization.

     This function must be supplied by the embedding application.  */

  virtual void handle_unload_dll (const DEBUG_EVENT &current_event) = 0;

  /* When EXCEPTION_ACCESS_VIOLATION is processed, we give the embedding
     application a chance to change it to be considered "unhandled".
     This function must be supplied by the embedding application.  If it
     returns true, then the exception is "unhandled".  */

  virtual bool handle_access_violation (const EXCEPTION_RECORD *rec) = 0;

  handle_exception_result handle_exception
      (DEBUG_EVENT &current_event,
       struct target_waitstatus *ourstatus, bool debug_exceptions);

  /* Call to indicate that a DLL was loaded.  */

  void dll_loaded_event (const DEBUG_EVENT &current_event);

  /* Iterate over all DLLs currently mapped by our inferior, and
     add them to our list of solibs.  */

  void add_all_dlls ();

  const char *pid_to_exec_file (int);

  template<typename Function>
  auto with_context (windows_thread_info *th, Function function)
  {
#ifdef __x86_64__
    if (wow64_process)
      return function (th != nullptr ? th->wow64_context : nullptr);
    else
#endif
      return function (th != nullptr ? th->context : nullptr);
  }

  DWORD *context_flags_ptr (windows_thread_info *th)
  {
    return with_context (th, [] (auto *context)
      {
	return &context->ContextFlags;
      });
  }

  /* Convert an EXIT_PROCESS_DEBUG_EVENT payload to a target wait
     status.  */

  target_waitstatus exit_process_to_target_status
    (const EXIT_PROCESS_DEBUG_INFO &info);

private:

  /* Handle MS_VC_EXCEPTION when processing a stop.  MS_VC_EXCEPTION is
     somewhat undocumented but is used to tell the debugger the name of
     a thread.

     Return true if the exception was handled; return false otherwise.  */

  bool handle_ms_vc_exception (const DEBUG_EVENT &current_event);

  /* Iterate over all DLLs currently mapped by our inferior, looking for
     a DLL which is loaded at LOAD_ADDR.  If found, add the DLL to our
     list of solibs; otherwise do nothing.  LOAD_ADDR NULL means add all
     DLLs to the list of solibs; this is used when the inferior finishes
     its initialization, and all the DLLs it statically depends on are
     presumed loaded.  */

  void add_dll (LPVOID load_addr);

  /* Try to determine the executable filename.

     EXE_NAME_RET is a pointer to a buffer whose size is EXE_NAME_MAX_LEN.

     Upon success, the filename is stored inside EXE_NAME_RET, and
     this function returns nonzero.

     Otherwise, this function returns zero and the contents of
     EXE_NAME_RET is undefined.  */

  int get_exec_module_filename (char *exe_name_ret, size_t exe_name_max_len);
};

#ifdef __CYGWIN__
/* Return true if the process with native Windows pid WINPID was
   started by a Cygwin parent -- that is, its Cygwin pinfo exists and
   has PID_CYGPARENT set.  Returns false if the process is not a
   Cygwin process at all, or if its parent is not a Cygwin process.

   ATTACHING indicates whether GDB is attaching to an already-running
   inferior (true) or has just launched it via CreateProcess
   (false).  */
extern bool inferior_started_by_cygwin (DWORD winpid, bool attaching);
#endif

/* Return a string version of EVENT_CODE.  */

extern std::string event_code_to_string (DWORD event_code);

/* A simple wrapper for ContinueDebugEvent that continues the last
   waited-for event.  If DEBUG_EVENTS is true, logging will be
   enabled.  */

extern BOOL continue_last_debug_event (DWORD continue_status,
				       bool debug_events);

/* Return the ptid_t of the thread that the last waited-for event was
   for.  */

extern ptid_t get_last_debug_event_ptid ();

/* A simple wrapper for WaitForDebugEvent that also sets the internal
   'last_wait_event' on success.  */

extern BOOL wait_for_debug_event (DEBUG_EVENT *event, DWORD timeout);

/* Wrappers for CreateProcess.  These exist primarily so that the
   "disable randomization" feature can be implemented in a single
   place.  */

extern BOOL create_process (const char *image, char *command_line,
			    DWORD flags, void *environment,
			    const char *cur_dir,
			    bool no_randomization,
			    STARTUPINFOA *startup_info,
			    PROCESS_INFORMATION *process_info);
#ifdef __CYGWIN__
extern BOOL create_process (const wchar_t *image, wchar_t *command_line,
			    DWORD flags, void *environment,
			    const wchar_t *cur_dir,
			    bool no_randomization,
			    STARTUPINFOW *startup_info,
			    PROCESS_INFORMATION *process_info);
#endif /* __CYGWIN__ */

#define AdjustTokenPrivileges		dyn_AdjustTokenPrivileges
#define DebugActiveProcessStop		dyn_DebugActiveProcessStop
#define DebugBreakProcess		dyn_DebugBreakProcess
#define DebugSetProcessKillOnExit	dyn_DebugSetProcessKillOnExit
#undef EnumProcessModules
#define EnumProcessModules		dyn_EnumProcessModules
#undef EnumProcessModulesEx
#define EnumProcessModulesEx		dyn_EnumProcessModulesEx
#undef GetModuleInformation
#define GetModuleInformation		dyn_GetModuleInformation
#undef GetModuleFileNameExA
#define GetModuleFileNameExA		dyn_GetModuleFileNameExA
#undef GetModuleFileNameExW
#define GetModuleFileNameExW		dyn_GetModuleFileNameExW
#define LookupPrivilegeValueA		dyn_LookupPrivilegeValueA
#define OpenProcessToken		dyn_OpenProcessToken
#define GetConsoleFontSize		dyn_GetConsoleFontSize
#define GetCurrentConsoleFont		dyn_GetCurrentConsoleFont
#define Wow64SuspendThread		dyn_Wow64SuspendThread
#define Wow64GetThreadContext		dyn_Wow64GetThreadContext
#define Wow64SetThreadContext		dyn_Wow64SetThreadContext
#define Wow64GetThreadSelectorEntry	dyn_Wow64GetThreadSelectorEntry
#define GenerateConsoleCtrlEvent	dyn_GenerateConsoleCtrlEvent
#define InitializeProcThreadAttributeList dyn_InitializeProcThreadAttributeList
#define UpdateProcThreadAttribute dyn_UpdateProcThreadAttribute
#define DeleteProcThreadAttributeList dyn_DeleteProcThreadAttributeList
#define GetEnabledXStateFeatures	dyn_GetEnabledXStateFeatures
#define InitializeContext		dyn_InitializeContext
#define GetXStateFeaturesMask		dyn_GetXStateFeaturesMask
#define SetXStateFeaturesMask		dyn_SetXStateFeaturesMask
#define LocateXStateFeature		dyn_LocateXStateFeature
#define RtlGetExtendedFeaturesMask	dyn_RtlGetExtendedFeaturesMask
#define RtlSetExtendedFeaturesMask	dyn_RtlSetExtendedFeaturesMask
#define RtlLocateExtendedFeature	dyn_RtlLocateExtendedFeature

typedef BOOL WINAPI (AdjustTokenPrivileges_ftype) (HANDLE, BOOL,
						   PTOKEN_PRIVILEGES,
						   DWORD, PTOKEN_PRIVILEGES,
						   PDWORD);
extern AdjustTokenPrivileges_ftype *AdjustTokenPrivileges;

typedef BOOL WINAPI (DebugActiveProcessStop_ftype) (DWORD);
extern DebugActiveProcessStop_ftype *DebugActiveProcessStop;

typedef BOOL WINAPI (DebugBreakProcess_ftype) (HANDLE);
extern DebugBreakProcess_ftype *DebugBreakProcess;

typedef BOOL WINAPI (DebugSetProcessKillOnExit_ftype) (BOOL);
extern DebugSetProcessKillOnExit_ftype *DebugSetProcessKillOnExit;

typedef BOOL WINAPI (EnumProcessModules_ftype) (HANDLE, HMODULE *, DWORD,
						LPDWORD);
extern EnumProcessModules_ftype *EnumProcessModules;

#ifdef __x86_64__
typedef BOOL WINAPI (EnumProcessModulesEx_ftype) (HANDLE, HMODULE *, DWORD,
						  LPDWORD, DWORD);
extern EnumProcessModulesEx_ftype *EnumProcessModulesEx;
#endif

typedef BOOL WINAPI (GetModuleInformation_ftype) (HANDLE, HMODULE,
						  LPMODULEINFO, DWORD);
extern GetModuleInformation_ftype *GetModuleInformation;

typedef DWORD WINAPI (GetModuleFileNameExA_ftype) (HANDLE, HMODULE, LPSTR,
						  DWORD);
extern GetModuleFileNameExA_ftype *GetModuleFileNameExA;

typedef DWORD WINAPI (GetModuleFileNameExW_ftype) (HANDLE, HMODULE,
						   LPWSTR, DWORD);
extern GetModuleFileNameExW_ftype *GetModuleFileNameExW;

typedef BOOL WINAPI (LookupPrivilegeValueA_ftype) (LPCSTR, LPCSTR, PLUID);
extern LookupPrivilegeValueA_ftype *LookupPrivilegeValueA;

typedef BOOL WINAPI (OpenProcessToken_ftype) (HANDLE, DWORD, PHANDLE);
extern OpenProcessToken_ftype *OpenProcessToken;

typedef BOOL WINAPI (GetCurrentConsoleFont_ftype) (HANDLE, BOOL,
						   CONSOLE_FONT_INFO *);
extern GetCurrentConsoleFont_ftype *GetCurrentConsoleFont;

typedef COORD WINAPI (GetConsoleFontSize_ftype) (HANDLE, DWORD);
extern GetConsoleFontSize_ftype *GetConsoleFontSize;

#ifdef __x86_64__
typedef DWORD WINAPI (Wow64SuspendThread_ftype) (HANDLE);
extern Wow64SuspendThread_ftype *Wow64SuspendThread;

typedef BOOL WINAPI (Wow64GetThreadContext_ftype) (HANDLE, PWOW64_CONTEXT);
extern Wow64GetThreadContext_ftype *Wow64GetThreadContext;

typedef BOOL WINAPI (Wow64SetThreadContext_ftype) (HANDLE,
						   const WOW64_CONTEXT *);
extern Wow64SetThreadContext_ftype *Wow64SetThreadContext;

typedef BOOL WINAPI (Wow64GetThreadSelectorEntry_ftype) (HANDLE, DWORD,
							 PLDT_ENTRY);
extern Wow64GetThreadSelectorEntry_ftype *Wow64GetThreadSelectorEntry;
#endif

typedef BOOL WINAPI (GenerateConsoleCtrlEvent_ftype) (DWORD, DWORD);
extern GenerateConsoleCtrlEvent_ftype *GenerateConsoleCtrlEvent;

/* We use a local typedef for this type to avoid depending on
   Windows 8.  */
using gdb_lpproc_thread_attribute_list = void *;

typedef BOOL WINAPI (InitializeProcThreadAttributeList_ftype)
     (gdb_lpproc_thread_attribute_list lpAttributeList,
      DWORD dwAttributeCount, DWORD dwFlags, PSIZE_T lpSize);
extern InitializeProcThreadAttributeList_ftype *InitializeProcThreadAttributeList;

typedef BOOL WINAPI (UpdateProcThreadAttribute_ftype)
     (gdb_lpproc_thread_attribute_list lpAttributeList,
      DWORD dwFlags, DWORD_PTR Attribute, PVOID lpValue, SIZE_T cbSize,
      PVOID lpPreviousValue, PSIZE_T lpReturnSize);
extern UpdateProcThreadAttribute_ftype *UpdateProcThreadAttribute;

typedef void WINAPI (DeleteProcThreadAttributeList_ftype)
     (gdb_lpproc_thread_attribute_list lpAttributeList);
extern DeleteProcThreadAttributeList_ftype *DeleteProcThreadAttributeList;

/* Return true if it's possible to disable randomization on this
   host.  */

extern bool disable_randomization_available ();

#if defined __i386__ || defined __x86_64__
typedef DWORD64 (WINAPI GetEnabledXStateFeatures_ftype) ();
extern GetEnabledXStateFeatures_ftype *GetEnabledXStateFeatures;

typedef BOOL (WINAPI InitializeContext_ftype) (PVOID, DWORD,
					       PCONTEXT*, PDWORD);
extern InitializeContext_ftype *InitializeContext;

typedef BOOL (WINAPI GetXStateFeaturesMask_ftype) (PCONTEXT, PDWORD64);
extern GetXStateFeaturesMask_ftype *GetXStateFeaturesMask;

typedef BOOL (WINAPI SetXStateFeaturesMask_ftype) (PCONTEXT, DWORD64);
extern SetXStateFeaturesMask_ftype *SetXStateFeaturesMask;

typedef PVOID (WINAPI LocateXStateFeature_ftype) (PCONTEXT, DWORD, PDWORD);
extern LocateXStateFeature_ftype *LocateXStateFeature;

#ifdef __x86_64__
typedef DWORD64 (WINAPI RtlGetExtendedFeaturesMask_ftype) (PVOID);
extern RtlGetExtendedFeaturesMask_ftype *RtlGetExtendedFeaturesMask;

typedef VOID (WINAPI RtlSetExtendedFeaturesMask_ftype) (PVOID, DWORD64);
extern RtlSetExtendedFeaturesMask_ftype *RtlSetExtendedFeaturesMask;

typedef PVOID (WINAPI RtlLocateExtendedFeature_ftype) (PVOID, DWORD, PDWORD);
extern RtlLocateExtendedFeature_ftype *RtlLocateExtendedFeature;
#endif
#endif

/* Helper classes to get the correct ContextFlags values based on the
   used type (CONTEXT or WOW64_CONTEXT).  */

template<typename Context>
struct WindowsContext;

template<>
struct WindowsContext<CONTEXT *>
{
  static constexpr DWORD control  = CONTEXT_CONTROL;
  static constexpr DWORD floating = CONTEXT_FLOATING_POINT;
  static constexpr DWORD debug    = CONTEXT_DEBUG_REGISTERS;
  static constexpr DWORD extended = CONTEXT_EXTENDED_REGISTERS;
  static constexpr DWORD full	  = CONTEXT_FULL;
  static constexpr DWORD all	  = (CONTEXT_FULL
				     | CONTEXT_FLOATING_POINT
#ifdef CONTEXT_SEGMENTS
				     | CONTEXT_SEGMENTS
#endif
				     | CONTEXT_DEBUG_REGISTERS
				     | CONTEXT_EXTENDED_REGISTERS);
};

#ifdef __x86_64__
template<>
struct WindowsContext<WOW64_CONTEXT *>
{
  static constexpr DWORD control  = WOW64_CONTEXT_CONTROL;
  static constexpr DWORD floating = WOW64_CONTEXT_FLOATING_POINT;
  static constexpr DWORD debug	  = WOW64_CONTEXT_DEBUG_REGISTERS;
  static constexpr DWORD extended = WOW64_CONTEXT_EXTENDED_REGISTERS;
  static constexpr DWORD full	  = WOW64_CONTEXT_FULL;
  static constexpr DWORD all	  = WOW64_CONTEXT_ALL;
};
#endif

/* Overloaded helper functions to call the correct function based on the used
   type (CONTEXT or WOW64_CONTEXT).  */

static inline BOOL
get_thread_context (HANDLE h, CONTEXT *context)
{
  return GetThreadContext (h, context);
}

static inline BOOL
set_thread_context (HANDLE h, CONTEXT *context)
{
  return SetThreadContext (h, context);
}

static inline BOOL
get_thread_selector_entry (CONTEXT *, HANDLE thread, DWORD sel,
			   LDT_ENTRY *info)
{
  return GetThreadSelectorEntry (thread, sel, info);
}

static inline BOOL
enum_process_modules (CONTEXT *, HANDLE process,
		      HMODULE *modules, DWORD size, LPDWORD needed)
{
  return EnumProcessModules (process, modules, size, needed);
}

#if defined __i386__ || defined __x86_64__
static inline BOOL
get_xstate_features_mask (CONTEXT *context, DWORD64 *mask)
{
  return GetXStateFeaturesMask (context, mask);
}

static inline BOOL
set_xstate_features_mask (CONTEXT *context, DWORD64 mask)
{
  return SetXStateFeaturesMask (context, mask);
}

static inline PVOID
locate_xstate_feature (CONTEXT *context, DWORD feature, DWORD *length)
{
  return LocateXStateFeature (context, feature, length);
}
#endif

#ifdef __x86_64__
static inline BOOL
get_thread_context (HANDLE h, WOW64_CONTEXT *context)
{
  if ((context->ContextFlags & CONTEXT_XSTATE_FLAG) != 0)
    {
      /* Wow64GetThreadContext doesn't handle CONTEXT_EXTENDED_REGISTERS and
	 CONTEXT_XSTATE combined correctly, but separate they work fine.  */
      DWORD flags = context->ContextFlags;
      context->ContextFlags &= ~CONTEXT_EXTENDED_REGISTERS_FLAG;
      BOOL ret = Wow64GetThreadContext (h, context);
      context->ContextFlags = flags;
      if (!ret)
	return FALSE;

      context->ContextFlags &= ~CONTEXT_XSTATE_FLAG;
      ret = Wow64GetThreadContext (h, context);
      context->ContextFlags = flags;
      return ret;
    }

  return Wow64GetThreadContext (h, context);
}

static inline BOOL
set_thread_context (HANDLE h, WOW64_CONTEXT *context)
{
  if ((context->ContextFlags & CONTEXT_XSTATE_FLAG) != 0)
    {
      /* Same limitation as Wow64GetThreadContext above.  */
      DWORD flags = context->ContextFlags;
      context->ContextFlags &= ~CONTEXT_EXTENDED_REGISTERS_FLAG;
      BOOL ret = Wow64SetThreadContext (h, context);
      context->ContextFlags = flags;
      if (!ret)
	return FALSE;

      context->ContextFlags &= ~CONTEXT_XSTATE_FLAG;
      ret = Wow64SetThreadContext (h, context);
      context->ContextFlags = flags;
      return ret;
    }

  return Wow64SetThreadContext (h, context);
}

static inline BOOL
get_thread_selector_entry (WOW64_CONTEXT *, HANDLE thread, DWORD sel,
			   LDT_ENTRY *info)
{
  return Wow64GetThreadSelectorEntry (thread, sel, info);
}

static inline BOOL
enum_process_modules (WOW64_CONTEXT *, HANDLE process,
		      HMODULE *modules, DWORD size, LPDWORD needed)
{
  return EnumProcessModulesEx (process, modules, size, needed,
			       LIST_MODULES_32BIT);
}

static inline BOOL
get_xstate_features_mask (WOW64_CONTEXT *context, DWORD64 *mask)
{
  /* Use lower level function, since there is no Wow64GetXStateFeaturesMask.  */
  *mask = RtlGetExtendedFeaturesMask (context + 1);
  return TRUE;
}

static inline BOOL
set_xstate_features_mask (WOW64_CONTEXT *context, DWORD64 mask)
{
  /* Use lower level function, since there is no Wow64SetXStateFeaturesMask.  */
  RtlSetExtendedFeaturesMask (context + 1, mask);
  return TRUE;
}

static inline PVOID
locate_xstate_feature (WOW64_CONTEXT *context, DWORD feature, DWORD *length)
{
  /* Use lower level function, since there is no Wow64LocateXStateFeature.  */
  return RtlLocateExtendedFeature (context + 1, feature, length);
}
#endif

#if defined __i386__ || defined __x86_64__
/* Available XState features.  */
extern DWORD64 xstate_features;
#endif

/* This is available starting with Windows 10.  */
#ifndef DBG_REPLY_LATER
# define DBG_REPLY_LATER 0x40010001L
#endif

/* Return true if it's possible to use DBG_REPLY_LATER with
   ContinueDebugEvent on this host.  */
extern bool dbg_reply_later_available ();

/* Load any functions which may not be available in ancient versions
   of Windows.  */

extern bool initialize_loadable ();

}

#endif /* GDB_NAT_WINDOWS_NAT_H */

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

#include "nat/windows-nat.h"
#include "gdbsupport/common-debug.h"
#include "gdbsupport/gdb_signals.h"
#include "gdbsupport/gdb_wait.h"
#include "gdbsupport/x86-xstate.h"
#include "gdbsupport/xml-utils.h"
#include "target/target.h"
#include <winternl.h>

#undef GetModuleFileNameEx

#ifndef __CYGWIN__
#define GetModuleFileNameEx GetModuleFileNameExA
#else
#include <sys/cygwin.h>
#define __USEWIDE
#define GetModuleFileNameEx GetModuleFileNameExW
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH 0xc0000004
#endif

namespace windows_nat
{

/* The most recent event from WaitForDebugEvent.  Unlike
   current_event, this is guaranteed never to come from a pending
   stop.  This is important because only data from the most recent
   event from WaitForDebugEvent can be used when calling
   ContinueDebugEvent.  */
static DEBUG_EVENT last_wait_event;

AdjustTokenPrivileges_ftype *AdjustTokenPrivileges;
DebugActiveProcessStop_ftype *DebugActiveProcessStop;
DebugBreakProcess_ftype *DebugBreakProcess;
DebugSetProcessKillOnExit_ftype *DebugSetProcessKillOnExit;
EnumProcessModules_ftype *EnumProcessModules;
#ifdef __x86_64__
EnumProcessModulesEx_ftype *EnumProcessModulesEx;
#endif
GetModuleInformation_ftype *GetModuleInformation;
GetModuleFileNameExA_ftype *GetModuleFileNameExA;
GetModuleFileNameExW_ftype *GetModuleFileNameExW;
LookupPrivilegeValueA_ftype *LookupPrivilegeValueA;
OpenProcessToken_ftype *OpenProcessToken;
GetCurrentConsoleFont_ftype *GetCurrentConsoleFont;
GetConsoleFontSize_ftype *GetConsoleFontSize;
#ifdef __x86_64__
Wow64SuspendThread_ftype *Wow64SuspendThread;
Wow64GetThreadContext_ftype *Wow64GetThreadContext;
Wow64SetThreadContext_ftype *Wow64SetThreadContext;
Wow64GetThreadSelectorEntry_ftype *Wow64GetThreadSelectorEntry;
#endif
GenerateConsoleCtrlEvent_ftype *GenerateConsoleCtrlEvent;

#define GetThreadDescription dyn_GetThreadDescription
typedef HRESULT WINAPI (GetThreadDescription_ftype) (HANDLE, PWSTR *);
static GetThreadDescription_ftype *GetThreadDescription;

#define IsWow64Process2 dyn_IsWow64Process2
typedef BOOL WINAPI (IsWow64Process2_ftype) (HANDLE, USHORT *, USHORT *);
static IsWow64Process2_ftype *IsWow64Process2;

#define GetProcessInformation dyn_GetProcessInformation
typedef BOOL WINAPI (GetProcessInformation_ftype) (HANDLE,
						   PROCESS_INFORMATION_CLASS,
						   LPVOID, DWORD);
static GetProcessInformation_ftype *GetProcessInformation;

InitializeProcThreadAttributeList_ftype *InitializeProcThreadAttributeList;
UpdateProcThreadAttribute_ftype *UpdateProcThreadAttribute;
DeleteProcThreadAttributeList_ftype *DeleteProcThreadAttributeList;

#if defined __i386__ || defined __x86_64__
GetEnabledXStateFeatures_ftype *GetEnabledXStateFeatures;
InitializeContext_ftype *InitializeContext;
GetXStateFeaturesMask_ftype *GetXStateFeaturesMask;
SetXStateFeaturesMask_ftype *SetXStateFeaturesMask;
LocateXStateFeature_ftype *LocateXStateFeature;
#ifdef __x86_64__
RtlGetExtendedFeaturesMask_ftype *RtlGetExtendedFeaturesMask;
RtlSetExtendedFeaturesMask_ftype *RtlSetExtendedFeaturesMask;
RtlLocateExtendedFeature_ftype *RtlLocateExtendedFeature;
#endif

DWORD64 xstate_features;
#endif

/* Note that 'debug_events' must be locally defined in the relevant
   functions.  */
#define DEBUG_EVENTS(fmt, ...) \
  debug_prefixed_printf_cond (debug_events, "windows events", fmt, \
			      ## __VA_ARGS__)

windows_thread_info::windows_thread_info (windows_process_info *proc_,
					  DWORD tid_, HANDLE h_, CORE_ADDR tlb)
  : proc (proc_),
    tid (tid_),
    h (h_),
    thread_local_base (tlb)
{
#if defined __i386__ || defined __x86_64__
  if (xstate_features != 0)
    {
      DWORD context_flags = proc->with_context (nullptr, [] (auto *context)
	{
	  return WindowsContext<decltype(context)>::all;
	});
      context_flags |= CONTEXT_XSTATE_FLAG;
      DWORD xstate_size = 0;
      InitializeContext (NULL, context_flags, NULL, &xstate_size);
      context_buffer.reset (xmalloc (xstate_size));
      CONTEXT *c = nullptr;
      if (!InitializeContext (context_buffer.get (),
			      context_flags, &c, &xstate_size))
	{
	  unsigned err = (unsigned) GetLastError ();
	  throw_winerror_with_name (_("InitializeContext failed"), err);
	}
#ifdef __x86_64__
      /* InitializeContext actually initializes a WOW64_CONTEXT when
	 context_flags contains a WOW64_CONTEXT_* value, so a cast is needed.
	 */
      if (proc->wow64_process)
	wow64_context = (WOW64_CONTEXT *) c;
      else
#endif
	context = c;
    }
#ifdef __x86_64__
  else if (proc->wow64_process)
    {
      context_buffer.reset (xmalloc (sizeof (WOW64_CONTEXT)));
      wow64_context = (WOW64_CONTEXT *) context_buffer.get ();
    }
#endif
  else
#endif
    {
      context_buffer.reset (xmalloc (sizeof (CONTEXT)));
      context = (CONTEXT *) context_buffer.get ();
    }
  *proc->context_flags_ptr (this) = 0;
}

void
windows_thread_info::suspend ()
{
  if (suspended != 0)
    return;

  if (SuspendThread (h) == (DWORD) -1)
    {
      DWORD err = GetLastError ();

      /* We get Access Denied (5) when trying to suspend
	 threads that Windows started on behalf of the
	 debuggee, usually when those threads are just
	 about to exit.
	 We can get Invalid Handle (6) if the main thread
	 has exited.  */
      if (err != ERROR_INVALID_HANDLE && err != ERROR_ACCESS_DENIED)
	warning (_("SuspendThread (tid=0x%x) failed. (winerr %u: %s)"),
		 (unsigned) tid, (unsigned) err, strwinerror (err));
      suspended = -1;
    }
  else
    suspended = 1;
}

void
windows_thread_info::resume ()
{
  if (suspended > 0)
    {
      stopped_at_software_breakpoint = false;

      if (ResumeThread (h) == (DWORD) -1)
	{
	  DWORD err = GetLastError ();
	  warning (_("warning: ResumeThread (tid=0x%x) failed. (winerr %u: %s)"),
		   (unsigned) tid, (unsigned) err, strwinerror (err));
	}
    }
  suspended = 0;
}

const char *
windows_thread_info::thread_name ()
{
  if (GetThreadDescription != nullptr)
    {
      PWSTR value;
      HRESULT result = GetThreadDescription (h, &value);
      if (SUCCEEDED (result))
	{
	  int needed = WideCharToMultiByte (CP_ACP, 0, value, -1, nullptr, 0,
					    nullptr, nullptr);
	  if (needed != 0)
	    {
	      /* USED_DEFAULT is how we detect that the encoding
		 conversion had to fall back to the substitution
		 character.  It seems better to just reject bad
		 conversions here.  */
	      BOOL used_default = FALSE;
	      gdb::unique_xmalloc_ptr<char> new_name
		((char *) xmalloc (needed));
	      if (WideCharToMultiByte (CP_ACP, 0, value, -1,
				       new_name.get (), needed,
				       nullptr, &used_default) == needed
		  && !used_default
		  && strlen (new_name.get ()) > 0)
		name = std::move (new_name);
	    }
	  LocalFree (value);
	}
    }

  return name.get ();
}

/* Read Windows signal info.  See nat/windows-nat.h.  */

bool
windows_thread_info::xfer_siginfo (gdb_byte *readbuf,
				   ULONGEST offset, ULONGEST len,
				   ULONGEST *xfered_len)
{
  if (last_event.dwDebugEventCode != EXCEPTION_DEBUG_EVENT)
    return false;

  if (readbuf == nullptr)
    return false;

  EXCEPTION_RECORD &er = last_event.u.Exception.ExceptionRecord;

  char *buf = (char *) &er;
  size_t bufsize = sizeof (er);

#ifdef __x86_64__
  EXCEPTION_RECORD32 er32;
  if (proc->wow64_process)
    {
      buf = (char *) &er32;
      bufsize = sizeof (er32);

      er32.ExceptionCode = er.ExceptionCode;
      er32.ExceptionFlags = er.ExceptionFlags;
      er32.ExceptionRecord
	= (uintptr_t) er.ExceptionRecord;
      er32.ExceptionAddress
	= (uintptr_t) er.ExceptionAddress;
      er32.NumberParameters = er.NumberParameters;
      for (int i = 0; i < EXCEPTION_MAXIMUM_PARAMETERS; i++)
	er32.ExceptionInformation[i] = er.ExceptionInformation[i];
    }
#endif

  if (offset > bufsize)
    return false;

  if (offset + len > bufsize)
    len = bufsize - offset;

  memcpy (readbuf, buf + offset, len);
  *xfered_len = len;

  return true;
}

#if defined __i386__ || defined __x86_64__
/* See nat/windows-nat.h.  */

void
windows_thread_info::zero_xstate_features ()
{
  if (xstate_features == 0)
    return;

  proc->with_context (this, [] (auto *context)
    {
      DWORD64 features = 0;
      if (!get_xstate_features_mask (context, &features))
	return;

      DWORD64 zeroed_features = xstate_features & ~features;
      if (zeroed_features == 0)
	return;

      for (int f = X86_XSTATE_AVX_ID; f <= X86_XSTATE_ZMM_ID; f++)
	{
	  DWORD64 flag = 1ULL << f;
	  if ((zeroed_features & flag) != 0)
	    {
	      DWORD size = 0;
	      void *loc = locate_xstate_feature (context, f, &size);
	      if (loc != nullptr && size > 0)
		memset (loc, 0, size);
	    }
	}
    });
}
#endif

/* Try to determine the executable filename.

   EXE_NAME_RET is a pointer to a buffer whose size is EXE_NAME_MAX_LEN.

   Upon success, the filename is stored inside EXE_NAME_RET, and
   this function returns nonzero.

   Otherwise, this function returns zero and the contents of
   EXE_NAME_RET is undefined.  */

int
windows_process_info::get_exec_module_filename (char *exe_name_ret,
						size_t exe_name_max_len)
{
  DWORD len;
  HMODULE dh_buf;
  DWORD cbNeeded;

  cbNeeded = 0;
  BOOL ret = with_context (nullptr, [&] (auto *context)
    {
      return enum_process_modules (context, handle, &dh_buf,
				   sizeof (HMODULE), &cbNeeded);
    });
  if (!ret || !cbNeeded)
    return 0;

  /* We know the executable is always first in the list of modules,
     which we just fetched.  So no need to fetch more.  */

#ifdef __CYGWIN__
  {
    /* Cygwin prefers that the path be in /x/y/z format, so extract
       the filename into a temporary buffer first, and then convert it
       to POSIX format into the destination buffer.  */
    wchar_t *pathbuf = (wchar_t *) alloca (exe_name_max_len * sizeof (wchar_t));

    len = GetModuleFileNameEx (handle,
			       dh_buf, pathbuf, exe_name_max_len);
    if (len == 0)
      {
	unsigned err = (unsigned) GetLastError ();
	throw_winerror_with_name (_("Error getting executable filename"), err);
      }
    if (cygwin_conv_path (CCP_WIN_W_TO_POSIX, pathbuf, exe_name_ret,
			  exe_name_max_len) < 0)
      error (_("Error converting executable filename to POSIX: %d."), errno);
  }
#else
  len = GetModuleFileNameEx (handle,
			     dh_buf, exe_name_ret, exe_name_max_len);
  if (len == 0)
    {
      unsigned err = (unsigned) GetLastError ();
      throw_winerror_with_name (_("Error getting executable filename"), err);
    }
#endif

    return 1;	/* success */
}

const char *
windows_process_info::pid_to_exec_file (int pid)
{
  static char path[MAX_PATH];
#ifdef __CYGWIN__
  /* Try to find exe name as symlink target of /proc/<pid>/exe.  */
  int nchars;
  char procexe[sizeof ("/proc/4294967295/exe")];

  xsnprintf (procexe, sizeof (procexe), "/proc/%u/exe", pid);
  nchars = readlink (procexe, path, sizeof(path));
  if (nchars > 0 && nchars < sizeof (path))
    {
      path[nchars] = '\0';	/* Got it */
      return path;
    }
#endif

  /* If we get here then either Cygwin is hosed, this isn't a Cygwin version
     of gdb, or we're trying to debug a non-Cygwin windows executable.  */
  if (!get_exec_module_filename (path, sizeof (path)))
    path[0] = '\0';

  return path;
}

static std::string
wcstostring (const wchar_t *in, int len)
{
  std::string out;
  if (in != nullptr && (len > 0 || (len < 0 && in[0])))
    {
      if (len < 0)
	len = wcslen (in);
      int needed = WideCharToMultiByte (CP_ACP, 0, in, len, nullptr, 0,
					nullptr, nullptr);
      if (needed > 0)
	{
	  out.resize (needed + 1);
	  WideCharToMultiByte (CP_ACP, 0, in, len, out.data (), needed,
			       nullptr, nullptr);
	}
    }
  return out;
}

static std::string
process_user (HANDLE process)
{
  std::string user;
  HANDLE token;
  if (OpenProcessToken (process, TOKEN_QUERY, &token))
  {
    DWORD tu_size = 0;
    char name[256];
    DWORD name_size = sizeof (name);
    SID_NAME_USE sid;
    if (!GetTokenInformation (token, TokenUser,
			      NULL, 0, &tu_size)
	&& GetLastError () == ERROR_INSUFFICIENT_BUFFER)
      {
	gdb::unique_xmalloc_ptr<TOKEN_USER> tu
	  ((TOKEN_USER *) xmalloc (tu_size));
	if (GetTokenInformation (token, TokenUser,
				 tu.get (), tu_size, &tu_size)
	    && LookupAccountSidA (NULL, tu->User.Sid, name,
				  &name_size, NULL, &tu_size, &sid))
	  user = name;
      }

    CloseHandle (token);
  }
  return user;
}

typedef LONG NTAPI NtQuerySystemInformation_ftype (int, PVOID, ULONG, PULONG);
typedef LONG NTAPI NtQueryInformationProcess_ftype (HANDLE, int,
						    PVOID, ULONG, PULONG);

static void
process_cmdline (HANDLE process, std::string &exe, std::string &cmdline,
		 NtQueryInformationProcess_ftype *query_process_info)
{
  PROCESS_BASIC_INFORMATION pbi;
  PRTL_USER_PROCESS_PARAMETERS rupp;
  SIZE_T done;
  if (query_process_info (process, ProcessBasicInformation,
			  &pbi, sizeof (pbi), NULL) == STATUS_SUCCESS
      && ReadProcessMemory (process, &pbi.PebBaseAddress->ProcessParameters,
			    &rupp, sizeof (rupp), &done)
      && done == sizeof (rupp))
    {
      UNICODE_STRING us;
      wchar_t buf[65536];

      if (ReadProcessMemory (process, &rupp->ImagePathName,
			     &us, sizeof (us), &done)
	  && done == sizeof (us)
	  && us.Length > 0 && us.Buffer != NULL
	  && ReadProcessMemory (process, us.Buffer, buf, us.Length, &done)
	  && done == us.Length)
	exe = wcstostring (buf, us.Length / 2);

      if (ReadProcessMemory (process, &rupp->CommandLine,
			     &us, sizeof (us), &done)
	  && done == sizeof (us)
	  && us.Length > 0 && us.Buffer != NULL
	  && ReadProcessMemory (process, us.Buffer, buf, us.Length, &done)
	  && done == us.Length)
	cmdline = wcstostring (buf, us.Length / 2);
    }
}

#define SystemProcessIdInformation 0x58

static std::string
process_path (HANDLE pid, NtQuerySystemInformation_ftype *query_system_info)
{
  struct SYSTEM_PROCESS_ID_INFORMATION
    {
      HANDLE ProcessId;
      UNICODE_STRING ImageName;
    };
  wchar_t buf[65536];
  SYSTEM_PROCESS_ID_INFORMATION spii = { pid, { 0, 65534, buf } };
  if (query_system_info (SystemProcessIdInformation,
			 &spii, sizeof (spii), 0) == STATUS_SUCCESS
      && spii.ImageName.Length >= 2)
    {
      int len = spii.ImageName.Length / 2;
      buf[len] = 0;
      wchar_t drives[128];
      if (GetLogicalDriveStringsW (127, drives))
	{
	  wchar_t path[MAX_PATH];
	  wchar_t drive[3] = L" :";
	  wchar_t *p = drives;
	  do
	    {
	      drive[0] = *p;
	      if (QueryDosDeviceW (drive, path, MAX_PATH)
		  && wcslen (path) > 2
		  && wcsncmp (buf, path, wcslen (path)) == 0
		  && wcslen (path) + len < MAX_PATH)
		{
		  p = buf + wcslen (path) - 2;
		  wcsncpy (p, drive, 2);
		  return wcstostring (p, -1);
		}
	    }
	  while (*p++);
	}
      return wcstostring (buf, len);
    }
  return {};
}

static std::string
win32_xfer_osdata_processes ()
{
  std::string buffer = "<osdata type=\"processes\">\n";

  HMODULE ntdll = GetModuleHandle ("ntdll.dll");
  NtQuerySystemInformation_ftype *query_system_info
    = (NtQuerySystemInformation_ftype *) GetProcAddress
    (ntdll, "NtQuerySystemInformation");
  NtQueryInformationProcess_ftype *query_process_info
    = (NtQueryInformationProcess_ftype *) GetProcAddress
    (ntdll, "NtQueryInformationProcess");

#ifdef __i386__
  BOOL wow64me = 0;
  IsWow64Process(GetCurrentProcess (), &wow64me);
#endif

  ULONG spi_size = 0;
  if (query_system_info != nullptr
      && query_system_info (SystemProcessInformation, NULL, 0, &spi_size)
      == STATUS_INFO_LENGTH_MISMATCH)
    {
      spi_size += 100000;
      gdb::unique_xmalloc_ptr<SYSTEM_PROCESS_INFORMATION> spi
	((SYSTEM_PROCESS_INFORMATION *) xmalloc (spi_size));

      if (query_system_info (SystemProcessInformation, spi.get (),
			     spi_size, NULL) == STATUS_SUCCESS)
	{
	  SYSTEM_PROCESS_INFORMATION *p = spi.get ();
	  while (true)
	    {
	      unsigned pid = (uintptr_t)p->UniqueProcessId;

	      const char *bitness = "";
	      std::string user, exe, cmdline;

	      HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION
					   | PROCESS_VM_READ, FALSE, pid);
	      if (process != NULL)
		{
		  USHORT machProc, machHost;
		  if (IsWow64Process2 != nullptr
		      && IsWow64Process2 (process, &machProc, &machHost))
		    {
		      if (machProc == IMAGE_FILE_MACHINE_UNKNOWN)
			{
			  USHORT pmi[4]; // size of PROCESS_MACHINE_INFORMATION
			  if (GetProcessInformation != nullptr
			      && GetProcessInformation (process,
							ProcessMachineTypeInfo,
							&pmi, sizeof (pmi)))
			    machProc = pmi[0];
			  else
			    machProc = machHost;
			}
		      if (machProc == IMAGE_FILE_MACHINE_I386)
			bitness = "32";
		      else if (machProc == IMAGE_FILE_MACHINE_AMD64)
			bitness = "64";
		      else if (machProc == IMAGE_FILE_MACHINE_ARM64)
			bitness = "64a";
		    }
#ifdef __i386__
		  else if (!wow64me)
		    bitness = "32";
#endif
		  else
		    {
		      BOOL wow64 = 0;
		      IsWow64Process(process, &wow64);
		      bitness = wow64 ? "32" : "64";
		    }

		  user = process_user (process);

		  process_cmdline (process, exe, cmdline, query_process_info);

		  CloseHandle (process);
		}

	      if (exe.empty ())
		exe = process_path (p->UniqueProcessId, query_system_info);
	      if (exe.empty ())
		exe = wcstostring (p->ImageName.Buffer,
				   p->ImageName.Length / 2);

	      string_xml_appendf
		(buffer,
		 "<item>"
		 "<column name=\"pid\">%u</column>"
		 "<column name=\"user\">%s</column>"
		 "<column name=\"command\">%s</column>"
		 "<column name=\"executable\">%s</column>"
		 "<column name=\"bitness\">%s</column>"
		 "</item>",
		 pid,
		 user.c_str (),
		 cmdline.c_str (),
		 exe.c_str (),
		 bitness);

	      if (p->NextEntryOffset == 0)
		break;
	      p = (SYSTEM_PROCESS_INFORMATION *)((char *)p
						 + p->NextEntryOffset);
	    }
	}
    }

  buffer += "</osdata>\n";

  return buffer;
}

static std::string win32_xfer_osdata_info_os_types ();

static struct osdata_type {
  const char *type;
  const char *title;
  const char *description;
  std::string (*take_snapshot) ();
  std::string buffer;
} osdata_table[] = {
  { "types", "Types", "Listing of info os types you can list",
    win32_xfer_osdata_info_os_types },
  { "processes", "Processes", "Listing of all processes",
    win32_xfer_osdata_processes },
  { NULL, NULL, NULL }
};

static std::string
win32_xfer_osdata_info_os_types ()
{
  std::string buffer =  "<osdata type=\"types\">\n";

  /* Start the below loop at 1, as we do not want to list ourselves.  */
  for (int i = 1; osdata_table[i].type; ++i)
    string_xml_appendf (buffer,
			"<item>"
			"<column name=\"Type\">%s</column>"
			"<column name=\"Description\">%s</column>"
			"<column name=\"Title\">%s</column>"
			"</item>",
			osdata_table[i].type,
			osdata_table[i].description,
			osdata_table[i].title);

  buffer += "</osdata>\n";

  return buffer;
}

static LONGEST
common_getter (struct osdata_type *osd,
	       gdb_byte *readbuf, ULONGEST offset, ULONGEST len)
{
  gdb_assert (readbuf);

  if (offset == 0)
    osd->buffer = osd->take_snapshot();

  if (offset >= osd->buffer.size ())
    {
      /* Done.  Get rid of the buffer.  */
      osd->buffer.clear ();
      return 0;
    }

  len = std::min (len, osd->buffer.size () - offset);
  memcpy (readbuf, &osd->buffer[offset], len);

  return len;

}

LONGEST
win32_common_xfer_osdata (const char *annex, gdb_byte *readbuf,
			  ULONGEST offset, ULONGEST len)
{
  if (!annex || *annex == '\0')
    {
      return common_getter (&osdata_table[0],
			    readbuf, offset, len);
    }
  else
    {
      int i;

      for (i = 0; osdata_table[i].type; ++i)
	{
	  if (strcmp (annex, osdata_table[i].type) == 0)
	    return common_getter (&osdata_table[i],
				  readbuf, offset, len);
	}

      return 0;
    }
}

/* Return the name of the DLL referenced by H at ADDRESS.  UNICODE
   determines what sort of string is read from the inferior.  Returns
   the name of the DLL, or NULL on error.  If a name is returned, it
   is stored in a static buffer which is valid until the next call to
   get_image_name.  */

static const char *
get_image_name (HANDLE h, void *address, int unicode)
{
#ifdef __CYGWIN__
  static char buf[MAX_PATH];
#else
  static char buf[(2 * MAX_PATH) + 1];
#endif
  DWORD size = unicode ? sizeof (WCHAR) : sizeof (char);
  char *address_ptr;
  int len = 0;
  char b[2];
  SIZE_T done;

  /* Attempt to read the name of the dll that was detected.
     This is documented to work only when actively debugging
     a program.  It will not work for attached processes.  */
  if (address == NULL)
    return NULL;

  /* See if we could read the address of a string, and that the
     address isn't null.  */
  if (!ReadProcessMemory (h, address,  &address_ptr,
			  sizeof (address_ptr), &done)
      || done != sizeof (address_ptr)
      || !address_ptr)
    return NULL;

  /* Find the length of the string.  */
  while (ReadProcessMemory (h, address_ptr + len++ * size, &b, size, &done)
	 && (b[0] != 0 || b[size - 1] != 0) && done == size)
    continue;

  if (!unicode)
    ReadProcessMemory (h, address_ptr, buf, len, &done);
  else
    {
      WCHAR *unicode_address = (WCHAR *) alloca (len * sizeof (WCHAR));
      ReadProcessMemory (h, address_ptr, unicode_address, len * sizeof (WCHAR),
			 &done);
#ifdef __CYGWIN__
      wcstombs (buf, unicode_address, MAX_PATH);
#else
      WideCharToMultiByte (CP_ACP, 0, unicode_address, len, buf, sizeof buf,
			   0, 0);
#endif
    }

  return buf;
}

/* See nat/windows-nat.h.  */

bool
windows_process_info::handle_ms_vc_exception (const DEBUG_EVENT &current_event)
{
  const EXCEPTION_RECORD *rec = &current_event.u.Exception.ExceptionRecord;

  if (rec->NumberParameters >= 3
      && (rec->ExceptionInformation[0] & 0xffffffff) == 0x1000)
    {
      DWORD named_thread_id;
      windows_thread_info *named_thread;
      CORE_ADDR thread_name_target;

      thread_name_target = rec->ExceptionInformation[1];
      named_thread_id = (DWORD) (0xffffffff & rec->ExceptionInformation[2]);

      if (named_thread_id == (DWORD) -1)
	named_thread_id = current_event.dwThreadId;

      named_thread = find_thread (ptid_t (current_event.dwProcessId,
					  named_thread_id, 0));
      if (named_thread != NULL)
	{
	  int thread_name_len;
	  gdb::unique_xmalloc_ptr<char> thread_name
	    = target_read_string (thread_name_target, 1025, &thread_name_len);
	  if (thread_name_len > 0)
	    {
	      thread_name.get ()[thread_name_len - 1] = '\0';
	      named_thread->name = std::move (thread_name);
	    }
	}

      return true;
    }

  return false;
}

/* The exception thrown by a program to tell the debugger the name of
   a thread.  The exception record contains an ID of a thread and a
   name to give it.  This exception has no documented name, but MSDN
   dubs it "MS_VC_EXCEPTION" in one code example.  */
#define MS_VC_EXCEPTION 0x406d1388

handle_exception_result
windows_process_info::handle_exception (DEBUG_EVENT &current_event,
					struct target_waitstatus *ourstatus,
					bool debug_exceptions)
{
#define DEBUG_EXCEPTION_SIMPLE(x)       if (debug_exceptions) \
  debug_printf ("gdb: Target exception %s at %s\n", x, \
    host_address_to_string (\
      current_event.u.Exception.ExceptionRecord.ExceptionAddress))

  EXCEPTION_RECORD *rec = &current_event.u.Exception.ExceptionRecord;
  DWORD code = rec->ExceptionCode;
  handle_exception_result result = HANDLE_EXCEPTION_HANDLED;

  switch (code)
    {
    case EXCEPTION_ACCESS_VIOLATION:
      DEBUG_EXCEPTION_SIMPLE ("EXCEPTION_ACCESS_VIOLATION");
      ourstatus->set_stopped (GDB_SIGNAL_SEGV);
      if (handle_access_violation (rec))
	return HANDLE_EXCEPTION_UNHANDLED;
      break;
    case STATUS_STACK_OVERFLOW:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_STACK_OVERFLOW");
      ourstatus->set_stopped (GDB_SIGNAL_SEGV);
      break;
    case STATUS_FLOAT_DENORMAL_OPERAND:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_FLOAT_DENORMAL_OPERAND");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      DEBUG_EXCEPTION_SIMPLE ("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case STATUS_FLOAT_INEXACT_RESULT:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_FLOAT_INEXACT_RESULT");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case STATUS_FLOAT_INVALID_OPERATION:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_FLOAT_INVALID_OPERATION");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case STATUS_FLOAT_OVERFLOW:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_FLOAT_OVERFLOW");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case STATUS_FLOAT_STACK_CHECK:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_FLOAT_STACK_CHECK");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case STATUS_FLOAT_UNDERFLOW:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_FLOAT_UNDERFLOW");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case STATUS_FLOAT_DIVIDE_BY_ZERO:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_FLOAT_DIVIDE_BY_ZERO");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case STATUS_INTEGER_DIVIDE_BY_ZERO:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_INTEGER_DIVIDE_BY_ZERO");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case STATUS_INTEGER_OVERFLOW:
      DEBUG_EXCEPTION_SIMPLE ("STATUS_INTEGER_OVERFLOW");
      ourstatus->set_stopped (GDB_SIGNAL_FPE);
      break;
    case EXCEPTION_BREAKPOINT:
#ifdef __x86_64__
      if (ignore_first_breakpoint)
	{
	  /* For WOW64 processes, there are always 2 breakpoint exceptions
	     on startup, first a BREAKPOINT for the 64bit ntdll.dll,
	     then a WX86_BREAKPOINT for the 32bit ntdll.dll.
	     Here we only care about the WX86_BREAKPOINT's.  */
	  DEBUG_EXCEPTION_SIMPLE ("EXCEPTION_BREAKPOINT - ignore_first_breakpoint");
	  ourstatus->set_spurious ();
	  ignore_first_breakpoint = false;
	  break;
	}
      else if (wow64_process)
	{
	  /* This breakpoint exception is triggered for WOW64 processes when
	     reaching an int3 instruction in 64bit code.
	     gdb checks for int3 in case of SIGTRAP, this fails because
	     Wow64GetThreadContext can only report the pc of 32bit code, and
	     gdb lets the target process continue.
	     So handle it as SIGINT instead, then the target is stopped
	     unconditionally.  */
	  DEBUG_EXCEPTION_SIMPLE ("EXCEPTION_BREAKPOINT - wow64_process");
	  rec->ExceptionCode = DBG_CONTROL_C;
	  ourstatus->set_stopped (GDB_SIGNAL_INT);
	  break;
	}
#endif
      [[fallthrough]];
    case STATUS_WX86_BREAKPOINT:
      DEBUG_EXCEPTION_SIMPLE ("EXCEPTION_BREAKPOINT");
      ourstatus->set_stopped (GDB_SIGNAL_TRAP);
      break;
    case DBG_CONTROL_C:
      DEBUG_EXCEPTION_SIMPLE ("DBG_CONTROL_C");
      ourstatus->set_stopped (GDB_SIGNAL_INT);
      break;
    case DBG_CONTROL_BREAK:
      DEBUG_EXCEPTION_SIMPLE ("DBG_CONTROL_BREAK");
      ourstatus->set_stopped (GDB_SIGNAL_INT);
      break;
    case EXCEPTION_SINGLE_STEP:
    case STATUS_WX86_SINGLE_STEP:
      DEBUG_EXCEPTION_SIMPLE ("EXCEPTION_SINGLE_STEP");
      ourstatus->set_stopped (GDB_SIGNAL_TRAP);
      break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      DEBUG_EXCEPTION_SIMPLE ("EXCEPTION_ILLEGAL_INSTRUCTION");
      ourstatus->set_stopped (GDB_SIGNAL_ILL);
      break;
    case EXCEPTION_PRIV_INSTRUCTION:
      DEBUG_EXCEPTION_SIMPLE ("EXCEPTION_PRIV_INSTRUCTION");
      ourstatus->set_stopped (GDB_SIGNAL_ILL);
      break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      DEBUG_EXCEPTION_SIMPLE ("EXCEPTION_NONCONTINUABLE_EXCEPTION");
      ourstatus->set_stopped (GDB_SIGNAL_ILL);
      break;
    case MS_VC_EXCEPTION:
      DEBUG_EXCEPTION_SIMPLE ("MS_VC_EXCEPTION");
      if (handle_ms_vc_exception (current_event))
	{
	  ourstatus->set_stopped (GDB_SIGNAL_TRAP);
	  result = HANDLE_EXCEPTION_IGNORED;
	  break;
	}
	/* treat improperly formed exception as unknown */
	[[fallthrough]];
    default:
      /* Treat unhandled first chance exceptions specially.  */
      if (current_event.u.Exception.dwFirstChance)
	return HANDLE_EXCEPTION_UNHANDLED;
      debug_printf ("gdb: unknown target exception 0x%08x at %s\n",
	(unsigned) current_event.u.Exception.ExceptionRecord.ExceptionCode,
	host_address_to_string (
	  current_event.u.Exception.ExceptionRecord.ExceptionAddress));
      ourstatus->set_stopped (GDB_SIGNAL_UNKNOWN);
      break;
    }

  if (ourstatus->kind () == TARGET_WAITKIND_STOPPED)
    {
      ptid_t ptid (current_event.dwProcessId, current_event.dwThreadId, 0);
      windows_thread_info *th = find_thread (ptid);
      th->last_sig = ourstatus->sig ();
    }

  return result;

#undef DEBUG_EXCEPTION_SIMPLE
}

/* See nat/windows-nat.h.  */

void
windows_process_info::add_dll (LPVOID load_addr)
{
  HMODULE dummy_hmodule;
  DWORD cb_needed;
  HMODULE *hmodules;
  int i;

  BOOL ret = with_context (nullptr, [&] (auto *context)
    {
      return enum_process_modules (context, handle, &dummy_hmodule,
				   sizeof (HMODULE), &cb_needed);
    });
  if (!ret || cb_needed < 1)
    return;

  hmodules = (HMODULE *) alloca (cb_needed);
  ret = with_context (nullptr, [&] (auto *context)
    {
      return enum_process_modules (context, handle, hmodules,
				   cb_needed, &cb_needed);
    });
  if (!ret)
    return;

#if defined __i386__ || defined __x86_64__
  char system_dir[MAX_PATH];
  char syswow_dir[MAX_PATH];
  size_t system_dir_len = 0;
  bool convert_syswow_dir = false;
#ifdef __x86_64__
  if (wow64_process)
#endif
    {
      /* This fails on 32bit Windows because it has no SysWOW64 directory,
	 and in this case a path conversion isn't necessary.  */
      UINT len = GetSystemWow64DirectoryA (syswow_dir, sizeof (syswow_dir));
      if (len > 0)
	{
	  /* Check that we have passed a large enough buffer.  */
	  gdb_assert (len < sizeof (syswow_dir));

	  len = GetSystemDirectoryA (system_dir, sizeof (system_dir));
	  /* Error check.  */
	  gdb_assert (len != 0);
	  /* Check that we have passed a large enough buffer.  */
	  gdb_assert (len < sizeof (system_dir));

	  strcat (system_dir, "\\");
	  strcat (syswow_dir, "\\");
	  system_dir_len = strlen (system_dir);

	  convert_syswow_dir = true;
	}

    }
#endif
  for (i = 1; i < (int) (cb_needed / sizeof (HMODULE)); i++)
    {
      MODULEINFO mi;
#ifdef __USEWIDE
      wchar_t dll_name[MAX_PATH];
      char dll_name_mb[MAX_PATH];
#else
      char dll_name[MAX_PATH];
#endif
      const char *name;
      if (GetModuleInformation (handle, hmodules[i],
				&mi, sizeof (mi)) == 0)
	continue;

      if (GetModuleFileNameEx (handle, hmodules[i],
			       dll_name, sizeof (dll_name)) == 0)
	continue;
#ifdef __USEWIDE
      wcstombs (dll_name_mb, dll_name, MAX_PATH);
      name = dll_name_mb;
#else
      name = dll_name;
#endif

#if defined __i386__ || defined __x86_64__
      /* Convert the DLL path of 32bit processes returned by
	 GetModuleFileNameEx from the 64bit system directory to the
	 32bit syswow64 directory if necessary.  */
      std::string syswow_dll_path;
      if (convert_syswow_dir
	  && strncasecmp (name, system_dir, system_dir_len) == 0
	  && strchr (name + system_dir_len, '\\') == nullptr)
	{
	  syswow_dll_path = syswow_dir;
	  syswow_dll_path += name + system_dir_len;
	  name = syswow_dll_path.c_str();
	}
#endif

      /* Record the DLL if either LOAD_ADDR is NULL or the address
	 at which the DLL was loaded is equal to LOAD_ADDR.  */
      if (!(load_addr != nullptr && mi.lpBaseOfDll != load_addr))
	{
	  maybe_note_cygwin1_dll (name);
	  handle_load_dll (name, mi.lpBaseOfDll);
	  if (load_addr != nullptr)
	    return;
	}
    }
}

/* See nat/windows-nat.h.  */

void
windows_process_info::dll_loaded_event (const DEBUG_EVENT &current_event)
{
  gdb_assert (current_event.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT);

  const LOAD_DLL_DEBUG_INFO *event = &current_event.u.LoadDll;
  const char *dll_name;

  /* Try getting the DLL name via the lpImageName field of the event.
     Note that Microsoft documents this fields as strictly optional,
     in the sense that it might be NULL.  And the first DLL event in
     particular is explicitly documented as "likely not pass[ed]"
     (source: MSDN LOAD_DLL_DEBUG_INFO structure).  */
  dll_name = get_image_name (handle, event->lpImageName, event->fUnicode);
  /* If the DLL name could not be gleaned via lpImageName, try harder
     by enumerating all the DLLs loaded into the inferior, looking for
     one that is loaded at base address = lpBaseOfDll. */
  if (dll_name != nullptr)
    {
      maybe_note_cygwin1_dll (dll_name);
      handle_load_dll (dll_name, event->lpBaseOfDll);
    }
  else if (event->lpBaseOfDll != nullptr)
    add_dll (event->lpBaseOfDll);
}

/* See nat/windows-nat.h.  */

void
windows_process_info::add_all_dlls ()
{
  add_dll (nullptr);
}

#ifdef __CYGWIN__

/* See nat/windows-nat.h.  */

void
windows_process_info::maybe_note_cygwin1_dll (const char *dll_path)
{
  const char *base = dll_path + strlen (dll_path);
  while (base > dll_path && base[-1] != '/' && base[-1] != '\\')
    base--;
  if (strcasecmp (base, "cygwin1.dll") == 0)
    cygwin1_dll_loaded = true;
}

/* See nat/windows-nat.h.  */

bool
inferior_started_by_cygwin (DWORD winpid, bool attaching)
{
  /* In the run (non-attach) case this is called early when the
     inferior has only just reached its first instruction and
     cygwin1.dll hasn't initialized itself yet -- GDB launched the
     inferior with raw CreateProcess, not through Cygwin's fork/spawn
     path, so PID_CYGPARENT is necessarily false, so we can shortcut
     without calling Cygwin.  */
  if (!attaching)
    return false;

  /* Note CW_WINPID_TO_CYGWIN_PID never fails.  It returns a synthetic
     pid for non-Cygwin or unknown winpids, in which case CW_GETPINFO
     returns either a pinfo with PID_CYGPARENT unset, or NULL.  */
  auto cygpid = (pid_t) cygwin_internal (CW_WINPID_TO_CYGWIN_PID, winpid);

  auto *pinfo = (external_pinfo *) cygwin_internal (CW_GETPINFO, cygpid);
  if (pinfo == nullptr)
    return false;

  return (pinfo->process_state & PID_CYGPARENT) != 0;
}

#endif /* __CYGWIN__.  */

/* See nat/windows-nat.h.  */

target_waitstatus
windows_process_info::exit_process_to_target_status
  (const EXIT_PROCESS_DEBUG_INFO &info)
{
  DWORD exit_code = info.dwExitCode;
  target_waitstatus tstatus;

#ifdef __CYGWIN__
  /* A Cygwin parent waiting on a Cygwin child via waitpid doesn't go
     through GetExitCodeProcess / the Win32 exit code at all.  It
     reads the child's wait status directly out of the child's Cygwin
     pinfo (shared memory), set by pinfo::exit in
     winsup/cygwin/pinfo.cc.  So sys/wait.h macros apply to that value
     verbatim.

     GDB, however, even though it is itself a Cygwin program, drives
     its inferiors via the native Win32 debugger API: it spawns them
     with CreateProcess (DEBUG_PROCESS), not via Cygwin's
     fork/spawn/posix_spawn, and consumes
     EXIT_PROCESS_DEBUG_EVENT.dwExitCode from WaitForDebugEvent rather
     than calling waitpid.  That dwExitCode value comes from the
     inferior's ExitProcess call.

     What that value means depends on two orthogonal things:

     1. Is the inferior a Cygwin process at all?  If not, dwExitCode
	is a raw Win32 exit value.

     2. For a Cygwin inferior, was it created through Cygwin's spawn
	path?

	- If not, cygwin1.dll's pinfo::exit byte-swaps the wait status
	  on the way out, so that the meaningful exit value lands in
	  the low byte where native Win32 consumers (cmd.exe's "echo
	  %errorlevel%", and bare GetExitCodeProcess readers) expect
	  it.  This is the case for Cygwin inferiors that we run, via
	  CreateProcess.

	- If yes, cygwin1.dll does not swap.  We see this case if we
	  attach to an already-running process with a Cygwin parent.

	See winsup/cygwin/pinfo.cc:

	 int exitcode = self->exitcode & 0xffff;
	 if (!self->cygstarted)
	   exitcode = ((exitcode & 0xff) << 8) | ((exitcode >> 8) & 0xff);
	 ...
	 ExitProcess (exitcode);
  */

  /* The inferior may also exit with a raw NTSTATUS error code, e.g.,
     STATUS_ACCESS_VIOLATION (0xc0000005), without going through the
     pinfo::exit at all -- for example, if the unhandled-exception
     filter didn't run (e.g., the inferior was killed before
     installing one), or for inferiors that don't link cygwin1.dll.
     Detect those and map them the same way Cygwin's set_exit_code
     does in winsup/cygwin/pinfo.cc, so Cygwin GDB sees the same
     status a Cygwin parent's waitpid would.  */
  if (exit_code >= 0xc0000000)
    {
      gdb_signal sig;
      switch (exit_code)
	{
	case EXCEPTION_ACCESS_VIOLATION:
	  sig = GDB_SIGNAL_SEGV;
	  break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	  sig = GDB_SIGNAL_ILL;
	  break;
	case STATUS_NO_MEMORY:
	  sig = GDB_SIGNAL_BUS;
	  break;
	case STATUS_CONTROL_C_EXIT:
	  sig = GDB_SIGNAL_INT;
	  break;
	default:
	  /* Cygwin maps any other NTSTATUS to exit 127.  */
	  tstatus.set_exited (127);
	  return tstatus;
	}
      tstatus.set_signalled (sig);
      return tstatus;
    }

  if (!this->cygwin1_dll_loaded)
    {
      /* Non-Cygwin inferior: dwExitCode is a raw Win32 exit value.
	 Limit to 8 bits, like Cygwin does, matching what happens with
	 Cygwin inferiors.  */
      tstatus.set_exited (exit_code & 0xff);
      return tstatus;
    }

  /* Note: when GDB attaches to a Cygwin inferior and the inferior is
     then killed externally (e.g., taskkill /F with exit code 1), GDB
     and Cygwin disagree.  Cygwin's parent waitpid reports WIFEXITED,
     code=1; GDB reports SIGHUP (signal 1, no swap below because
     started_by_cygwin).  Cygwin's parent distinguishes "pinfo::exit
     ran" from "didn't run" via the child's wait pipe and only applies
     the swap-undo for the former.  GDB has only dwExitCode and can't
     tell.  This can't be solved without Cygwin's help.  However, such
     an external termination steps out of Cygwin and falls outside
     Cygwin's contract, so it matters less than the cases where the
     inferior exits through Cygwin's own mechanisms.  */

  int wstatus = exit_code & 0xffff;
  if (!this->started_by_cygwin)
    wstatus = ((wstatus & 0xff) << 8) | ((wstatus >> 8) & 0xff);
  if (!WIFSIGNALED (wstatus))
    tstatus.set_exited (WEXITSTATUS (wstatus));
  else
    tstatus.set_signalled (gdb_signal_from_host (WTERMSIG (wstatus)));
#else
  /* If the exit status looks like a fatal exception, but we don't
     recognize the exception's code, make the original exit status
     value available, to avoid losing information.  */
  int exit_signal
    = WIFSIGNALED (exit_code) ? WTERMSIG (exit_code) : -1;
  if (exit_signal == -1)
    tstatus.set_exited (exit_code);
  else
    tstatus.set_signalled (gdb_signal_from_host (exit_signal));
#endif

  return tstatus;
}

/* See nat/windows-nat.h.  */

std::string
event_code_to_string (DWORD event_code)
{
#define CASE(X) \
  case X: return #X

  switch (event_code)
    {
    CASE (CREATE_THREAD_DEBUG_EVENT);
    CASE (EXIT_THREAD_DEBUG_EVENT);
    CASE (CREATE_PROCESS_DEBUG_EVENT);
    CASE (EXIT_PROCESS_DEBUG_EVENT);
    CASE (LOAD_DLL_DEBUG_EVENT);
    CASE (UNLOAD_DLL_DEBUG_EVENT);
    CASE (EXCEPTION_DEBUG_EVENT);
    CASE (OUTPUT_DEBUG_STRING_EVENT);
    default:
      return string_printf ("unknown event code %u", (unsigned) event_code);
    }

#undef CASE
}

/* See nat/windows-nat.h.  */

ptid_t
get_last_debug_event_ptid ()
{
  return ptid_t (last_wait_event.dwProcessId, last_wait_event.dwThreadId, 0);
}

/* See nat/windows-nat.h.  */

BOOL
continue_last_debug_event (DWORD cont_status, bool debug_events)
{
  DEBUG_EVENTS
    ("ContinueDebugEvent (cpid=%d, ctid=0x%x, %s)",
     (unsigned) last_wait_event.dwProcessId,
     (unsigned) last_wait_event.dwThreadId,
     cont_status == DBG_CONTINUE ? "DBG_CONTINUE" :
     cont_status == DBG_EXCEPTION_NOT_HANDLED ? "DBG_EXCEPTION_NOT_HANDLED" :
     cont_status == DBG_REPLY_LATER ? "DBG_REPLY_LATER" :
     "DBG_???");

  return ContinueDebugEvent (last_wait_event.dwProcessId,
			     last_wait_event.dwThreadId,
			     cont_status);
}

/* See nat/windows-nat.h.  */

BOOL
wait_for_debug_event (DEBUG_EVENT *event, DWORD timeout)
{
  BOOL result = WaitForDebugEvent (event, timeout);
  if (result)
    last_wait_event = *event;
  return result;
}

/* Flags to pass to UpdateProcThreadAttribute.  */
#define relocate_aslr_flags ((0x2 << 8) | (0x2 << 16))

/* Attribute to pass to UpdateProcThreadAttribute.  */
#define mitigation_policy 0x00020007

/* Pick one of the symbols as a sentinel.  */
#ifdef PROCESS_CREATION_MITIGATION_POLICY_FORCE_RELOCATE_IMAGES_ALWAYS_OFF

static_assert ((PROCESS_CREATION_MITIGATION_POLICY_FORCE_RELOCATE_IMAGES_ALWAYS_OFF
		| PROCESS_CREATION_MITIGATION_POLICY_BOTTOM_UP_ASLR_ALWAYS_OFF)
	       == relocate_aslr_flags,
	       "check that ASLR flag values are correct");

static_assert (PROC_THREAD_ATTRIBUTE_MITIGATION_POLICY == mitigation_policy,
	       "check that mitigation policy value is correct");

#endif

/* Helper template for the CreateProcess wrappers.

   FUNC is the type of the underlying CreateProcess call.  CHAR is the
   character type to use, and INFO is the "startupinfo" type to use.

   DO_CREATE_PROCESS is the underlying CreateProcess function to use;
   the remaining arguments are passed to it.  */
template<typename FUNC, typename CHAR, typename INFO>
BOOL
create_process_wrapper (FUNC *do_create_process, const CHAR *image,
			CHAR *command_line, DWORD flags,
			void *environment, const CHAR *cur_dir,
			bool no_randomization,
			INFO *startup_info,
			PROCESS_INFORMATION *process_info)
{
  if (no_randomization && disable_randomization_available ())
    {
      static bool tried_and_failed;

      if (!tried_and_failed)
	{
	  /* Windows 8 is required for the real declaration, but to
	     allow building on earlier versions of Windows, we declare
	     the type locally.  */
	  struct gdb_extended_info
	  {
	    INFO StartupInfo;
	    gdb_lpproc_thread_attribute_list lpAttributeList;
	  };

#	  ifndef EXTENDED_STARTUPINFO_PRESENT
#	   define EXTENDED_STARTUPINFO_PRESENT 0x00080000
#	  endif

	  gdb_extended_info info_ex {};

	  if (startup_info != nullptr)
	    info_ex.StartupInfo = *startup_info;
	  info_ex.StartupInfo.cb = sizeof (info_ex);
	  SIZE_T size = 0;
	  /* Ignore the result here.  The documentation says the first
	     call always fails, by design.  */
	  InitializeProcThreadAttributeList (nullptr, 1, 0, &size);
	  info_ex.lpAttributeList
	    = (gdb_lpproc_thread_attribute_list) alloca (size);
	  InitializeProcThreadAttributeList (info_ex.lpAttributeList,
					     1, 0, &size);

	  std::optional<BOOL> return_value;
	  DWORD attr_flags = relocate_aslr_flags;
	  if (!UpdateProcThreadAttribute (info_ex.lpAttributeList, 0,
					  mitigation_policy,
					  &attr_flags,
					  sizeof (attr_flags),
					  nullptr, nullptr))
	    tried_and_failed = true;
	  else
	    {
	      BOOL result = do_create_process (image, command_line,
					       nullptr, nullptr,
					       TRUE,
					       (flags
						| EXTENDED_STARTUPINFO_PRESENT),
					       environment,
					       cur_dir,
					       &info_ex.StartupInfo,
					       process_info);
	      if (result)
		return_value = result;
	      else if (GetLastError () == ERROR_INVALID_PARAMETER)
		tried_and_failed = true;
	      else
		return_value = FALSE;
	    }

	  DeleteProcThreadAttributeList (info_ex.lpAttributeList);

	  if (return_value.has_value ())
	    return *return_value;
	}
    }

  return do_create_process (image,
			    command_line, /* command line */
			    nullptr,	  /* Security */
			    nullptr,	  /* thread */
			    TRUE,	  /* inherit handles */
			    flags,	  /* start flags */
			    environment,  /* environment */
			    cur_dir,	  /* current directory */
			    startup_info,
			    process_info);
}

/* See nat/windows-nat.h.  */

BOOL
create_process (const char *image, char *command_line, DWORD flags,
		void *environment, const char *cur_dir,
		bool no_randomization,
		STARTUPINFOA *startup_info,
		PROCESS_INFORMATION *process_info)
{
  return create_process_wrapper (CreateProcessA, image, command_line, flags,
				 environment, cur_dir, no_randomization,
				 startup_info, process_info);
}

#ifdef __CYGWIN__

/* See nat/windows-nat.h.  */

BOOL
create_process (const wchar_t *image, wchar_t *command_line, DWORD flags,
		void *environment, const wchar_t *cur_dir,
		bool no_randomization,
		STARTUPINFOW *startup_info,
		PROCESS_INFORMATION *process_info)
{
  return create_process_wrapper (CreateProcessW, image, command_line, flags,
				 environment, cur_dir, no_randomization,
				 startup_info, process_info);
}

#endif /* __CYGWIN__ */

/* Define dummy functions which always return error for the rare cases where
   these functions could not be found.  */
template<typename... T>
BOOL WINAPI
bad (T... args)
{
  return FALSE;
}

template<typename... T>
DWORD WINAPI
bad (T... args)
{
  return 0;
}

static BOOL WINAPI
bad_GetCurrentConsoleFont (HANDLE w, BOOL bMaxWindow, CONSOLE_FONT_INFO *f)
{
  f->nFont = 0;
  return 1;
}

static COORD WINAPI
bad_GetConsoleFontSize (HANDLE w, DWORD nFont)
{
  COORD size;
  size.X = 8;
  size.Y = 12;
  return size;
}

/* See windows-nat.h.  */

bool
disable_randomization_available ()
{
  return (InitializeProcThreadAttributeList != nullptr
	  && UpdateProcThreadAttribute != nullptr
	  && DeleteProcThreadAttributeList != nullptr);
}

/* See windows-nat.h.  */

bool
dbg_reply_later_available ()
{
  static int available = -1;
  if (available == -1)
    {
      /* DBG_REPLY_LATER is supported since Windows 10, Version 1507.
	 If supported, this fails with ERROR_INVALID_HANDLE (tested on
	 Win10 and Win11).  If not supported, it fails with
	 ERROR_INVALID_PARAMETER (tested on Win7).  */
      if (ContinueDebugEvent (0, 0, DBG_REPLY_LATER))
	internal_error (_("ContinueDebugEvent call should not "
			  "have succeeded"));
      available = (GetLastError () != ERROR_INVALID_PARAMETER);
    }
  return available;
}

/* See windows-nat.h.  */

bool
initialize_loadable ()
{
  bool result = true;
  HMODULE hm = NULL;

#define GPA(m, func)					\
  func = (func ## _ftype *) GetProcAddress (m, #func)

  hm = LoadLibrary (TEXT ("kernel32.dll"));
  if (hm)
    {
      GPA (hm, DebugActiveProcessStop);
      GPA (hm, DebugBreakProcess);
      GPA (hm, DebugSetProcessKillOnExit);
      GPA (hm, GetConsoleFontSize);
      GPA (hm, DebugActiveProcessStop);
      GPA (hm, GetCurrentConsoleFont);
#ifdef __x86_64__
      GPA (hm, Wow64SuspendThread);
      GPA (hm, Wow64GetThreadContext);
      GPA (hm, Wow64SetThreadContext);
      GPA (hm, Wow64GetThreadSelectorEntry);
#endif
      GPA (hm, GenerateConsoleCtrlEvent);
      GPA (hm, GetThreadDescription);

      GPA (hm, IsWow64Process2);
      GPA (hm, GetProcessInformation);

      GPA (hm, InitializeProcThreadAttributeList);
      GPA (hm, UpdateProcThreadAttribute);
      GPA (hm, DeleteProcThreadAttributeList);

#if defined __i386__ || defined __x86_64__
      GPA (hm, GetEnabledXStateFeatures);
      GPA (hm, InitializeContext);
      GPA (hm, GetXStateFeaturesMask);
      GPA (hm, SetXStateFeaturesMask);
      GPA (hm, LocateXStateFeature);
#endif
    }

  /* Set variables to dummy versions of these processes if the function
     wasn't found in kernel32.dll.  */
  if (!DebugBreakProcess)
    DebugBreakProcess = bad;
  if (!DebugActiveProcessStop || !DebugSetProcessKillOnExit)
    {
      DebugActiveProcessStop = bad;
      DebugSetProcessKillOnExit = bad;
    }
  if (!GetConsoleFontSize)
    GetConsoleFontSize = bad_GetConsoleFontSize;
  if (!GetCurrentConsoleFont)
    GetCurrentConsoleFont = bad_GetCurrentConsoleFont;

  /* Load optional functions used for retrieving filename information
     associated with the currently debugged process or its dlls.  */
  hm = LoadLibrary (TEXT ("psapi.dll"));
  if (hm)
    {
      GPA (hm, EnumProcessModules);
#ifdef __x86_64__
      GPA (hm, EnumProcessModulesEx);
#endif
      GPA (hm, GetModuleInformation);
      GPA (hm, GetModuleFileNameExA);
      GPA (hm, GetModuleFileNameExW);
    }

  if (!EnumProcessModules || !GetModuleInformation
      || !GetModuleFileNameExA || !GetModuleFileNameExW)
    {
      /* Set variables to dummy versions of these processes if the function
	 wasn't found in psapi.dll.  */
      EnumProcessModules = bad;
      GetModuleInformation = bad;
      GetModuleFileNameExA = bad;
      GetModuleFileNameExW = bad;

      result = false;
    }

  hm = LoadLibrary (TEXT ("advapi32.dll"));
  if (hm)
    {
      GPA (hm, OpenProcessToken);
      GPA (hm, LookupPrivilegeValueA);
      GPA (hm, AdjustTokenPrivileges);
      /* Only need to set one of these since if OpenProcessToken fails nothing
	 else is needed.  */
      if (!OpenProcessToken || !LookupPrivilegeValueA
	  || !AdjustTokenPrivileges)
	OpenProcessToken = bad;
    }

  /* On some versions of Windows, this function is only available in
     KernelBase.dll, not kernel32.dll.  */
  if (GetThreadDescription == nullptr)
    {
      hm = LoadLibrary (TEXT ("KernelBase.dll"));
      if (hm)
	GPA (hm, GetThreadDescription);
    }

#ifdef __x86_64__
  hm = LoadLibrary (TEXT ("ntdll.dll"));
  if (hm)
    {
      GPA (hm, RtlGetExtendedFeaturesMask);
      GPA (hm, RtlSetExtendedFeaturesMask);
      GPA (hm, RtlLocateExtendedFeature);
    }
#endif

#undef GPA

#if defined __i386__ || defined __x86_64__
  if (GetEnabledXStateFeatures != nullptr
      && InitializeContext != nullptr
      && GetXStateFeaturesMask != nullptr
      && SetXStateFeaturesMask != nullptr
      && LocateXStateFeature != nullptr
#ifdef __x86_64__
      && RtlGetExtendedFeaturesMask != nullptr
      && RtlSetExtendedFeaturesMask != nullptr
      && RtlLocateExtendedFeature != nullptr
#endif
  )
    {
      /* Available XState features masked with implemented features.  */
      xstate_features = (GetEnabledXStateFeatures ()
			 & X86_XSTATE_AVX_AVX512_MASK);
      /* The extended XState functions are only needed if the available
	 features exceed SSE.  */
      if ((xstate_features & ~X86_XSTATE_SSE_MASK) == 0)
	xstate_features = 0;
    }
#endif

  return result;
}

}


#ifdef HAVE_LIBWINIPT
#include "windows-btrace.h"
#include "nat/x86-cpuid.h"

#include <windows.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
extern "C" {
#include <libipt.h>
}
#pragma GCC diagnostic pop

struct handle_deleter
{
  void operator() (HANDLE h) const
    {
      if (h != INVALID_HANDLE_VALUE)
	CloseHandle (h);
    }
  using pointer = HANDLE;
};

using handle_up = std::unique_ptr<HANDLE, handle_deleter>;

struct sc_handle_deleter
{
  void operator() (SC_HANDLE sch) const
    {
      CloseServiceHandle (sch);
    }
  using pointer = SC_HANDLE;
};

using sc_handle_up = std::unique_ptr<SC_HANDLE, sc_handle_deleter>;

/* Look up and start IPT service.  */

static void
windows_enable_ipt_service ()
{
  sc_handle_up scManager
    (OpenSCManager (nullptr, nullptr, SC_MANAGER_CONNECT));
  if (!scManager)
    {
      unsigned err = (unsigned) GetLastError ();
      error (_("Can't open SCM handle (error %u: %s)."),
	     err, strwinerror (err));
    }

  sc_handle_up scService
    (OpenServiceW (scManager.get (), L"Ipt", SERVICE_START));
  if (!scService)
    {
      unsigned err = (unsigned) GetLastError ();
      error (_("Can't open IPT service (error %u: %s)."),
	     err, strwinerror (err));
    }

  if (!StartService (scService.get (), 0, nullptr))
    {
      unsigned err = (unsigned) GetLastError ();
      if (err != ERROR_SERVICE_ALREADY_RUNNING)
	error (_("Can't start IPT service (error %u: %s)."),
	       err, strwinerror (err));
    }
}

/* See windows-btrace.h.  */

struct btrace_target_info *
windows_enable_btrace (ptid_t ptid, const struct btrace_config *conf,
		       int &ipt_threads)
{
  if (conf->format != BTRACE_FORMAT_PT)
    error (_("Unknown branch trace format."));

  DWORD size = conf->pt.size;
  if (size < 4 * 1024)
    size = 4 * 1024;
  else if (size > 128 * 1024 * 1024)
    size = 128 * 1024 * 1024;

  /* IPT tracing is enabled per process, so only do it for the first
     thread.  */
  if (ipt_threads == 0)
    {
      windows_enable_ipt_service ();

      DWORD bufferVersion;
      if (!GetIptBufferVersion (&bufferVersion))
	error (_("Can't get IPT buffer version."));
      if (bufferVersion != IPT_BUFFER_MAJOR_VERSION_CURRENT)
	error (_("IPT buffer version mismatch."));

      WORD traceVersion;
      if (!GetIptTraceVersion (&traceVersion))
	error (_("Can't get IPT trace version."));
      if (traceVersion != IPT_TRACE_VERSION_CURRENT)
	error (_("IPT trace version mismatch."));

      int pid = ptid.pid ();
      handle_up hProcess (OpenProcess (PROCESS_VM_READ, FALSE, pid));
      if (!hProcess)
	{
	  unsigned err = (unsigned) GetLastError ();
	  error (_("Can't open process handle (error %u: %s)."),
		 err, strwinerror (err));
	}

      DWORD index;
      BitScanReverse(&index, size);

      IPT_OPTIONS options;
      options.AsULonglong = 0;
      options.OptionVersion = 1;
      options.TopaPagesPow2 = index - 12;
      options.TimingSettings = IptNoTimingPackets;
      options.ModeSettings = IptCtlUserModeOnly;

      if (!StartProcessIptTracing (hProcess.get (), options))
	error (_("Can't start trace."));
    }

  ipt_threads++;

  std::unique_ptr<btrace_target_info> tinfo
    { std::make_unique<btrace_target_info> (ptid) };

  tinfo->conf.format = BTRACE_FORMAT_PT;
  tinfo->conf.pt.size = size;

  return tinfo.release ();
}

/* See windows-btrace.h.  */

bool
windows_disable_btrace (struct btrace_target_info *tinfo, int &ipt_threads)
{
  if (tinfo->conf.format != BTRACE_FORMAT_PT)
    return false;

  /* Disable IPT tracing when the last thread is reached.  */
  if (--ipt_threads > 0)
    return true;

  int pid = tinfo->ptid.pid ();
  handle_up hProcess (OpenProcess (PROCESS_VM_READ, FALSE, pid));
  if (!hProcess)
    return false;

  if (!StopProcessIptTracing (hProcess.get ()))
    return false;

  return true;
}

/* Identify the cpu we're running on.  */

static struct btrace_cpu
btrace_this_cpu (void)
{
  struct btrace_cpu cpu;
  unsigned int eax, ebx, ecx, edx;
  int ok;

  memset (&cpu, 0, sizeof (cpu));

  ok = x86_cpuid (0, &eax, &ebx, &ecx, &edx);
  if (ok != 0)
    {
      if (ebx == signature_INTEL_ebx && ecx == signature_INTEL_ecx
	  && edx == signature_INTEL_edx)
	{
	  unsigned int cpuid, ignore;

	  ok = x86_cpuid (1, &cpuid, &ignore, &ignore, &ignore);
	  if (ok != 0)
	    {
	      cpu.vendor = CV_INTEL;

	      cpu.family = (cpuid >> 8) & 0xf;
	      if (cpu.family == 0xf)
		cpu.family += (cpuid >> 20) & 0xff;

	      cpu.model = (cpuid >> 4) & 0xf;
	      if ((cpu.family == 0x6) || ((cpu.family & 0xf) == 0xf))
		cpu.model += (cpuid >> 12) & 0xf0;
	    }
	}
      else if (ebx == signature_AMD_ebx && ecx == signature_AMD_ecx
	       && edx == signature_AMD_edx)
	cpu.vendor = CV_AMD;
    }

  return cpu;
}

/* See windows-btrace.h.  */

enum btrace_error
windows_read_btrace (struct btrace_data *data,
		     struct btrace_target_info *btinfo,
		     enum btrace_read_type type)
{
  if (btinfo->conf.format != BTRACE_FORMAT_PT)
    return BTRACE_ERR_NOT_SUPPORTED;

  data->format = BTRACE_FORMAT_PT;
  data->variant.pt.data = nullptr;
  data->variant.pt.size = 0;
  data->variant.pt.config.cpu = btrace_this_cpu ();

  if (type == BTRACE_READ_NEW)
    type = BTRACE_READ_ALL;

  if (type != BTRACE_READ_ALL)
    return BTRACE_ERR_NOT_SUPPORTED;

  int pid = btinfo->ptid.pid ();
  handle_up hProcess (OpenProcess (PROCESS_VM_READ, FALSE, pid));
  if (!hProcess)
    return BTRACE_ERR_UNKNOWN;

  DWORD traceSize;
  if (!GetProcessIptTraceSize (hProcess.get (), &traceSize))
    return BTRACE_ERR_UNKNOWN;

  gdb::unique_xmalloc_ptr<unsigned char> buf
    ((unsigned char *) xmalloc (traceSize));

  if (!GetProcessIptTrace (hProcess.get (), buf.get (), traceSize))
    return BTRACE_ERR_UNKNOWN;

  IPT_TRACE_DATA *pt_data = (IPT_TRACE_DATA *) buf.get ();
  unsigned char *buf_end = buf.get () + traceSize;
  size_t trace_offset = offsetof (IPT_TRACE_HEADER, Trace);

  DWORD threadId = btinfo->ptid.lwp ();
  IPT_TRACE_HEADER *pt_thread = (IPT_TRACE_HEADER *) pt_data->TraceData;
  while ((unsigned char *) pt_thread + trace_offset < buf_end
	 && pt_thread->Trace + pt_thread->TraceSize <= buf_end)
    {
      if (pt_thread->ThreadId == threadId)
	{
	  uint32_t trace_size = pt_thread->TraceSize;
	  BYTE *trace_data = pt_thread->Trace;
	  gdb::unique_xmalloc_ptr<gdb_byte> bytes
	    ((gdb_byte *) xmalloc (trace_size));
	  uint32_t start = pt_thread->RingBufferOffset;
	  if (start > 0)
	    memcpy (bytes.get () + (trace_size - start), trace_data, start);
	  memcpy (bytes.get (), trace_data + start, trace_size - start);

	  data->variant.pt.data = bytes.release ();
	  data->variant.pt.size = trace_size;
	  return BTRACE_ERR_NONE;
	}

      pt_thread
	= (IPT_TRACE_HEADER *) (pt_thread->Trace + pt_thread->TraceSize);
    }

  return BTRACE_ERR_UNKNOWN;
}

/* See windows-btrace.h.  */

const struct btrace_config *
windows_btrace_conf (const struct btrace_target_info *btinfo)
{
  return &btinfo->conf;
}
#endif

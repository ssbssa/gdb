
#ifdef HAVE_LIBWINIPT
#include "gdbsupport/btrace-common.h"

/* See target_ops::enable_btrace in target.h.  */
struct btrace_target_info *
windows_enable_btrace (ptid_t ptid, const struct btrace_config *conf,
		       int &ipt_threads);

/* See target_ops::disable_btrace in target.h.  */
bool windows_disable_btrace (struct btrace_target_info *tinfo,
			     int &ipt_threads);

/* See target_ops::read_btrace in target.h.  */
enum btrace_error windows_read_btrace (struct btrace_data *data,
				       struct btrace_target_info *btinfo,
				       enum btrace_read_type type);

/* See target_ops::btrace_conf in target.h.  */
const struct btrace_config *
windows_btrace_conf (const struct btrace_target_info *btinfo);
#endif

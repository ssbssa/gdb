# Additional TUI windows.
# Copyright 2021 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import gdb
import sys
import re
#import traceback

PY3 = sys.version_info[0] == 3

custom_windows = {}


if sys.platform == 'win32':
    import ctypes
    import ctypes.wintypes as w

    CF_TEXT = 1

    u32 = ctypes.WinDLL('user32')
    k32 = ctypes.WinDLL('kernel32')

    OpenClipboard = u32.OpenClipboard
    OpenClipboard.argtypes = w.HWND,
    OpenClipboard.restype = w.BOOL

    EmptyClipboard = u32.EmptyClipboard
    EmptyClipboard.restype = w.BOOL

    SetClipboardData = u32.SetClipboardData
    SetClipboardData.argtypes = w.UINT, w.HANDLE,
    SetClipboardData.restype = w.HANDLE

    CloseClipboard = u32.CloseClipboard
    CloseClipboard.restype = w.BOOL

    GHND = 0x0042

    GlobalAlloc = k32.GlobalAlloc
    GlobalAlloc.argtypes = w.UINT, ctypes.c_size_t,
    GlobalAlloc.restype = w.HGLOBAL

    GlobalLock = k32.GlobalLock
    GlobalLock.argtypes = w.HGLOBAL,
    GlobalLock.restype = w.LPVOID

    GlobalUnlock = k32.GlobalUnlock
    GlobalUnlock.argtypes = w.HGLOBAL,
    GlobalUnlock.restype = w.BOOL

    def set_clipboard_text(s):
        if OpenClipboard(None):
            s = s.encode('utf-8')
            EmptyClipboard()
            h = GlobalAlloc(GHND, len(s) + 1)
            p = GlobalLock(h)
            ctypes.memmove(p, s, len(s))
            GlobalUnlock(h)
            SetClipboardData(CF_TEXT, h)
            CloseClipboard()

else:
    import base64

    def set_clipboard_text(s):
        b64 = base64.b64encode(s.encode('utf-8')).decode()
        sys.__stdout__.write("\x1b]52;c;" + b64 + "\x07")


class TextWindow(object):
    def __init__(self, win, title):
        self.win = win
        self.line_ofs = 0
        self.col_ofs = 0
        self.lines = []
        win.title = title
        global custom_windows
        custom_windows[title] = self

    def close(self):
        global custom_windows
        del custom_windows[self.win.title]

    def render(self):
        self.line_ofs = 0
        self.col_ofs = 0

    def hscroll(self, num):
        prev_col_ofs = self.col_ofs
        self.col_ofs = self.col_ofs + num
        if self.col_ofs < 0:
            self.col_ofs = 0
        if self.col_ofs != prev_col_ofs:
            self.redraw()

    def vscroll(self, num):
        prev_line_ofs = self.line_ofs
        self.line_ofs = self.line_ofs + num
        l = len(self.lines)
        if self.line_ofs >= l:
            self.line_ofs = l - 1
        if self.line_ofs < 0:
            self.line_ofs = 0
        if self.line_ofs != prev_line_ofs:
            self.redraw()

    def redraw(self):
        l = len(self.lines)
        if self.line_ofs > 0 and self.line_ofs + self.win.height > l:
            self.line_ofs = l - self.win.height
            if self.line_ofs < 0:
                self.line_ofs = 0
        start = self.line_ofs
        stop = self.line_ofs + self.win.height
        if stop > l:
            stop = l
        if stop > start:
            self.win.write("".join(self.lines[start:stop]), self.col_ofs, True)
        else:
            self.win.erase()


def is_string_instance(s):
    if PY3:
        return isinstance(s, str)
    else:
        return isinstance(s, basestring)

def val_cmp_color(prev, cur, sym_not_init, argument, empty=False):
    var_col_s, var_col_e, val_col_s, val_col_e = "", "", "", ""
    if empty:
        var_col_s, var_col_e = "\033[1;30m", "\033[0m"
    elif prev is None:
        var_col_s, var_col_e = "\033[1;32m", "\033[0m"
    elif prev != cur:
        val_col_s, val_col_e = "\033[1;31m", "\033[0m"
    if argument:
        var_col_s, var_col_e = "\033[1;35m", "\033[0m"
    elif sym_not_init:
        var_col_s, var_col_e = "\033[33m", "\033[0m"
    return (var_col_s, var_col_e, val_col_s, val_col_e)

def octal_escape(s):
    return "".join(c if ord(c) < 128 else "\\%03o" % ord(c) for c in s)

def value_string(valstr, hint):
    if hint == "string":
        is_lazy = type(valstr).__name__ == "LazyString"
        if is_lazy:
            l = valstr.length
        elif not is_string_instance(valstr):
            l = 1
        else:
            l = len(valstr)
        if l == 0:
            if is_lazy and str(valstr.type) == "wchar_t *":
                valstr = "L\"\""
            else:
                valstr = "\"\""
        else:
            valstr = gdb.Value(valstr).format_string(symbols=False, address=False)
    if isinstance(valstr, gdb.Value):
        valstr = valstr.format_string(symbols=False, address=False)
    elif not type(valstr) is str:
        valstr = str(valstr)
    return valstr

def is_typedef_of(valtype, name):
    if valtype.name == name:
        return True
    while valtype.code == gdb.TYPE_CODE_TYPEDEF:
        valtype = valtype.target()
        if valtype.name == name:
            return True
    return False


class VariableWindow(TextWindow):
    def __init__(self, win, title, convenience_name):
        super(VariableWindow, self).__init__(win, title)
        self.prev_vals = {}
        self.line_names = []
        self.convenience_name = convenience_name

    def click(self, x, y, button):
        line = y + self.line_ofs
        if button == 1 and line < len(self.line_names):
            name = self.line_names[line]
            if name:
                prev = self.prev_vals[name]
                if prev is not None and prev[0] is not None:
                    prev[0] = not prev[0]
                    self.refill(True)
        elif button == 3 and line < len(self.line_names):
            name = self.line_names[line]
            if name:
                prev = self.prev_vals[name]
                if prev is not None and prev[3] is not None:
                    gdb.set_convenience_variable(self.convenience_name, prev[3])
                    self.refill(True)
        elif button == 2 and line < len(self.line_names):
            name = self.line_names[line]
            if name:
                prev = self.prev_vals[name]
                if prev is not None:
                    if prev[0]:
                        name2 = name + ":"
                        child_texts = []
                        for l in range(line + 1, len(self.line_names)):
                            child_name = self.line_names[l]
                            if not child_name:
                                continue
                            if not child_name.startswith(name2):
                                break
                            child = self.prev_vals[child_name]
                            if child is not None and child[1] != False:
                                child_texts.append(child[1])
                        set_clipboard_text("\n".join(child_texts))
                    elif prev[1] != False:
                        set_clipboard_text(prev[1])

    def refill(self, keep_prev=False):
        if not self.win.is_valid():
            return
        self.lines = []
        self.line_names = []
        cur_vals = {}
        for v in self.variables():
            name = v.symbol()
            value = v.value()
            sym_not_init = False
            argument = False
            num = 0
            raw = False
            fmt = None
            error = None
            if hasattr(v, "undeclared"):
                sym_not_init = v.undeclared()
            if hasattr(v, "argument"):
                argument = v.argument()
            if hasattr(v, "number"):
                num = v.number()
            if hasattr(v, "raw"):
                raw = v.raw()
            if hasattr(v, "format"):
                fmt = v.format()
            if hasattr(v, "error"):
                error = v.error()
            self.add_val(name, name, value, 0, num, cur_vals, keep_prev, False, sym_not_init, argument, raw, fmt, True, error)
        self.prev_vals = cur_vals
        self.redraw()

    def add_val(self, n, fn, v, inset, num, cur_vals, keep_prev, def_expand, sym_not_init, argument, raw, fmt, dyn_type, error):
        n2 = fn
        if inset == 0:
            if num == 0:
                count = 1
                while n2 in cur_vals:
                    count += 1
                    n2 = "%s:%d" % (n, count)
            else:
                n2 = "%d:%s" % (num, n)

        cur_entry = [None, False, None, None]
        cur_vals[n2] = cur_entry
        expand = None
        prev_val = None
        if n2 in self.prev_vals:
            pv = self.prev_vals[n2]
            expand = pv[0]
            if keep_prev:
                prev_val = pv[2]
            else:
                prev_val = pv[1]
            cur_entry[0] = expand
            cur_entry[2] = prev_val

        spaces = "  " * inset
        if num > 0:
            numstr = "%d: " % num
        else:
            numstr = ""

        if v is None:
            if error is None:
                (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(False, False, sym_not_init, argument, empty=True)
                self.lines.append(spaces + numstr + "  " + var_col_s + n + var_col_e + "\n")
            else:
                (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(False, False, sym_not_init, argument)
                self.lines.append(spaces + numstr + "  " + var_col_s + n + var_col_e + " = \033[1;30m<" + error + ">\033[0m\n")
            self.line_names.append(n2)
            return

        if is_string_instance(v):
            (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(prev_val, v, sym_not_init, argument)
            self.lines.append(spaces + numstr + "  " + var_col_s + n + var_col_e + " = " + val_col_s + v + val_col_e + "\n")
            self.line_names.append(n2)
            cur_entry[1] = v
            return

        is_pp = hasattr(v, "to_string") or hasattr(v, "children")

        v_addr = None
        if is_pp:
            if hasattr(v, "address"):
                try:
                    v_addr = v.address()
                except:
                    v_addr = None
        elif isinstance(v, gdb.Value):
            is_optimized_out = False
            try:
                if v.type.code in [gdb.TYPE_CODE_REF, gdb.TYPE_CODE_RVALUE_REF]:
                    v = v.referenced_value()

                is_optimized_out = v.is_optimized_out
            except:
                self.add_val(n, fn, None, inset, num, cur_vals, keep_prev, False, sym_not_init, argument, raw, fmt, False, str(sys.exc_info()[1]))
                return

            if is_optimized_out and v.type.strip_typedefs().code not in [gdb.TYPE_CODE_STRUCT, gdb.TYPE_CODE_UNION]:
                self.add_val(n, fn, None, inset, num, cur_vals, keep_prev, False, sym_not_init, argument, raw, fmt, False, "optimized out")
                return

            v_addr = v.address
        else:
            v = gdb.Value(v)

        cv_str = ""
        if v_addr is not None:
            v_addr = v_addr.dereference().reference_value()
            cur_entry[3] = v_addr
            cv = gdb.convenience_variable(self.convenience_name)
            if cv is not None and cv.address == v_addr.address and cv.type == v_addr.type:
                cv_str = " = \033[1;36m$" + self.convenience_name + "\033[0m"

        pp = None
        if is_pp:
            pp = v
        elif not raw:
            try:
                pp = gdb.default_visualizer(v)
            except:
                pp = None
        if pp:
            valstr = None
            try:
                if expand is None and hasattr(pp, "children"):
                    expand = def_expand
                    cur_entry[0] = expand
                expand_str = "  "
                if expand is not None:
                    if expand:
                        expand_str = "- "
                    else:
                        expand_str = "+ "
                numstr += expand_str

                if hasattr(pp, "to_string"):
                    valstr = pp.to_string()
                hint = None
                if hasattr(pp, "display_hint"):
                    hint = pp.display_hint()
                if valstr is not None:
                    valstr = value_string(valstr, hint)
                    valstr = octal_escape(valstr)
                    sp = valstr.split("\n")
                    (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(prev_val, valstr, sym_not_init, argument)
                    self.lines.append(spaces + numstr + var_col_s + n + var_col_e + " = " + val_col_s + sp.pop(0) + val_col_e + cv_str + "\n")
                    self.line_names.append(n2)
                    for extra in sp:
                        self.lines.append(spaces + "    " + extra + "\n")
                        self.line_names.append(None)
                    cur_entry[1] = valstr
                else:
                    (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(prev_val, False, sym_not_init, argument)
                    self.lines.append(spaces + numstr + var_col_s + n + var_col_e + cv_str + "\n")
                    self.line_names.append(n2)

                if expand:
                    childs = None
                    if hasattr(pp, "children"):
                        childs = pp.children()
                    if childs:
                        if hint == "map":
                            count = 0
                            key_prev = None
                            for c in childs:
                                (nc, vc) = c
                                count += 1
                                fnc = ":".join([n2, nc])
                                if (count % 2) == 1:
                                    key_prev = None
                                    if is_string_instance(vc):
                                        key_prev = vc
                                    else:
                                        vc_check = vc
                                        if vc_check.type.code in [gdb.TYPE_CODE_REF, gdb.TYPE_CODE_RVALUE_REF]:
                                            vc_check = vc_check.referenced_value()
                                        key_pp = gdb.default_visualizer(vc_check)
                                        if key_pp:
                                            if hasattr(key_pp, "to_string") and not hasattr(key_pp, "children"):
                                                vc_check = key_pp.to_string()
                                                if vc_check is not None:
                                                    hint = None
                                                    if hasattr(key_pp, "display_hint"):
                                                        hint = key_pp.display_hint()
                                                    key_prev = value_string(vc_check, hint)
                                        else:
                                            t_check = vc_check.type.strip_typedefs()
                                            if t_check.code in [gdb.TYPE_CODE_ENUM, gdb.TYPE_CODE_INT, gdb.TYPE_CODE_FLT,
                                                    gdb.TYPE_CODE_BOOL, gdb.TYPE_CODE_COMPLEX, gdb.TYPE_CODE_DECFLOAT]:
                                                if fmt:
                                                    key_prev = vc_check.format_string(symbols=False, raw=True, format=fmt)
                                                else:
                                                    key_prev = vc_check.format_string(symbols=False, raw=True)
                                    if key_prev is not None and "\n" in key_prev:
                                        key_prev = None
                                    if key_prev is None:
                                        self.add_val("key", fnc, vc, inset + 1, 0, cur_vals, keep_prev, False, False, False, raw, fmt, True, None)
                                else:
                                    if key_prev is not None:
                                        self.add_val("[" + str(key_prev) + "]", fnc, vc, inset + 1, 0, cur_vals, keep_prev, False, False, False, raw, fmt, True, None)
                                    else:
                                        self.add_val("value", fnc, vc, inset + 1, 0, cur_vals, keep_prev, False, False, False, raw, fmt, True, None)
                        else:
                            for c in childs:
                                (nc, vc) = c
                                fnc = ":".join([n2, nc])
                                self.add_val(nc, fnc, vc, inset + 1, 0, cur_vals, keep_prev, False, False, False, raw, fmt, True, None)
            except:
                self.add_val(n, fn, None, inset, num, cur_vals, keep_prev, False, sym_not_init, argument, raw, fmt, False, str(sys.exc_info()[1]))
                #traceback.print_exc()
            return

        t = v.type.strip_typedefs()

        is_string = False
        if t.code in [gdb.TYPE_CODE_ARRAY, gdb.TYPE_CODE_PTR]:
            target_type = t.target().strip_typedefs()
            if target_type.code == gdb.TYPE_CODE_INT and (target_type.name == "char" or is_typedef_of(t.target(), "wchar_t")):
                is_string = True
        is_array = t.code in [gdb.TYPE_CODE_ARRAY, gdb.TYPE_CODE_RANGE]
        is_ptr = t.code == gdb.TYPE_CODE_PTR and not target_type.code in [gdb.TYPE_CODE_FUNC, gdb.TYPE_CODE_METHOD]
        name_add = ""
        if dyn_type and v.type.code == gdb.TYPE_CODE_STRUCT:
            try:
                if v.type != v.dynamic_type:
                    v = v.cast(v.dynamic_type)
                    name_add = " <" + v.type.name + ">"
            except:
                pass
        expand_str = "  "
        if is_ptr or is_array or t.code in [gdb.TYPE_CODE_STRUCT, gdb.TYPE_CODE_UNION]:
            if expand is None:
                expand = def_expand
                cur_entry[0] = expand
            if expand is not None:
                if expand:
                    expand_str = "- "
                else:
                    expand_str = "+ "
        numstr += expand_str

        if is_string or t.code in [gdb.TYPE_CODE_PTR, gdb.TYPE_CODE_ENUM, gdb.TYPE_CODE_FUNC, gdb.TYPE_CODE_INT,
                gdb.TYPE_CODE_FLT, gdb.TYPE_CODE_STRING, gdb.TYPE_CODE_METHOD, gdb.TYPE_CODE_METHODPTR,
                gdb.TYPE_CODE_MEMBERPTR, gdb.TYPE_CODE_CHAR, gdb.TYPE_CODE_BOOL, gdb.TYPE_CODE_COMPLEX, gdb.TYPE_CODE_DECFLOAT]:
            try:
                if fmt and not is_string:
                    valstr = v.format_string(symbols=False, raw=True, address=True, format=fmt)
                else:
                    valstr = v.format_string(symbols=False, raw=True, address=True)
                valstr = octal_escape(valstr)
                (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(prev_val, valstr, sym_not_init, argument)
                self.lines.append(spaces + numstr + var_col_s + n + var_col_e + " = " + val_col_s + valstr + val_col_e + cv_str + "\n")
                self.line_names.append(n2)
                cur_entry[1] = valstr
            except:
                self.add_val(n, fn, None, inset, num, cur_vals, keep_prev, False, sym_not_init, argument, raw, fmt, False, str(sys.exc_info()[1]))
                #traceback.print_exc()
                return
        else:
            (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(prev_val, False, sym_not_init, argument)
            self.lines.append(spaces + numstr + var_col_s + n + var_col_e + name_add + cv_str + "\n")
            self.line_names.append(n2)
        if t.code == gdb.TYPE_CODE_ENUM:
            return

        if not expand:
            return

        if is_array:
            (low, high) = t.range()
            for i in range(low, high + 1):
                nc = "[%d]" % i
                fnc = ":".join([n2, nc])
                self.add_val(nc, fnc, v[i], inset + 1, 0, cur_vals, keep_prev, False, False, False, raw, fmt, True, None)
            return

        if is_ptr:
            nc = "[0]"
            fnc = ":".join([n2, nc])
            try:
                v = v.dereference()
            except:
                v = None
            self.add_val(nc, fnc, v, inset + 1, 0, cur_vals, keep_prev, target_type.code != gdb.TYPE_CODE_PTR, False, False, raw, fmt, True, None)
            return

        fields = None
        if t.code in [gdb.TYPE_CODE_STRUCT, gdb.TYPE_CODE_UNION]:
            try:
                fields = v.type.fields()
            except:
                pass
        if fields:
            num = 1
            for f in fields:
                if not hasattr(f, "bitpos"):
                    continue
                try:
                    n = f.name
                except:
                    n = None
                if not n:
                    n = "<anonymous>"
                elif f.is_base_class:
                    n = "<" + n + ">"
                vf = v[f]
                fnc = ":".join([n2, n, "%d" % num])
                num += 1
                self.add_val(n, fnc, vf, inset + 1, 0, cur_vals, keep_prev, False, False, False, raw, fmt, False, None)

class VarNameValue(object):
    def __init__(self, sym, val, undecl=False, arg=False, num=0, r=False, fmt=None, err=None):
        self.sym = sym
        self.val = val
        self.undecl = undecl
        self.arg = arg
        self.num = num
        self.r = r
        self.fmt = fmt
        self.err = err

    def symbol(self):
        return self.sym

    def value(self):
        return self.val

    def undeclared(self):
        return self.undecl

    def argument(self):
        return self.arg

    def number(self):
        return self.num

    def raw(self):
        return self.r

    def format(self):
        return self.fmt

    def error(self):
        return self.err

class LocalsWindow(VariableWindow):
    def __init__(self, win):
        super(LocalsWindow, self).__init__(win, "locals", "lv")

    def variables(self):
        thread = gdb.selected_thread()
        thread_valid = thread and thread.is_valid()
        if thread_valid:
            frame = gdb.selected_frame()
            cur_line = frame.find_sal().line
            try:
                block = frame.block()
            except:
                block = None
            while block:
                if not block.is_global:
                    for symbol in block:
                        if symbol.is_argument or symbol.is_variable:
                            sym_not_init = symbol.is_variable and symbol.line > 0 and cur_line <= symbol.line
                            yield VarNameValue(symbol.name, symbol.value(frame), undecl=sym_not_init, arg=symbol.is_argument)
                if block.function:
                    break
                block = block.superblock

class DisplayWindow(VariableWindow):
    def __init__(self, win):
        super(DisplayWindow, self).__init__(win, "display", "dv")
        self.disp_re = re.compile('^(?P<num>\d+): +(?P<enabled>[yn]) +(?P<fmt>\/\w+ +)?(?P<expr>.*)')
        gdb.set_tui_auto_display(False)

    def close(self):
        VariableWindow.close(self)
        gdb.set_tui_auto_display(True)

    def variables(self):
        thread = gdb.selected_thread()
        thread_valid = thread and thread.is_valid()
        cant_eval_str = " (cannot be evaluated in the current context)"
        display = gdb.execute("info display", to_string=True).split("\n")
        formats = "xduotacfszi"
        for d in display:
            m = self.disp_re.search(d)
            if m:
                num = int(m.group("num"))
                numstr = "%d:   " % num
                expr = m.group("expr").strip()
                cant_eval = expr.endswith(cant_eval_str)
                if cant_eval:
                    expr = expr[:-len(cant_eval_str)].strip()
                v = None
                sym_not_init = False
                raw = False
                fmt = None
                error = None
                if m.group("enabled") != "y":
                    sym_not_init = True
                elif thread_valid and not cant_eval:
                    format_flags = m.group("fmt")
                    if format_flags:
                        raw = "r" in format_flags
                        for f in format_flags:
                            if f in formats:
                                fmt = f
                    try:
                        v = gdb.parse_and_eval(expr)
                        if v is not None:
                            if fmt == "i":
                                dis = gdb.selected_inferior().architecture().disassemble(int(v))
                                if len(dis):
                                    v = dis[0]["asm"]
                                fmt = None
                            elif fmt == "s":
                                v = v.cast(gdb.lookup_type("char").pointer())
                                fmt = None
                    except:
                        v = None
                        error = str(sys.exc_info()[1])
                        #traceback.print_exc()
                yield VarNameValue(expr, v, undecl=sym_not_init, num=num, r=raw, fmt=fmt, err=error)


template_re = None
def filter_templates(n):
    global template_re
    if template_re is None:
        template_re = re.compile(r'(?<!\boperator)(<|>)')
    level = 0
    rest_arr = []
    for s in template_re.split(n):
        if s == '<':
            level += 1
        elif s == '>':
            if level > 0:
                level -= 1
        elif level == 0:
            rest_arr.append(s)
    return ''.join(rest_arr)


class ThreadsWindow(TextWindow):
    def __init__(self, win):
        super(ThreadsWindow, self).__init__(win, "threads")

    def refill(self):
        if not self.win.is_valid():
            return
        self.lines = []
        self.threads = []
        inferior = gdb.selected_inferior()
        if inferior and inferior.is_valid():
            sel_thread = gdb.selected_thread()
            if sel_thread and sel_thread.is_valid():
                sel_frame = gdb.selected_frame()
                for thread in reversed(inferior.threads()):
                    thread.switch()
                    frame = gdb.newest_frame()
                    name = frame.name()
                    if not name:
                        name = format(frame.pc(), "#x")
                    else:
                        name = filter_templates(name)
                    num_str = "%d" % thread.num
                    if thread.ptid[1] > 0:
                        thread_id = thread.ptid[1]
                    else:
                        thread_id = thread.ptid[2]
                    id_str = "[%d]" % thread_id
                    name_col_s, name_col_e = "", ""
                    if thread.ptid == sel_thread.ptid:
                        name_col_s, name_col_e = "\033[1;37m", "\033[0m"
                    self.lines.append(num_str + ": " + name_col_s + id_str + " " + name + name_col_e + "\n")
                    self.threads.append(thread)
                sel_thread.switch()
                sel_frame.select()
        self.redraw()

    def click(self, x, y, button):
        line = y + self.line_ofs
        if button == 1 and line < len(self.threads):
            self.threads[line].switch()
            gdb.selected_frame().select(True)
            var_change_handler()


class FramesWindow(TextWindow):
    def __init__(self, win):
        super(FramesWindow, self).__init__(win, "frames")

    def refill(self):
        if not self.win.is_valid():
            return
        self.lines = []
        self.frames = []
        thread = gdb.selected_thread()
        thread_valid = thread and thread.is_valid()
        if thread_valid:
            frame = gdb.newest_frame()
            selected = gdb.selected_frame()
            num = 0
            while frame:
                name = frame.name()
                if not name:
                    name = format(frame.pc(), "#x")
                else:
                    name = filter_templates(name)
                num_str = "#%-2d " % num
                name_col_s, name_col_e = "", ""
                if frame == selected:
                    name_col_s, name_col_e = "\033[1;37m", "\033[0m"
                self.lines.append(num_str + name_col_s + name + name_col_e + "\n")
                self.frames.append(frame)
                frame = frame.older()
                num += 1
        self.redraw()

    def click(self, x, y, button):
        line = y + self.line_ofs
        if button == 1 and line < len(self.frames):
            self.frames[line].select(True)
            var_change_handler()


class MemoryWindow(TextWindow):
    def __init__(self, win):
        super(MemoryWindow, self).__init__(win, "memory")
        self.pointer = 0

    def refill(self):
        if not self.win.is_valid():
            return
        self.lines = []
        pointer = self.pointer
        try:
            mem = gdb.selected_inferior().read_memory(pointer, 16 * self.win.height)
            ptr_hex_count = int(gdb.lookup_type("void").pointer().sizeof) * 2
            ofs = 0
            for h in range(self.win.height):
                l = "0x%0*x  " % (ptr_hex_count, pointer + ofs)
                l += " ".join(["%02x%02x" % (ord(mem[ofs + i]), ord(mem[ofs + i + 1])) for i in range(0, 16, 2)])
                l += "  "
                l += "".join([self.printable(mem[ofs + i]) for i in range(16)])
                l += "\n"
                self.lines.append(l)
                ofs += 16
        except:
            self.lines = [str(sys.exc_info()[1]) + "\n"]
        self.redraw()

    def vscroll(self, num):
        self.pointer += num * 16
        if self.pointer < 0:
            self.pointer = 0
        self.refill()

    def printable(self, c):
        o = ord(c)
        if o < 0x20 or o >= 0x7f:
            return "."
        return c

class MemoryCommand(gdb.Command):
    """Set start pointer for memory window."""

    def __init__(self):
        super(MemoryCommand, self).__init__("memory", gdb.COMMAND_TUI, gdb.COMPLETE_EXPRESSION)

    def invoke(self, arg, from_tty):
        self.dont_repeat()
        global custom_windows
        if 'memory' not in custom_windows or not custom_windows['memory'].win.is_valid():
            gdb.execute("layout memory")
        memory_window = custom_windows['memory']
        memory_window.pointer = long(gdb.parse_and_eval(arg))

MemoryCommand()


gdb.register_window_type("locals", LocalsWindow)
gdb.register_window_type("display", DisplayWindow)
gdb.register_window_type("threads", ThreadsWindow)
gdb.register_window_type("frames", FramesWindow)
gdb.register_window_type("memory", MemoryWindow)


class CustomCommandWindow(TextWindow):
    def __init__(self, win, title, command):
        super(CustomCommandWindow, self).__init__(win, title)
        self.commands = command.splitlines()

    def refill(self):
        if not self.win.is_valid():
            return
        self.lines = []
        for c in self.commands:
            try:
                self.lines.extend([octal_escape(l) + "\n" for l in gdb.execute(c, to_string=True, styled=True).split("\n")])
                if (self.lines and self.lines[-1] == "\n"):
                    del self.lines[-1]
            except:
                self.lines.append(str(sys.exc_info()[1]) + "\n")
        self.redraw()

class CustomCommandWindowFactory:
    def __init__(self, title, command):
        self.title = title
        self.command = command

    def __call__(self, win):
        return CustomCommandWindow(win, self.title, self.command)

class CreateCustomCommandWindow(gdb.Command):
    """Create custom command window."""

    def __init__(self):
        super(CreateCustomCommandWindow, self).__init__("cccw", gdb.COMMAND_TUI)

    def invoke(self, arg, from_tty):
        self.dont_repeat()
        argv = gdb.string_to_argv(arg)
        if len(argv) != 2:
            raise gdb.GdbError("This expects the 2 arguments title and command.")
        gdb.register_window_type(argv[0], CustomCommandWindowFactory(argv[0], argv[1]))

CreateCustomCommandWindow()


def var_change_handler(event=None):
    global custom_windows
    for key, value in custom_windows.items():
        value.refill()

gdb.events.before_prompt.connect(var_change_handler)


gdb.execute("tui new-layout locals {-horizontal src 2 locals 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout display {-horizontal src 2 display 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout locals-display {-horizontal src 2 {locals 1 display 1} 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout threads {-horizontal src 2 threads 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout frames {-horizontal src 2 frames 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout threads-frames {-horizontal src 2 {threads 1 frames 1} 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout locals-frames {-horizontal src 2 {locals 2 frames 1} 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout display-frames {-horizontal src 2 {display 2 frames 1} 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout locals-display-frames {-horizontal src 2 {locals 2 display 2 frames 1} 1} 3 status 0 cmd 1")
gdb.execute("tui new-layout threads-locals-frames-display {-horizontal src 3 {threads 1 locals 2} 1 {frames 1 display 2} 1} 3 status 0 cmd 1")
gdb.execute("tui new-layout memory memory 1 src 2 status 0 cmd 1")
gdb.execute("tui new-layout all {-horizontal {{-horizontal asm 1 regs 1} 1 src 2} 3 {threads 1 locals 2} 1 {frames 1 display 2} 1} 3 status 0 cmd 1")

gdb.execute("alias ll = layout locals")
gdb.execute("alias ld = layout display")
gdb.execute("alias lld = layout locals-display")
gdb.execute("alias lt = layout threads")
gdb.execute("alias lf = layout frames")
gdb.execute("alias ltf = layout threads-frames")
gdb.execute("alias llf = layout locals-frames")
gdb.execute("alias ldf = layout display-frames")
gdb.execute("alias lldf = layout locals-display-frames")
gdb.execute("alias ltlfd = layout threads-locals-frames-display")
gdb.execute("alias lm = layout memory")
gdb.execute("alias la = layout all")

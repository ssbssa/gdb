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
import locale
import os
from gdb import styling

PY3 = sys.version_info[0] == 3

if PY3:
    basestring = str
    long = int

custom_windows = {}

encoding = locale.getdefaultlocale()[1]


if sys.platform == "win32":
    import ctypes
    import ctypes.wintypes as w

    CF_TEXT = 1

    u32 = ctypes.WinDLL("user32")
    k32 = ctypes.WinDLL("kernel32")

    OpenClipboard = u32.OpenClipboard
    OpenClipboard.argtypes = (w.HWND,)
    OpenClipboard.restype = w.BOOL

    EmptyClipboard = u32.EmptyClipboard
    EmptyClipboard.restype = w.BOOL

    SetClipboardData = u32.SetClipboardData
    SetClipboardData.argtypes = (
        w.UINT,
        w.HANDLE,
    )
    SetClipboardData.restype = w.HANDLE

    CloseClipboard = u32.CloseClipboard
    CloseClipboard.restype = w.BOOL

    GHND = 0x0042

    GlobalAlloc = k32.GlobalAlloc
    GlobalAlloc.argtypes = (
        w.UINT,
        ctypes.c_size_t,
    )
    GlobalAlloc.restype = w.HGLOBAL

    GlobalLock = k32.GlobalLock
    GlobalLock.argtypes = (w.HGLOBAL,)
    GlobalLock.restype = w.LPVOID

    GlobalUnlock = k32.GlobalUnlock
    GlobalUnlock.argtypes = (w.HGLOBAL,)
    GlobalUnlock.restype = w.BOOL

    def set_clipboard_text(s):
        if OpenClipboard(None):
            s = s.encode("utf-8")
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
        b64 = base64.b64encode(s.encode("utf-8")).decode()
        sys.__stdout__.write("\x1b]52;c;" + b64 + "\x07")
        sys.__stdout__.flush()


col_esc_seq_re = re.compile("(\033\\[[0-9;]*m)")


def colored_substr(text, offset, count):
    """Returns a substring ignoring the color escape sequences in offset and
    count, but keeping them in the returned string.

    The returned string contains exactly count printable characters, filling
    it with spaces if necessary."""

    col_esc_seq = False
    init_col_esc_seq = None
    last_col_esc_seq = None
    sub = ""
    for p in col_esc_seq_re.split(text):
        t = ""
        if offset > 0:
            if col_esc_seq:
                init_col_esc_seq = p
            else:
                l = len(p)
                if l <= offset:
                    offset -= l
                else:
                    if init_col_esc_seq:
                        sub += init_col_esc_seq
                    t = p[offset:]
                    offset = 0
        else:
            t = p
        if count > 0:
            if col_esc_seq:
                sub += t
            else:
                l = len(t)
                if l <= count:
                    sub += t
                    count -= l
                else:
                    sub += t[:count]
                    count = 0
        elif col_esc_seq:
            last_col_esc_seq = p
        col_esc_seq = not col_esc_seq
    if last_col_esc_seq:
        sub += last_col_esc_seq
    if count > 0:
        sub += " " * count
    return sub


class TextWindow(object):
    """Base text window class which takes care of drawing and scrolling.

    An inheriting class needs to implement the refill method which fills
    the list self.lines with the lines that should be displayed.
    This method will be called each time a prompt is presented to the user.
    """

    def __init__(self, win):
        self.win = win
        self.line_ofs = 0
        self.col_ofs = 0
        self.lines = []
        win.title = self._window_name
        global custom_windows
        custom_windows[self._window_name] = self

    def close(self):
        global custom_windows
        del custom_windows[self._window_name]

    def render(self):
        self.col_ofs = 0
        self.redraw()

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
            self.win.write(
                "".join(
                    [
                        colored_substr(l, self.col_ofs, self.win.width)
                        for l in self.lines[start:stop]
                    ]
                ),
                True,
            )
        else:
            self.win.erase()

    @classmethod
    def register_window_type(cls, window_name):
        cls._window_name = window_name
        gdb.register_window_type(window_name, cls)


def val_cmp_color(prev, cur, sym_not_init, argument, empty=False):
    """Returns the color escape sequences for variable name and value."""
    var_col_s, var_col_e, val_col_s, val_col_e = "", "", "", ""
    if empty:
        # Variable without contents
        var_col_s, var_col_e = "\033[1;30m", "\033[0m"
    elif prev is None:
        # New variable
        var_col_s, var_col_e = "\033[1;32m", "\033[0m"
    elif prev != cur:
        # Variable contents changed
        val_col_s, val_col_e = "\033[1;31m", "\033[0m"
    if argument:
        # Variable is a function argument
        var_col_s, var_col_e = "\033[1;35m", "\033[0m"
    elif sym_not_init:
        # Variable was not yet initialized
        var_col_s, var_col_e = "\033[33m", "\033[0m"
    return (var_col_s, var_col_e, val_col_s, val_col_e)


def octal_escape(s):
    """Escape non-printable characters as octal string."""
    return "".join(chr(b) if b < 128 else "\\%03o" % b for b in s.encode(encoding))


def output_encode(s):
    return s.encode(encoding)


def output_unchanged(s):
    return s


output_convert = output_unchanged

if sys.platform == "win32":
    if PY3:
        output_convert = octal_escape
    else:
        output_convert = output_encode


def value_string(valstr, hint):
    """Convert value to string representation."""
    if hint == "string":
        is_lazy = type(valstr).__name__ == "LazyString"
        if is_lazy:
            l = valstr.length
        elif not isinstance(valstr, basestring):
            l = 1
        else:
            l = len(valstr)
        if l == 0:
            if is_lazy and str(valstr.type) == "wchar_t *":
                return 'L""'
            else:
                return '""'
        else:
            return gdb.Value(valstr).format_string(symbols=False, address=False)
    if isinstance(valstr, gdb.Value):
        return valstr.format_string(symbols=False, address=False)
    elif not isinstance(valstr, basestring):
        return str(valstr)
    return valstr


def is_typedef_of(valtype, name):
    """Check for type variants."""
    if valtype.name == name:
        return True
    while valtype.code == gdb.TYPE_CODE_TYPEDEF:
        valtype = valtype.target()
        if valtype.name == name:
            return True
    return False


def is_string_element_type(t):
    """Return if an array of this type is considered a string."""
    target_type = t.target().strip_typedefs()
    return target_type.code == gdb.TYPE_CODE_INT and (
        target_type.name == "char" or is_typedef_of(t.target(), "wchar_t")
    )


class ArrayPrinter(object):
    """Pretty printer for arrays."""

    def __init__(self, v, t):
        self.v = v
        self.t = t

    def to_string(self):
        if is_string_element_type(self.t):
            return self.v.format_string(symbols=False, raw=True, address=True)

    def children(self):
        (low, high) = self.t.range()
        for i in range(low, high + 1):
            yield "[%d]" % i, self.v[i]


class StructPrinter(object):
    """Pretty printer for structs/unions."""

    def __init__(self, v, dyn_type, raw):
        self.v = v
        self.raw = raw
        self.dyn_name = None
        if dyn_type and v.type.code == gdb.TYPE_CODE_STRUCT:
            try:
                if v.type != v.dynamic_type:
                    self.v = v.cast(v.dynamic_type)
                    self.dyn_name = "<" + self.v.type.name + ">"
            except:
                pass

    def to_string(self):
        return self.dyn_name

    def children(self):
        fields = self.v.type.fields()
        for f in fields:
            if not hasattr(f, "bitpos"):
                continue
            try:
                n = f.name
            except:
                n = None
            if not n:
                n = "[anonymous]"
            elif f.is_base_class:
                n = "<" + n + ">"

            child_v = self.v[f]

            # Disable dynamic_type for base classes, to prevent infinite
            # recursion.
            if child_v.type.code == gdb.TYPE_CODE_STRUCT and f.is_base_class:
                pp = None
                if not self.raw:
                    try:
                        pp = gdb.default_visualizer(v)
                    except:
                        pass
                if pp is None:
                    child_v = StructPrinter(child_v, False, self.raw)

            yield n, child_v

    def address(self):
        return self.v.address


class PointerPrinter(object):
    """Pretty printer for pointers."""

    def __init__(self, v):
        self.v = v

    def to_string(self):
        return self.v.format_string(symbols=False, raw=True, address=True)

    def children(self):
        try:
            val = self.v.dereference()
        except Exception as e:
            val = e
        yield "[0]", val


class ValuePrinter(object):
    """Pretty printer for value types that don't have children."""

    def __init__(self, v, fmt):
        self.v = v
        self.fmt = fmt

    def to_string(self):
        if self.fmt:
            return self.v.format_string(
                symbols=False, raw=True, address=True, format=self.fmt
            )
        else:
            return self.v.format_string(symbols=False, raw=True, address=True)


class VariableWindow(TextWindow):
    """Base variable window class for interactive expansion of child values.

    An inheriting class needs to implement the variables method which returns
    an iterable of objects (based on what FrameDecorator.frame_args returns).
    These objects must implement a symbol method, returning either a Python
    string of a gdb.Symbol.
    And optionally they can implement these methods:
    - value: Returns either a Python value, a gdb.Value, or a pretty-printer
        (if not implemented, or returns None, and the symbol method returns a
        gdb.Symbol, the value of the gdb.Symbol is used).
    - undeclared: Returns a Boolean indicating if the declaration of this
        variable was not yet reached (default=False).
    - argument: Returns a Boolean indicating this variable is an argument
        (default=False).
    - number: If this returns a positive integer, it is shown before
        symbol name (default=0).
    - raw: Returns a Boolean indicating that no pretty-printers should be
        used to format the value (default=False).
    - format: Same as the format argument of Value.format_string.
    - error: Python string indicating an error message that is shown if
        method value returns None.
    - expand: Returns a Boolean indicating the children of this value should
        be immediately expanded.
    """

    def __init__(self, win, convenience_name):
        super(VariableWindow, self).__init__(win)
        self.prev_vals = {}
        self.line_names = []
        self.convenience_name = convenience_name

    def click(self, x, y, button):
        line = y + self.line_ofs
        if line < len(self.line_names):
            name = self.line_names[line]
            if name:
                prev = self.prev_vals[name]
                if button == 1:
                    # On left-click, invert expansion of children.
                    if prev is not None and prev[0] is not None:
                        prev[0] = not prev[0]
                        self.refill(True)
                        if prev[0]:
                            c = 0
                            l = len(self.line_names)
                            while line + c < l and c < self.win.height:
                                n = self.line_names[line + c]
                                if n is not None and not n.startswith(name):
                                    break
                                c += 1
                            if line + c > self.line_ofs + self.win.height:
                                self.line_ofs = line + c - self.win.height
                        self.redraw()
                elif button == 3:
                    # On right-click, set element for convenience variable.
                    if prev is not None and prev[3] is not None:
                        gdb.set_convenience_variable(self.convenience_name, prev[3])
                        self.refill(True)
                        self.redraw()
                elif button == 2:
                    # On middle-click, copy value(s) to clipboard
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
        self.lines = []
        self.line_names = []
        cur_vals = {}
        frame = None
        for v in self.variables():
            symbol = v.symbol()
            if isinstance(symbol, gdb.Symbol):
                name = symbol.name
            else:
                name = symbol

            value = None
            error = None
            if hasattr(v, "value"):
                value = v.value()
            if value is None and isinstance(symbol, gdb.Symbol):
                if frame is None:
                    frame = gdb.selected_frame()
                try:
                    value = symbol.value(frame)
                except:
                    error = str(sys.exc_info()[1])

            sym_not_init = False
            if hasattr(v, "undeclared"):
                sym_not_init = v.undeclared()

            argument = False
            if hasattr(v, "argument"):
                argument = v.argument()

            num = 0
            if hasattr(v, "number"):
                num = v.number()

            raw = False
            if hasattr(v, "raw"):
                raw = v.raw()

            fmt = None
            if hasattr(v, "format"):
                fmt = v.format()

            if error is None and hasattr(v, "error"):
                error = v.error()

            expand = False
            if hasattr(v, "expand"):
                expand = v.expand()

            self.add_val(
                name,
                name,
                value,
                0,
                num,
                cur_vals,
                keep_prev,
                expand,
                sym_not_init,
                argument,
                raw,
                fmt,
                error,
            )
        self.prev_vals = cur_vals

    def add_val(
        self,
        n,
        fn,
        v,
        inset,
        num,
        cur_vals,
        keep_prev,
        def_expand,
        sym_not_init,
        argument,
        raw,
        fmt,
        error,
    ):
        if num == 0:
            n2 = fn
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

        if isinstance(v, Exception):
            error = str(v)
            v = None

        if v is None:
            if error is None:
                (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(
                    False, False, sym_not_init, argument, empty=True
                )
                self.lines.append(spaces + numstr + "  " + var_col_s + n + var_col_e)
            else:
                (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(
                    False, False, sym_not_init, argument
                )
                self.lines.append(
                    spaces
                    + numstr
                    + "  "
                    + var_col_s
                    + n
                    + var_col_e
                    + " = \033[1;30m<"
                    + error
                    + ">\033[0m"
                )
            self.line_names.append(n2)
            return

        if isinstance(v, basestring):
            v = output_convert(v)
            (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(
                prev_val, v, sym_not_init, argument
            )
            self.lines.append(
                spaces
                + numstr
                + "  "
                + var_col_s
                + n
                + var_col_e
                + " = "
                + val_col_s
                + v
                + val_col_e
            )
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
                self.add_val(
                    n,
                    fn,
                    None,
                    inset,
                    num,
                    cur_vals,
                    keep_prev,
                    False,
                    sym_not_init,
                    argument,
                    raw,
                    fmt,
                    str(sys.exc_info()[1]),
                )
                return

            if is_optimized_out and v.type.strip_typedefs().code not in [
                gdb.TYPE_CODE_STRUCT,
                gdb.TYPE_CODE_UNION,
            ]:
                self.add_val(
                    n,
                    fn,
                    None,
                    inset,
                    num,
                    cur_vals,
                    keep_prev,
                    False,
                    sym_not_init,
                    argument,
                    raw,
                    fmt,
                    "optimized out",
                )
                return

            v_addr = v.address
        else:
            v = gdb.Value(v)

        cv_str = ""
        if v_addr is not None:
            v_addr = v_addr.dereference().reference_value()
            cur_entry[3] = v_addr
            cv = gdb.convenience_variable(self.convenience_name)
            if (
                cv is not None
                and cv.address == v_addr.address
                and cv.type == v_addr.type
            ):
                cv_str = " = \033[1;36m$" + self.convenience_name + "\033[0m"

        pp = None
        if is_pp:
            pp = v
        elif not raw:
            try:
                pp = gdb.default_visualizer(v)
            except:
                pp = None

        def_expand_child = False
        if pp is None:
            t = v.type.strip_typedefs()
            if t.code == gdb.TYPE_CODE_PTR:
                ptr_target_type = t.target().strip_typedefs().code
                def_expand_child = ptr_target_type != gdb.TYPE_CODE_PTR

            if t.code in [gdb.TYPE_CODE_ARRAY, gdb.TYPE_CODE_RANGE]:
                pp = ArrayPrinter(v, t)
            elif t.code in [gdb.TYPE_CODE_STRUCT, gdb.TYPE_CODE_UNION]:
                pp = StructPrinter(v, gdb.parameter("print object"), raw)
            elif t.code == gdb.TYPE_CODE_PTR and not ptr_target_type in [
                gdb.TYPE_CODE_FUNC,
                gdb.TYPE_CODE_METHOD,
            ]:
                pp = PointerPrinter(v)
            else:
                pp = ValuePrinter(v, fmt)

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

            valstr = None
            if hasattr(pp, "to_string"):
                valstr = pp.to_string()
            hint = None
            if hasattr(pp, "display_hint"):
                hint = pp.display_hint()

            if valstr is not None:
                valstr = value_string(valstr, hint)
                valstr = output_convert(valstr)
                sp = valstr.split("\n")
                (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(
                    prev_val, valstr, sym_not_init, argument
                )
                self.lines.append(
                    spaces
                    + numstr
                    + var_col_s
                    + n
                    + var_col_e
                    + " = "
                    + val_col_s
                    + sp.pop(0)
                    + val_col_e
                    + cv_str
                )
                self.line_names.append(n2)
                for extra in sp:
                    self.lines.append(spaces + "    " + extra)
                    self.line_names.append(None)
                cur_entry[1] = valstr
            else:
                (var_col_s, var_col_e, val_col_s, val_col_e) = val_cmp_color(
                    prev_val, False, sym_not_init, argument
                )
                self.lines.append(spaces + numstr + var_col_s + n + var_col_e + cv_str)
                self.line_names.append(n2)
        except:
            self.add_val(
                n,
                fn,
                None,
                inset,
                num,
                cur_vals,
                keep_prev,
                False,
                sym_not_init,
                argument,
                raw,
                fmt,
                str(sys.exc_info()[1]),
            )
            return

        try:
            if expand:
                childs = None
                if hasattr(pp, "children"):
                    childs = pp.children()
                if childs:
                    max_elements = gdb.parameter("print elements")
                    if hint == "map":
                        # If key can be displayed in 1 line, show it as:
                        #   [$KEY_STRING]
                        #     $VALUE_CHILDREN
                        # Otherwise, use key/value pairs:
                        #   key
                        #     $KEY_CHILDREN
                        #   value
                        #     $VALUE_CHILDREN
                        key = None
                        for count, c in enumerate(childs, 1):
                            if max_elements and count > max_elements:
                                break
                            (nc, vc) = c
                            fnc = ":".join([n2, nc])
                            if (count % 2) == 1:
                                key = None
                                if isinstance(vc, basestring):
                                    key = vc
                                else:
                                    vc_check = vc
                                    if vc_check.type.code in [
                                        gdb.TYPE_CODE_REF,
                                        gdb.TYPE_CODE_RVALUE_REF,
                                    ]:
                                        vc_check = vc_check.referenced_value()
                                    key_pp = gdb.default_visualizer(vc_check)
                                    if key_pp:
                                        if hasattr(key_pp, "to_string") and not hasattr(
                                            key_pp, "children"
                                        ):
                                            vc_check = key_pp.to_string()
                                            if vc_check is not None:
                                                hint = None
                                                if hasattr(key_pp, "display_hint"):
                                                    hint = key_pp.display_hint()
                                                key = value_string(vc_check, hint)
                                    else:
                                        t_check = vc_check.type.strip_typedefs()
                                        if t_check.code in [
                                            gdb.TYPE_CODE_ENUM,
                                            gdb.TYPE_CODE_INT,
                                            gdb.TYPE_CODE_FLT,
                                            gdb.TYPE_CODE_BOOL,
                                            gdb.TYPE_CODE_COMPLEX,
                                            gdb.TYPE_CODE_DECFLOAT,
                                        ]:
                                            if fmt:
                                                key = vc_check.format_string(
                                                    symbols=False, raw=True, format=fmt
                                                )
                                            else:
                                                key = vc_check.format_string(
                                                    symbols=False, raw=True
                                                )
                                if key is not None and "\n" in key:
                                    key = None
                                if key is None:
                                    self.add_val(
                                        "key",
                                        fnc,
                                        vc,
                                        inset + 1,
                                        0,
                                        cur_vals,
                                        keep_prev,
                                        False,
                                        False,
                                        False,
                                        raw,
                                        fmt,
                                        None,
                                    )
                            else:
                                if key is not None:
                                    self.add_val(
                                        "[" + str(key) + "]",
                                        fnc,
                                        vc,
                                        inset + 1,
                                        0,
                                        cur_vals,
                                        keep_prev,
                                        False,
                                        False,
                                        False,
                                        raw,
                                        fmt,
                                        None,
                                    )
                                else:
                                    self.add_val(
                                        "value",
                                        fnc,
                                        vc,
                                        inset + 1,
                                        0,
                                        cur_vals,
                                        keep_prev,
                                        False,
                                        False,
                                        False,
                                        raw,
                                        fmt,
                                        None,
                                    )
                    else:
                        for count, c in enumerate(childs, 1):
                            if max_elements and count > max_elements:
                                break
                            (nc, vc) = c
                            fnc = ":".join([n2, nc])
                            self.add_val(
                                nc,
                                fnc,
                                vc,
                                inset + 1,
                                0,
                                cur_vals,
                                keep_prev,
                                def_expand_child,
                                False,
                                False,
                                raw,
                                fmt,
                                None,
                            )
        except:
            self.add_val(
                n,
                fn,
                None,
                inset + 1,
                0,
                cur_vals,
                keep_prev,
                False,
                sym_not_init,
                argument,
                raw,
                fmt,
                str(sys.exc_info()[1]),
            )


class VarNameValue(object):
    """Object returned by LocalsWindow.variables."""

    def __init__(
        self,
        sym,
        val=None,
        undecl=False,
        arg=False,
        num=0,
        r=False,
        fmt=None,
        err=None,
        exp=False,
    ):
        self.sym = sym
        self.val = val
        self.undecl = undecl
        self.arg = arg
        self.num = num
        self.r = r
        self.fmt = fmt
        self.err = err
        self.exp = exp

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

    def expand(self):
        return self.exp


class LocalsWindow(VariableWindow):
    """Window for local variables."""

    def __init__(self, win):
        super(LocalsWindow, self).__init__(win, "lv")

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
                        if (
                            symbol.is_argument
                            or symbol.is_variable
                            or symbol.is_constant
                        ):
                            sym_not_init = (
                                symbol.is_variable
                                and symbol.line > 0
                                and cur_line <= symbol.line
                            )
                            error = None
                            try:
                                v = symbol.value(frame)
                            except:
                                v = None
                                error = str(sys.exc_info()[1])
                            yield VarNameValue(
                                symbol.name,
                                val=v,
                                undecl=sym_not_init,
                                arg=symbol.is_argument,
                                err=error,
                            )
                if block.function:
                    break
                block = block.superblock


class DisplayWindow(VariableWindow):
    def __init__(self, win):
        super(DisplayWindow, self).__init__(win, "dv")
        self.disp_re = re.compile(
            "^(?P<num>\\d+): +(?P<enabled>[yn]) +(?P<fmt>\\/\\w+ +)?(?P<expr>.*)"
        )
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
                expr = m.group("expr").strip()
                cant_eval = expr.endswith(cant_eval_str)
                if cant_eval:
                    expr = expr[: -len(cant_eval_str)].strip()
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
                                dis = (
                                    gdb.selected_inferior()
                                    .architecture()
                                    .disassemble(int(v))
                                )
                                if len(dis):
                                    v = dis[0]["asm"]
                                fmt = None
                            elif fmt == "s":
                                v = v.cast(gdb.lookup_type("char").pointer())
                                fmt = None
                    except:
                        v = None
                        error = str(sys.exc_info()[1])
                yield VarNameValue(
                    expr, v, undecl=sym_not_init, num=num, r=raw, fmt=fmt, err=error
                )


template_re = None


def filter_templates(n):
    global template_re
    if template_re is None:
        template_re = re.compile(r"(?<!\boperator)(<|>)")
    level = 0
    rest_arr = []
    for s in template_re.split(n):
        if s == "<":
            level += 1
        elif s == ">":
            if level > 0:
                level -= 1
        elif level == 0:
            rest_arr.append(s)
    return "".join(rest_arr)


class ChoiceWindow(TextWindow):
    def refill(self):
        self.lines = []
        for prefix, text, selected in self.choices():
            text_col_s, text_col_e = "", ""
            if selected:
                text_col_s, text_col_e = "\033[1;37m", "\033[0m"
            self.lines.append(prefix + text_col_s + text + text_col_e)

    def click(self, x, y, button):
        line = y + self.line_ofs
        if button == 1 and line < len(self.lines):
            self.select(line)
            refresh_tui_windows()


class ThreadsWindow(ChoiceWindow):
    def choices(self):
        self.threads = []
        inferior = gdb.selected_inferior()
        if inferior and inferior.is_valid():
            sel_thread = gdb.selected_thread()
            if sel_thread and sel_thread.is_valid():
                sel_frame = gdb.selected_frame()
                try:
                    for thread in reversed(inferior.threads()):
                        thread.switch()
                        frame = gdb.newest_frame()
                        name = frame.name()
                        if not name:
                            name = format(frame.pc(), "#x")
                        else:
                            name = filter_templates(name)
                        num_str = "%d: " % thread.num
                        if thread.ptid[1] > 0:
                            thread_id = thread.ptid[1]
                        else:
                            thread_id = thread.ptid[2]
                        text = "[%d] %s" % (thread_id, name)

                        self.threads.append(thread)
                        yield num_str, text, thread.ptid == sel_thread.ptid
                finally:
                    sel_thread.switch()
                    sel_frame.select()

    def select(self, line):
        self.threads[line].switch()
        gdb.selected_frame().select(True)


class FramesWindow(ChoiceWindow):
    def choices(self):
        self.frames = []
        thread = gdb.selected_thread()
        thread_valid = thread and thread.is_valid()
        if thread_valid:
            frame = gdb.newest_frame()
            selected = gdb.selected_frame()
            while frame:
                name = frame.name()
                if not name:
                    name = format(frame.pc(), "#x")
                else:
                    name = filter_templates(name)
                num_str = "#%-2d " % frame.level()

                self.frames.append(frame)
                yield num_str, name, frame == selected

                frame = frame.older()

    def select(self, line):
        self.frames[line].select(True)


class MemoryWindow(TextWindow):
    def __init__(self, win):
        super(MemoryWindow, self).__init__(win)
        self.pointer = 0

    def refill(self):
        self.lines = []
        pointer = self.pointer
        try:
            mem = gdb.selected_inferior().read_memory(pointer, 16 * self.win.height)
            ptr_hex_count = int(gdb.lookup_type("void").pointer().sizeof) * 2
            ofs = 0
            for h in range(self.win.height):
                l = "0x%0*x  " % (ptr_hex_count, pointer + ofs)
                l += " ".join(
                    [
                        "%02x%02x" % (ord(mem[ofs + i]), ord(mem[ofs + i + 1]))
                        for i in range(0, 16, 2)
                    ]
                )
                l += "  "
                l += "".join([self.printable(mem[ofs + i]) for i in range(16)])
                self.lines.append(l)
                ofs += 16
        except:
            self.lines = [str(sys.exc_info()[1])]

    def vscroll(self, num):
        self.pointer += num * 16
        if self.pointer < 0:
            self.pointer = 0
        self.refill()
        self.redraw()

    def printable(self, c):
        o = ord(c)
        if o < 0x20 or o >= 0x7F:
            return "."
        return chr(o)


class MemoryCommand(gdb.Command):
    """Set start pointer for memory window."""

    def __init__(self):
        super(MemoryCommand, self).__init__(
            "memory", gdb.COMMAND_TUI, gdb.COMPLETE_EXPRESSION
        )

    def invoke(self, arg, from_tty):
        self.dont_repeat()
        global custom_windows
        if (
            "memory" not in custom_windows
            or not custom_windows["memory"].win.is_valid()
        ):
            gdb.execute("layout memory")
        memory_window = custom_windows["memory"]
        memory_window.pointer = long(gdb.parse_and_eval(arg))


MemoryCommand()


LocalsWindow.register_window_type("locals")
DisplayWindow.register_window_type("display")
ThreadsWindow.register_window_type("threads")
FramesWindow.register_window_type("frames")
MemoryWindow.register_window_type("memory")


class CustomCommandWindow(TextWindow):
    def __init__(self, win, title, command):
        self._window_name = title
        super(CustomCommandWindow, self).__init__(win)
        self.commands = command.splitlines()

    def refill(self):
        self.lines = []
        for c in self.commands:
            try:
                self.lines.extend(
                    [
                        output_convert(l)
                        for l in gdb.execute(c, to_string=True, styled=True).split("\n")
                    ]
                )
                if self.lines and self.lines[-1] == "":
                    del self.lines[-1]
            except:
                self.lines.append(str(sys.exc_info()[1]))


class CustomCommandWindowFactory(object):
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


class SourceWindow(TextWindow):
    def __init__(self, win):
        super(SourceWindow, self).__init__(win)
        self.filename = None
        self.highlight = 0
        self.source = []

    def refill(self):
        self.lines = []

        filename, lineno, highlight = self.location()
        if filename != self.filename:
            self.filename = filename
            self.highlight = 0
            self.source = []

            if filename is not None:
                try:
                    f = open(os.fsencode(filename), "r")
                except:
                    return
                with f:
                    source = f.read()
                    source_styled = styling.colorize(filename, source)
                    if source_styled:
                        self.source = source_styled.decode(
                            gdb.host_charset()
                        ).splitlines()
                    else:
                        self.source = source.splitlines()

            if lineno is None or lineno == 0:
                lineno = 1
            if highlight is None:
                highlight = 0

        if highlight is not None:
            self.highlight = highlight

        if self.filename is None:
            self.win.title = "(no source available)"
            return

        self.win.title = self.filename

        for l, line in enumerate(self.source, 1):
            marker = " "
            num_str = "%6d  " % l
            line_col_s, line_col_e = "", ""
            if l == self.highlight:
                marker = ">"
                line_col_s, line_col_e = "\033[7m", "\033[27m"
                line = col_esc_seq_re.sub("", line)
            self.lines.append(marker + num_str + line_col_s + line + line_col_e)
        if lineno > 0 and lineno <= len(self.lines):
            if self.line_ofs >= lineno or self.line_ofs + self.win.height < lineno:
                self.line_ofs = max(lineno - 1 - self.win.height // 2, 0)


def refresh_tui_windows(event=None):
    """Refresh all TextWindow instances when user is presented a prompt."""
    global custom_windows
    for key, value in custom_windows.items():
        if value.win.is_valid():
            value.refill()
            value.redraw()


gdb.events.before_prompt.connect(refresh_tui_windows)


gdb.execute("tui new-layout locals {-horizontal src 2 locals 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout display {-horizontal src 2 display 1} 2 status 0 cmd 1")
gdb.execute(
    "tui new-layout locals-display {-horizontal src 2 {locals 1 display 1} 1} 2 status 0 cmd 1"
)
gdb.execute("tui new-layout threads {-horizontal src 2 threads 1} 2 status 0 cmd 1")
gdb.execute("tui new-layout frames {-horizontal src 2 frames 1} 2 status 0 cmd 1")
gdb.execute(
    "tui new-layout threads-frames {-horizontal src 2 {threads 1 frames 1} 1} 2 status 0 cmd 1"
)
gdb.execute(
    "tui new-layout locals-frames {-horizontal src 2 {locals 2 frames 1} 1} 2 status 0 cmd 1"
)
gdb.execute(
    "tui new-layout display-frames {-horizontal src 2 {display 2 frames 1} 1} 2 status 0 cmd 1"
)
gdb.execute(
    "tui new-layout locals-display-frames {-horizontal src 2 {locals 2 display 2 frames 1} 1} 3 status 0 cmd 1"
)
gdb.execute(
    "tui new-layout threads-locals-frames-display {-horizontal src 3 {threads 1 locals 2} 1 {frames 1 display 2} 1} 3 status 0 cmd 1"
)
gdb.execute("tui new-layout memory memory 1 src 2 status 0 cmd 1")

try:
    gdb.execute(
        "tui new-layout locals-output {-horizontal src 2 locals 1} 2 output 1 status 0 cmd 1"
    )
    gdb.execute(
        "tui new-layout display-output {-horizontal src 2 display 1} 2 output 1 status 0 cmd 1"
    )
    gdb.execute(
        "tui new-layout locals-display-output {-horizontal src 2 {locals 1 display 1} 1} 2 output 1 status 0 cmd 1"
    )
    gdb.execute(
        "tui new-layout threads-output {-horizontal src 2 threads 1} 2 output 1 status 0 cmd 1"
    )
    gdb.execute(
        "tui new-layout frames-output {-horizontal src 2 frames 1} 2 output 1 status 0 cmd 1"
    )
    gdb.execute(
        "tui new-layout threads-frames-output {-horizontal src 2 {threads 1 frames 1} 1} 2 output 1 status 0 cmd 1"
    )
    gdb.execute(
        "tui new-layout locals-frames-output {-horizontal src 2 {locals 2 frames 1} 1} 2 output 1 status 0 cmd 1"
    )
    gdb.execute(
        "tui new-layout display-frames-output {-horizontal src 2 {display 2 frames 1} 1} 2 output 1 status 0 cmd 1"
    )
    gdb.execute(
        "tui new-layout locals-display-frames-output {-horizontal src 2 {locals 2 display 2 frames 1} 1} 3 output 1 status 0 cmd 1"
    )
    gdb.execute(
        "tui new-layout threads-locals-frames-display-output {-horizontal src 3 {threads 1 locals 2} 1 {frames 1 display 2} 1} 3 output 1 status 0 cmd 1"
    )
except:
    pass

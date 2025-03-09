import gdb
import ctypes
import sys


class WinError(gdb.Command):
    """Show the string describing a windows error code.
    If no error code is specified, the last error code of the currently
    selected thread is used.

    Usage: winerror [ERROR-CODE]"""

    def __init__(self):
        super(WinError, self).__init__(
            "winerror", gdb.COMMAND_DATA, gdb.COMPLETE_EXPRESSION
        )

    def invoke(self, arg, from_tty):
        if arg:
            error = gdb.parse_and_eval(arg)
        else:
            tlb = gdb.convenience_variable("_tlb")
            if not tlb:
                raise gdb.GdbError(
                    "winerror requires an argument or a running inferior"
                )
            error = tlb["last_error_number"]
        if error:
            print("%d: %s" % (error, ctypes.FormatError(error)))
        else:
            print("0: No error.")


if sys.platform == "win32":
    WinError()

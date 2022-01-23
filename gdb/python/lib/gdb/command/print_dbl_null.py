import gdb


class PrintDblNull(gdb.Command):
    """Print a double-null-terminated string list.

    Usage: print_dbl_null VALUE"""

    def __init__(self):
        super(PrintDblNull, self).__init__("print_dbl_null", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        env = gdb.parse_and_eval(arg)
        if env.type.code == gdb.TYPE_CODE_TYPEDEF:
            env = env.cast(env.type.target())
        while env[0]:
            print(env.format_string(address=False))
            env += len(env.string()) + 1


PrintDblNull()

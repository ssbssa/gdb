import gdb


class SetArr(gdb.Command):
    """Assign array contents ARR to variable VAR.
    Usage: setarr VAR = ARR"""

    def __init__(self):
        super(SetArr, self).__init__("setarr", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        args = arg.split("=", maxsplit=1)
        if len(args) != 2:
            raise gdb.GdbError("no assignment operator in argument")

        dest = gdb.parse_and_eval(args[0])
        dest_type = dest.type.strip_typedefs()
        if dest_type.code not in [gdb.TYPE_CODE_PTR, gdb.TYPE_CODE_ARRAY]:
            raise gdb.GdbError("destination has to be an array or a pointer")

        src = gdb.parse_and_eval(args[1])
        src_type = src.type.strip_typedefs()
        if src_type.code != gdb.TYPE_CODE_ARRAY:
            raise gdb.GdbError("source has to be an array")

        if (
            dest_type.code == gdb.TYPE_CODE_ARRAY
            and dest_type.range()[1] < src_type.range()[1]
        ):
            raise gdb.GdbError("destination is smaller than source")
        if dest_type.target().sizeof < src_type.target().sizeof:
            raise gdb.GdbError("destination element is smaller than source element")

        for i in range(src_type.range()[1] + 1):
            dest[i].assign(src[i])


SetArr()

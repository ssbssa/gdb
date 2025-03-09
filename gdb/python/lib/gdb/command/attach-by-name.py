import gdb
import sys
import ntpath
import os


if sys.platform == "win32":

    def get_executable_name(p):
        if not p:
            return None
        name = ntpath.basename(p)
        if name.endswith(".exe"):
            name = name[:-4]
        return name

    def exe_from_info_os_proc(p):
        return p["col3"]


else:

    def get_executable_name(p):
        if not p:
            return None
        return os.path.basename(p)

    def exe_from_info_os_proc(p):
        return p["col2"].split(" ", 1)[0]


class AttachByName(gdb.Command):
    """Attach to a process by name.
    If no executable name is specified, the executable name of the current
    inferior is used.

    Usage: attach-by-name [EXECUTABLE-NAME]"""

    def __init__(self):
        super(AttachByName, self).__init__("attach-by-name", gdb.COMMAND_DATA)

    def invoke(self, arg, from_tty):
        if arg:
            exe = get_executable_name(arg)
        else:
            exe = get_executable_name(gdb.current_progspace().executable_filename)
            if not exe:
                raise gdb.GdbError(
                    "attach-by-name requires an argument or an executable in the current inferior"
                )

        native_pid = -1
        if (
            gdb.selected_inferior().connection is None
            or gdb.selected_inferior().connection.type == "native"
        ):
            native_pid = os.getpid()

        matches = []
        for p in gdb.execute_mi("info os processes")["OSDataTable"]:
            if (
                get_executable_name(exe_from_info_os_proc(p)) == exe
                and int(p["col0"]) != native_pid
            ):
                matches.append(p)
        if not matches:
            raise gdb.GdbError("No process named %s found" % exe)
        if len(matches) > 1:
            for p in matches:
                print("pid=%s; user=%s; cmd=%s" % (p["col0"], p["col1"], p["col2"]))
            raise gdb.GdbError("%d processes named %s found" % (len(matches), exe))
        gdb.execute("attach %s" % matches[0]["col0"], from_tty)


AttachByName()

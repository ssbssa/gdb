import gdb.printing


class wchar_t_listPrinter:
    "Print a gdb_wchar_t_list"

    def __init__(self, val):
        self.val = val

    def children(self):
        wchar_t_ptr = self.val.type.target()
        wchar_t = wchar_t_ptr.target()
        env = self.val.cast(wchar_t_ptr)
        while env[0]:
            pair = env.string()
            assign = pair.index("=")
            yield pair[0:assign], env + assign + 1
            env += len(pair) + 1


winproc = gdb.printing.RegexpCollectionPrettyPrinter("winproc")
winproc.add_printer("gdb_wchar_t_list", "^gdb_wchar_t_list$", wchar_t_listPrinter)
gdb.printing.register_pretty_printer(None, winproc)

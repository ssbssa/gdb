import gdb.xmethod


def get_size_type():
    if gdb.lookup_type("void").pointer().sizeof == 4:
        return gdb.lookup_type("unsigned int")
    else:
        return gdb.lookup_type("unsigned long long")


class VectorPrinter(object):
    def __init__(self, printer, val, kwargs):
        self._printer = printer
        self._val = val
        self._kwargs = kwargs

    def children(self):
        count = int(self._printer.size(self._val, **self._kwargs))
        if hasattr(self._printer, "subscript"):
            for i in range(count):
                yield "[%d]" % i, self._printer.subscript(self._val, i, **self._kwargs)
        else:
            dereference = False
            if hasattr(self._printer, "dereference"):
                dereference = self._printer.dereference(self._val, **self._kwargs)
            data = self._printer.data(self._val, **self._kwargs)
            for i in range(count):
                sub = data[i]
                if dereference:
                    sub = sub.dereference()
                yield "[%d]" % i, sub

    def to_string(self):
        if hasattr(self._printer, "to_string"):
            return self._printer.to_string(self._val, **self._kwargs)
        if hasattr(self._printer, "name"):
            name = self._printer.name(self._val, **self._kwargs)
        else:
            name = self._val.type.name
        return "%s of length %d" % (
            name,
            int(self._printer.size(self._val, **self._kwargs)),
        )

    def address(self):
        if hasattr(self._printer, "address"):
            return self._printer.address()
        return self._val.address

    def display_hint(self):
        return "array"


class VectorXMethodSubscript(gdb.xmethod.XMethodWorker):
    def __init__(self, printer, child_type):
        self._printer = printer
        self._child_type = child_type

    def get_arg_types(self):
        if hasattr(self._printer, "size_type"):
            return self._printer.size_type()
        return get_size_type()

    def get_result_type(self, val, subscript):
        return self._child_type

    def __call__(self, val, subscript):
        deref = val.dereference()
        if hasattr(self._printer, "subscript"):
            return self._printer.subscript(deref, subscript)
        else:
            sub = self._printer.data(deref)[subscript]
            if hasattr(self._printer, "dereference") and self._printer.dereference(
                deref
            ):
                sub = sub.dereference()
            return sub


class VectorXMethodSize(gdb.xmethod.XMethodWorker):
    def __init__(self, printer, child_type):
        self._printer = printer

    def get_arg_types(self):
        return None

    def get_result_type(self, val):
        if hasattr(self._printer, "size_type"):
            return self._printer.size_type()
        return get_size_type()

    def __call__(self, val):
        return int(self._printer.size(val.dereference()))


class VectorXMethodEmpty(gdb.xmethod.XMethodWorker):
    def __init__(self, printer, child_type):
        self._printer = printer

    def get_arg_types(self):
        return None

    def get_result_type(self, val):
        return gdb.lookup_type("bool")

    def __call__(self, val):
        return int(self._printer.size(val.dereference())) == 0


class VectorXMethodFront(gdb.xmethod.XMethodWorker):
    def __init__(self, printer, child_type):
        self._printer = printer
        self._child_type = child_type

    def get_arg_types(self):
        return None

    def get_result_type(self, val):
        return self._child_type

    def __call__(self, val):
        deref = val.dereference()
        if hasattr(self._printer, "subscript"):
            return self._printer.subscript(deref, 0)
        else:
            sub = self._printer.data(deref)[0]
            if hasattr(self._printer, "dereference") and self._printer.dereference(
                deref
            ):
                sub = sub.dereference()
            return sub


class VectorXMethodBack(gdb.xmethod.XMethodWorker):
    def __init__(self, printer, child_type):
        self._printer = printer
        self._child_type = child_type

    def get_arg_types(self):
        return None

    def get_result_type(self, val):
        return self._child_type

    def __call__(self, val):
        deref = val.dereference()
        i = int(self._printer.size(deref)) - 1
        if hasattr(self._printer, "subscript"):
            return self._printer.subscript(deref, i)
        else:
            sub = self._printer.data(deref)[i]
            if hasattr(self._printer, "dereference") and self._printer.dereference(
                deref
            ):
                sub = sub.dereference()
            return sub


class VectorXMethodData(gdb.xmethod.XMethodWorker):
    def __init__(self, printer, child_type):
        self._printer = printer
        self._child_type = child_type

    def get_arg_types(self):
        return None

    def get_result_type(self, val):
        return self._child_type.pointer()

    def __call__(self, val):
        return self._printer.data(val.dereference())


class VectorXMethod(gdb.xmethod.XMethod):
    def __init__(self, name, worker):
        gdb.xmethod.XMethod.__init__(self, name)
        self._worker = worker


class VectorPrinterXMethodMatcher(gdb.xmethod.XMethodMatcher):
    def __init__(self, printer):
        gdb.xmethod.XMethodMatcher.__init__(self, printer.class_name())
        self._printer = printer
        self.methods = [
            VectorXMethod("operator[]", VectorXMethodSubscript),
            VectorXMethod("size", VectorXMethodSize),
            VectorXMethod("empty", VectorXMethodEmpty),
            VectorXMethod("front", VectorXMethodFront),
            VectorXMethod("back", VectorXMethodBack),
        ]
        if hasattr(self._printer, "data"):
            self.methods.append(VectorXMethod("data", VectorXMethodData))

    # pretty printer
    def __call__(self, val, **kwargs):
        return VectorPrinter(self._printer, val, kwargs)

    # xmethods
    def match(self, class_type, method_name):
        if hasattr(self._printer, "is_class_type"):
            class_matches = self._printer.is_class_type(class_type)
        else:
            class_matches = class_type.tag == self._printer.class_name()
        if class_matches:
            if hasattr(self._printer, "child_type"):
                if isinstance(class_matches, dict):
                    child_type = self._printer.child_type(class_type, **class_matches)
                else:
                    child_type = self._printer.child_type(class_type)
            else:
                child_type = class_type.template_argument(0)
            for method in self.methods:
                if method.enabled and method_name == method.name:
                    return method._worker(self._printer, child_type)

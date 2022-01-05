
import gdb


class CallerVar(gdb.Function):
    """Return the value of a calling function's variable.

    Usage: $_caller_var (NAME [, NUMBER-OF-FRAMES [, DEFAULT-VALUE]])

    Arguments:

      NAME: The name of the variable.

      NUMBER-OF-FRAMES: How many stack frames to traverse back from the
	currently selected frame to compare with.
        The default is 1.

      DEFAULT-VALUE: Return value if the variable can't be found.
        The default is 0.

    Returns:
      The value of the variable in the specified frame, DEFAULT-VALUE if the
      variable can't be found."""

    def __init__(self):
        super(CallerVar, self).__init__("_caller_var")

    def invoke(self, name, nframes=1, defvalue=0):
        if nframes < 0:
            raise ValueError("nframes must be >= 0")
        frame = gdb.selected_frame()
        while nframes > 0:
            frame = frame.older()
            if frame is None:
                return defvalue
            nframes = nframes - 1
        try:
            return frame.read_var(name.string())
        except:
            return defvalue


CallerVar()

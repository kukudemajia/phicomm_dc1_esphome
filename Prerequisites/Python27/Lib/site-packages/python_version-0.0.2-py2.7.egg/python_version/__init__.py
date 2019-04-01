import platform
import sys


def check(name=None, min=None, max=None, exit_on_error=False):
    message = None
    if min is not None and sys.version_info < min:
        message = build_message(name, min=min)
    elif max is not None and sys.version_info > max:
        message = build_message(name, max=max)
    if message is not None:
        if exit_on_error:
            sys.stderr.write(message + "\n")
            sys.exit(1)
        else:
            raise VersionConflict(message)


def build_message(name, min=None, max=None):
    if name is not None:
        prefix = name + " "
    else:
        prefix = ""

    if min is not None:
        relation = ">="
        version = min
    else:
        relation = "<="
        version = max

    return prefix + "requires python version " + relation + " " + \
        ".".join(str(v) for v in version) + \
        " but the running python version is " + \
        platform.python_version()


class VersionConflict(RuntimeError):
    pass

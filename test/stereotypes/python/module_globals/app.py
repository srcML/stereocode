# A global of another module is reached through the name of the module

import settings
from settings import level


# @stereotype global-command
def reset_level():
    settings.level = 0


# @stereotype unclassified
def read_level():
    return settings.level


# A call on a global of another module other than a query method changes the global
# @stereotype global-command wrapper
def remember(name):
    settings.names.append(name)


# A name without the module reaches only the globals of its own file, and an imported name is no variable
# @stereotype literal
def copied_level():
    return level


# A name of a module outside the archive is no global
# @stereotype literal
def path_separator():
    return os.sep


class Monitor:
    def __init__(self):
        self.seen = 0

    # A global is no state of the object
    # @stereotype incidental
    def bump(self):
        settings.level += 1

    # @stereotype set
    def watch(self):
        self.seen = settings.level

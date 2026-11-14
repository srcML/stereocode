# The globals of the module settings, which the module app reaches through the name of the module

level = 1
names = []


# @stereotype global-command
def raise_level():
    global level
    level += 1

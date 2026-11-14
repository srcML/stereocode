# Locals and globals: a name that a function binds is a local, and a name bound at the top level of a file is a global

limit = 10
names = []
low, high = 0, 100


# A name that the body binds is a local in the whole body, which hides the global of its name
# @stereotype literal
def shadow():
    limit = 3
    return limit


# A name of a global statement is the global of the file
# @stereotype global-command
def raise_limit():
    global limit
    limit += 1


# A function that reads a global is no literal
# @stereotype unclassified
def read_limit():
    return limit


# Each name of a tuple of targets is a global
# @stereotype unclassified
def span():
    return high - low


# A call on a global other than a query method changes the global
# @stereotype global-command wrapper
def remember(name):
    names.append(name)


# A query method only reads the global
# @stereotype property wrapper
def known(name):
    return names.count(name)


# The target of a loop is a local
# @stereotype property
def total(values):
    result = 0
    for value in values:
        result += value
    return result


# The target of a with clause, of an except clause, of a comprehension, and of := is a local
# @stereotype property wrapper
def first_line(path):
    with open(path) as handle:
        return handle.readline()


# @stereotype property
def doubled(values):
    return [value * 2 for value in values]


# @stereotype property
def half(values):
    if (count := len(values)) > 1:
        return count / 2
    return 0


# A write of a local changes no global and no parameter
# @stereotype unclassified
def rebind(values):
    values = []
    limit = values


# A write through a member of a local changes another object
# @stereotype wrapper
def configure(options):
    parser = make_parser(options)
    parser.strict = True

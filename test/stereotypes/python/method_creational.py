# Creational methods (Method-Stereotypes wiki): constructor, destructor, and factory (Python has no copy constructor)

class Matrix:
    # __new__ takes the class and creates the object
    # @stereotype constructor
    def __new__(cls, rows: int = 0):
        return super().__new__(cls)

    # @stereotype constructor
    def __init__(self, rows: int = 0):
        self.rows = rows

    # @stereotype destructor
    def __del__(self):
        self.rows = 0

    # A call of a class creates an object, as new does in the other languages
    # @stereotype factory stateless
    def make_empty(self) -> "Matrix":
        return Matrix()

    # @stereotype factory stateless
    def make_square(self, count: int) -> "Matrix":
        result = Matrix(count)
        return result

    # Without an annotation, the class of the returned object is the return type
    # @stereotype factory stateless
    def make_row(self):
        return Matrix(1)

    # A class outside the archive is known by the CapWords convention of its name
    # @stereotype factory collaborator stateless
    def make_view(self):
        view = MatrixView()
        return view

    # The class of the object creates an object of the own type
    # @stereotype factory stateless
    def make_same(self):
        return self.__class__()

    # A function with a lowercase name creates no object
    # @stereotype wrapper stateless
    def make_copy(self):
        return deepcopy(Matrix)


class token:
    def __init__(self):
        self.text = ""


class Scanner:
    def __init__(self, source):
        self.source = source

    # A call of a class of the archive creates an object, also where the name of the class is lowercase
    # @stereotype factory collaborator stateless
    def make_token(self):
        return token()

    # @stereotype factory collaborator stateless
    def make_other_token(self):
        return lexer.token()

    # A name that is called on an object names a method of the object, also where a class of the archive has the name
    # @stereotype command
    def next_token(self):
        self.source.token()

    # @stereotype controller stateless
    def skip_token(self, stream):
        stream.token()

    # A method of another object may have a name of the form of the name of a class, as the methods of an expat parser have
    # @stereotype command
    def parse(self, data):
        self.source.Parse(data)

    # @stereotype controller stateless
    def parse_with(self, parser, data):
        parser.Parse(data)

    # A class of a module is reached through the name of the module
    # @stereotype factory collaborator stateless
    def make_parser(self):
        return expat.ParserCreate()


class Morsel:
    def __init__(self):
        self.value = ""

    # @stereotype property
    def OutputString(self):
        return "v=" + self.value

    # A method of the class is called, not created, also where its name has the form of the name of a class
    # @stereotype property
    def output(self):
        return "Set-Cookie: " + self.OutputString()

    # A name that is called on the object and is no class of the archive names a method, which a parent outside the archive
    # or the object itself may define at run time, also where the name has the form of the name of a class
    # @stereotype non-void-command
    def make_handler(self):
        return self.HandlerClass()


class SignedMorsel(Morsel):
    # @stereotype property
    def signed(self):
        return self.OutputString() + "!"


class Server:
    ParserClass = None

    def __init__(self, handler):
        self.HandlerClass = handler

    # A data member or a variable of the class with a name of the form of the name of a class holds a class, whose call creates an object
    # The call reads the data member, which a name reached through the object is
    # @stereotype property factory collaborator
    def make_handler(self):
        return self.HandlerClass()

    # @stereotype property factory collaborator
    def make_parser(self):
        return self.ParserClass()


class Window(Frame):
    def __init__(self):
        self.title = ""

    # A method of a parent outside the archive may have a name of the form of the name of a class, as the methods of wxPython have
    # @stereotype command
    def open(self):
        self.Show()


class Tree:
    class Node:
        def __init__(self):
            self.value = 0

    def __init__(self):
        self.root = None

    # A class of the archive that is called on the object creates an object
    # @stereotype set collaborator
    def plant(self):
        self.root = self.Node()

    # @stereotype factory collaborator stateless
    def sprout(self):
        return self.Node()

    # A class in the body of the class is no data member, so its name after the object reads no state
    # @stereotype incidental
    def node_class(self):
        return self.Node


user32 = ctypes.windll.user32


class Dialog:
    def __init__(self, handle):
        self.handle = handle

    # A name that is called on a global of the file names a method of the object that the global holds, also where the name has
    # the form of the name of a class, as the functions of a library that ctypes loads have
    # @stereotype wrapper
    def invalidate(self):
        user32.InvalidateRect(self.handle, 0, 0)

    # @stereotype property wrapper
    def send(self, message):
        return user32.SendMessageW(self.handle, message, 0, 0)


try:
    import ssl
except ImportError:
    ssl = None


class Channel:
    def __init__(self):
        self.buffer = None

    # A name that the file imports names a module, also where the file binds the name again when the import fails,
    # so a call of a class of the module creates an object
    # @stereotype set collaborator
    def open(self):
        self.buffer = ssl.MemoryBIO()

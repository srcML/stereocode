// A macro after the first name of a C++ return type is no part of the type, so it makes no collaborator and no non-void-command:
// a calling convention, which the specifiers of C++ list, or a name in capital letters, digits, and underscores that names no type
// of the archive, such as SCI_METHOD of Scintilla (Method-Stereotypes wiki, collaborator, command, and incidental)

#define SCI_METHOD __stdcall
#define CALLING_CONVENTION __stdcall

class Lexer {
    int state;
    int flags;
public:
    // delete this uses the object itself, which keeps the method from incidental
    // @stereotype unclassified
    void SCI_METHOD Release() { delete this; }
    // @stereotype command
    void SCI_METHOD Clear() { state = 0; flags = 0; }
    void SCI_METHOD Reset();
    // @stereotype incidental
    int SCI_METHOD Version() const { return 2; }
    // @stereotype get
    int SCI_METHOD State() const { return state; }
    // @stereotype predicate
    bool SCI_METHOD IsEmpty() const { return state == 0; }
    // A macro after a pointer declarator leaves the type as well
    // @stereotype incidental
    const char *SCI_METHOD Name() const { return "lexer"; }
    // The first name always stays, as HANDLE does
    // @stereotype collaborator incidental
    HANDLE SCI_METHOD Handle() const { return 0; }
    // A name with a lowercase letter stays, as the macro fastcall of old Linux kernels does
    // @stereotype collaborator incidental
    int fastcall Count() const { return 3; }
};

// A method defined outside its type leaves the macro of its return type out as well
// @stereotype set
void SCI_METHOD Lexer::Reset() { state = 0; }

// Each calling convention of Windows is a specifier of C++, which leaves the type
class Callbacks {
public:
    // @stereotype incidental
    int __stdcall Stdcall() const { return 1; }
    // @stereotype incidental
    int __cdecl Cdecl() const { return 1; }
    // @stereotype incidental
    int __fastcall Fastcall() const { return 1; }
    // @stereotype incidental
    int __thiscall Thiscall() const { return 1; }
    // @stereotype incidental
    int __vectorcall Vectorcall() const { return 1; }
    // @stereotype incidental
    int __clrcall Clrcall() const { return 1; }
    // @stereotype incidental
    int WINAPI Winapi() const { return 1; }
    // @stereotype incidental
    int WINAPIV Winapiv() const { return 1; }
    // @stereotype incidental
    int APIENTRY Apientry() const { return 1; }
    // @stereotype incidental
    int CALLBACK Callback() const { return 1; }
    // @stereotype incidental
    int PASCAL Pascal() const { return 1; }
    // @stereotype incidental
    int STDMETHODCALLTYPE Stdmethodcalltype() const { return 1; }
    // @stereotype incidental
    int STDAPICALLTYPE Stdapicalltype() const { return 1; }
    // The first name always stays, as LRESULT does
    // @stereotype collaborator incidental
    LRESULT CALLBACK WindowProc() const { return 0; }
};

// A free function returns a Boolean or a pointer to a primitive whatever macro follows the first name of its return type
// (Free-Function-Stereotypes wiki, predicate, factory, and literal)
// @stereotype predicate
bool CALLING_CONVENTION IsPositive(int value) { return value > 0; }
// @stereotype literal
int* CALLING_CONVENTION NewCounter() { return new int(0); }

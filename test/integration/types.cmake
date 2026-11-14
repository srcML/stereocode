# SPDX-License-Identifier: GPL-3.0-only
# @file types.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Integration checks of the kinds of types, with nested, anonymous, and typedef types, the local code of Python, Java enums without -e,
# C# properties, large classes, and the macros of C++ return types
# test/CMakeLists.txt registers each function check_<name> as the test integration/types/<name>

include(${CMAKE_CURRENT_LIST_DIR}/run_and_check.cmake)

# Fails unless the srcML output of the last run labels the given number of elements
function(expect_labeled_elements expected)
    file(READ "${WORK_DIR}/input.stereotypes.xml" output)
    string(REGEX MATCHALL "st:stereotype=\"" labels "${output}")
    list(LENGTH labels labelCount)
    if(NOT labelCount EQUAL expected)
        message(FATAL_ERROR "Expected ${expected} labeled elements, got ${labelCount}:\n${output}")
    endif()
endfunction()

# A nested named type is a type of its own, which the CSV report names with the types around it,
# while local and anonymous types and their methods get no stereotype
#
# The C++ file:
#   class Outer {
#       int x;
#   public:
#       int getX() const { return x; }
#       class Inner { int v; public: int get() const { return v; } };
#       struct { int a; int get() const { return a; } } anonymous;
#       void run() const { struct Local { int w; int get() const { return w; } }; }
#   };
#
# The Java file:
#   class Holder {
#       int x;
#       static class Node { int n; int get() { return n; } }
#       Runnable task = new Runnable() { public void run() { x = 1; } };
#       void walk() { class Local { int w; int get() { return w; } } }
#   }
function(check_nested_types)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit revision="1.0.0" language="C++" filename="outer.cpp" hash="da30e509e8a116b1a4556ec43eac9d47d03ed83d"><class>class <name>Outer</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
    <class>class <name>Inner</name> <block>{<private type="default"> <decl_stmt><decl><type><name>int</name></type> <name>v</name></decl>;</decl_stmt> </private><public>public: <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>v</name></expr>;</return> </block_content>}</block></function> </public>}</block>;</class>
    <struct>struct <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>a</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>a</name></expr>;</return> </block_content>}</block></function> </public>}</block> <decl><name>anonymous</name></decl>;</struct>
    <function><type><name>void</name></type> <name>run</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <struct>struct <name>Local</name> <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>w</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function> </public>}</block>;</struct> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit revision="1.0.0" language="Java" filename="Holder.java" hash="aad36f395fe6e97f28612eb89cc11884379addd1"><class>class <name>Holder</name> <block>{
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
    <class><specifier>static</specifier> class <name>Node</name> <block>{ <decl_stmt><decl><type><name>int</name></type> <name>n</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>n</name></expr>;</return> </block_content>}</block></function> }</block></class>
    <decl_stmt><decl><type><name>Runnable</name></type> <name>task</name> <init>= <expr><operator>new</operator> <class><super><name>Runnable</name></super><argument_list>()</argument_list> <block>{ <function><type><specifier>public</specifier> <name>void</name></type> <name>run</name><parameter_list>()</parameter_list> <block>{<block_content> <expr_stmt><expr><name>x</name> <operator>=</operator> <literal type="number">1</literal></expr>;</expr_stmt> </block_content>}</block></function> }</block></class></expr></init></decl>;</decl_stmt>
    <function><type><name>void</name></type> <name>walk</name><parameter_list>()</parameter_list> <block>{<block_content> <class>class <name>Local</name> <block>{ <decl_stmt><decl><type><name>int</name></type> <name>w</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function> }</block></class> </block_content>}</block></function>
}</block></class>
</unit>

</unit>
]=])

    set(EXPECTED_CSV [=[
type_name,type_stereotype,function_name,function_stereotype
"Outer","data-provider lazy-class small-class","getX","get"
"Outer","data-provider lazy-class small-class","run","incidental"
"Outer::Inner","data-provider data-class small-class","get","get"
"Holder","degenerate small-class","walk","incidental"
"Holder.Node","data-provider data-class small-class","get","get"
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -t -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "${EXPECTED_CSV}")

    # Outer, getX, Inner, its get, and run in C++, and Holder, Node, its get, and walk in Java
    expect_labeled_elements(9)
endfunction()

# A Python class in a class is a type of its own, while a class or a function in a function is local code, which gets no stereotype
#
# The Python file:
#   class Outer:
#       def __init__(self):
#           self.x = 0
#
#       def get_x(self):
#           return self.x
#
#       class Inner:
#           def __init__(self):
#               self.v = 0
#
#           def get(self):
#               return self.v
#
#       def run(self):
#           class Local:
#               def __init__(self):
#                   self.w = 0
#
#               def get(self):
#                   return self.w
#
#           def helper():
#               return 1
#           return helper
#
#
#   def build():
#       def inner():
#           return 2
#       return inner
function(check_python_local_code)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="Python" filename="outer.py"><class>class <name>Outer</name><block>:<block_content>
    <function>def <name>__init__</name><parameter_list>(<parameter><name>self</name></parameter>)</parameter_list><block>:<block_content>
        <expr_stmt><expr><name><name>self</name><operator>.</operator><name>x</name></name> <operator>=</operator> <literal type="number">0</literal></expr></expr_stmt>
</block_content></block></function>
    <function>def <name>get_x</name><parameter_list>(<parameter><name>self</name></parameter>)</parameter_list><block>:<block_content>
        <return>return <expr><name><name>self</name><operator>.</operator><name>x</name></name></expr></return>
</block_content></block></function>
    <class>class <name>Inner</name><block>:<block_content>
        <function>def <name>__init__</name><parameter_list>(<parameter><name>self</name></parameter>)</parameter_list><block>:<block_content>
            <expr_stmt><expr><name><name>self</name><operator>.</operator><name>v</name></name> <operator>=</operator> <literal type="number">0</literal></expr></expr_stmt>
</block_content></block></function>
        <function>def <name>get</name><parameter_list>(<parameter><name>self</name></parameter>)</parameter_list><block>:<block_content>
            <return>return <expr><name><name>self</name><operator>.</operator><name>v</name></name></expr></return>
</block_content></block></function></block_content></block></class>
    <function>def <name>run</name><parameter_list>(<parameter><name>self</name></parameter>)</parameter_list><block>:<block_content>
        <class>class <name>Local</name><block>:<block_content>
            <function>def <name>__init__</name><parameter_list>(<parameter><name>self</name></parameter>)</parameter_list><block>:<block_content>
                <expr_stmt><expr><name><name>self</name><operator>.</operator><name>w</name></name> <operator>=</operator> <literal type="number">0</literal></expr></expr_stmt>
</block_content></block></function>
            <function>def <name>get</name><parameter_list>(<parameter><name>self</name></parameter>)</parameter_list><block>:<block_content>
                <return>return <expr><name><name>self</name><operator>.</operator><name>w</name></name></expr></return>
</block_content></block></function></block_content></block></class>
        <function>def <name>helper</name><parameter_list>()</parameter_list><block>:<block_content>
            <return>return <expr><literal type="number">1</literal></expr></return>
</block_content></block></function>        <return>return <expr><name>helper</name></expr></return>
</block_content></block></function></block_content></block></class>

<function>def <name>build</name><parameter_list>()</parameter_list><block>:<block_content>
    <function>def <name>inner</name><parameter_list>()</parameter_list><block>:<block_content>
        <return>return <expr><literal type="number">2</literal></expr></return>
</block_content></block></function>    <return>return <expr><name>inner</name></expr></return>
</block_content></block></function></unit>
]=])

    set(HEADER "type_name,type_stereotype,function_name,function_stereotype\n")
    set(TYPE_ROWS [=[
"Outer","data-provider lazy-class small-class","__init__","constructor"
"Outer","data-provider lazy-class small-class","get_x","get"
"Outer","data-provider lazy-class small-class","run","incidental"
"Outer.Inner","data-provider data-class small-class","__init__","constructor"
"Outer.Inner","data-provider data-class small-class","get","get"
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "${HEADER}${TYPE_ROWS}")
    expect_file_content("${WORK_DIR}/input.free_functions.stereotypes.csv" "${HEADER}\"N/A\",\"N/A\",\"build\",\"literal\"\n")

    # Outer, its three methods, Inner, its two methods, and build
    expect_labeled_elements(8)
endfunction()

# Only -e gives a Java enum and its methods stereotypes and rows in the reports, while the other types take in the enum
# either way: its inner class counts its fields as data members, and a call on a field of the enum counts by the called method
#
# The Java file:
#   enum Level {
#       LOW;
#       private int weight;
#       int weight() { return weight; }
#       class Probe {
#           int weight() { return weight; }
#           void raise() { weight++; }
#       }
#   }
#
#   class Gauge {
#       private Level level;
#       int read() { return level.weight(); }
#   }
function(check_unlabeled_enums)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="Java" filename="Level.java"><enum>enum <name>Level</name> <block>{
    <decl><name>LOW</name></decl>;

    <decl_stmt><decl><type><specifier>private</specifier> <name>int</name></type> <name>weight</name></decl>;</decl_stmt>

    <function><type><name>int</name></type> <name>weight</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>weight</name></expr>;</return> </block_content>}</block></function>

    <class>class <name>Probe</name> <block>{
        <function><type><name>int</name></type> <name>weight</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>weight</name></expr>;</return> </block_content>}</block></function>
        <function><type><name>void</name></type> <name>raise</name><parameter_list>()</parameter_list> <block>{<block_content> <expr_stmt><expr><name>weight</name><operator>++</operator></expr>;</expr_stmt> </block_content>}</block></function>
    }</block></class>
}</block></enum>

<class>class <name>Gauge</name> <block>{
    <decl_stmt><decl><type><specifier>private</specifier> <name>Level</name></type> <name>level</name></decl>;</decl_stmt>

    <function><type><name>int</name></type> <name>read</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><call><name><name>level</name><operator>.</operator><name>weight</name></name><argument_list>()</argument_list></call></expr>;</return> </block_content>}</block></function>
}</block></class>
</unit>
]=])

    set(HEADER "type_name,type_stereotype,function_name,function_stereotype\n")
    set(ENUM_ROWS [=[
"Level","data-provider data-enum small-enum","weight","get"
]=])
    # weight of Probe returns the field of the enum, raise changes it, and read calls an accessor of the enum on its field
    set(OTHER_ROWS [=[
"Level.Probe","data-class small-class","weight","get"
"Level.Probe","data-class small-class","raise","set"
"Gauge","data-provider boundary small-class","read","property collaborator"
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    # Probe, its two methods, Gauge, and read, also with every other option that adds a kind of type or function
    run_stereocode(input.xml -r -x -f -i -t -u)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "${HEADER}${OTHER_ROWS}")
    expect_labeled_elements(5)
    file(READ "${WORK_DIR}/input.stereotypes.txt" report)
    if(report MATCHES "\nLevel ")
        message(FATAL_ERROR "The text report names the enum without -e:\n${report}")
    endif()

    # With -e, the enum and its method weight as well
    run_stereocode(input.xml -r -x -f -i -t -u -e)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "${HEADER}${ENUM_ROWS}${OTHER_ROWS}")
    expect_labeled_elements(7)
    file(READ "${WORK_DIR}/input.stereotypes.txt" report)
    if(NOT report MATCHES "\nLevel +data-provider data-enum small-enum\n")
        message(FATAL_ERROR "The text report lacks the row of the enum with -e:\n${report}")
    endif()
endfunction()

# The object of a class without a name at namespace scope is a global, while the class and its method get no stereotype
#
# The C++ file:
#   class { int x; public: int get() const { return x; } } g_anon;
#   void setAnon(int v) { g_anon.x = v; }
function(check_anonymous_class_objects)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit revision="1.0.0" language="C++" filename="anon.cpp" hash="76290d4dbb1b628628dd20ef704da461ef9267d4"><class>class <block>{<private type="default"> <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt> </private><public>public: <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function> </public>}</block> <decl><name>g_anon</name></decl>;</class>
<function><type><name>void</name></type> <name>setAnon</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><name><name>g_anon</name><operator>.</operator><name>x</name></name> <operator>=</operator> <name>v</name></expr>;</expr_stmt> </block_content>}</block></function>
</unit>

</unit>
]=])

    set(EXPECTED_FREE_FUNCTIONS_CSV [=[
type_name,type_stereotype,function_name,function_stereotype
"N/A","N/A","setAnon","global-command"
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.free_functions.stereotypes.csv" "${EXPECTED_FREE_FUNCTIONS_CSV}")

    # Only setAnon carries a stereotype
    expect_labeled_elements(1)
endfunction()

# A C++ class, struct, or union without a name takes the first name of its typedef that names the type itself, while a type that
# a typedef names only through a pointer stays anonymous and gets no stereotype
# A type nested in a type of a typedef takes the name of the typedef as its qualifier, and the members of a union of a typedef are no globals
#
# The C++ file:
#   typedef struct { int x; int getX() const { return x; } } *PointPtr, Point;
#   typedef struct { int h; int getH() const { return h; } } *Handle;
#   typedef union { int i; float f; } Number;
#   struct Holder { typedef struct { int d; int getD() const { return d; } } Level; };
#   typedef struct { struct Leaf { int w; int getW() const { return w; } } leaf; } Tree;
#   void clear() { i = 0; }
function(check_typedef_types)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="typedefs.cpp"><typedef>typedef <type><struct>struct <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function> </public>}</block></struct></type> *<name>PointPtr</name><operator>,</operator> <name>Point</name>;</typedef>
<typedef>typedef <type><struct>struct <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>h</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>getH</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>h</name></expr>;</return> </block_content>}</block></function> </public>}</block></struct></type> *<name>Handle</name>;</typedef>
<typedef>typedef <type><union>union <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>i</name></decl>;</decl_stmt> <decl_stmt><decl><type><name>float</name></type> <name>f</name></decl>;</decl_stmt> </public>}</block></union></type> <name>Number</name>;</typedef>
<struct>struct <name>Holder</name> <block>{<public type="default"> <typedef>typedef <type><struct>struct <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>d</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>getD</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>d</name></expr>;</return> </block_content>}</block></function> </public>}</block></struct></type> <name>Level</name>;</typedef> </public>}</block>;</struct>
<typedef>typedef <type><struct>struct <block>{<public type="default"> <struct>struct <name>Leaf</name> <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>w</name></decl>;</decl_stmt> <function><type><name>int</name></type> <name>getW</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function> </public>}</block> <decl><name>leaf</name></decl>;</struct> </public>}</block></struct></type> <name>Tree</name>;</typedef>
<function><type><name>void</name></type> <name>clear</name><parameter_list>()</parameter_list> <block>{<block_content> <expr_stmt><expr><name>i</name> <operator>=</operator> <literal type="number">0</literal></expr>;</expr_stmt> </block_content>}</block></function>
</unit>
]=])

    set(EXPECTED_CSV [=[
type_name,type_stereotype,function_name,function_stereotype
"Point","data-provider data-struct small-struct","getX","get"
"Number","empty","N/A","N/A"
"Holder","empty","N/A","N/A"
"Holder::Level","data-provider data-struct small-struct","getD","get"
"Tree","empty","N/A","N/A"
"Tree::Leaf","data-provider data-struct small-struct","getW","get"
]=])

    # The free function writes the name of a member of Number, which is no global
    set(EXPECTED_FREE_FUNCTIONS_CSV [=[
type_name,type_stereotype,function_name,function_stereotype
"N/A","N/A","clear","literal"
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -t -u -f -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "${EXPECTED_CSV}")
    expect_file_content("${WORK_DIR}/input.free_functions.stereotypes.csv" "${EXPECTED_FREE_FUNCTIONS_CSV}")

    # The six types, the three methods, and the free function, while the struct of Handle and its method stay unlabeled
    expect_labeled_elements(10)
endfunction()

# The get accessor of a C# property returns the type of the property, which a separate query of the property finds
# A STEREOCODE_SANITIZE build also detects memory that the query never frees
#
# The C# file:
#   class Account {
#       private int balance;
#       public int Balance {
#           get { return balance; }
#           set { balance = value; }
#       }
#   }
function(check_csharp_property)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C#" filename="property.cs"><class>class <name>Account</name> <block>{
    <decl_stmt><decl><type><specifier>private</specifier> <name>int</name></type> <name>balance</name></decl>;</decl_stmt>
    <property><type><specifier>public</specifier> <name>int</name></type> <name>Balance</name> <block>{
        <function><name>get</name> <block>{<block_content> <return>return <expr><name>balance</name></expr>;</return> </block_content>}</block></function>
        <function><name>set</name> <block>{<block_content> <expr_stmt><expr><name>balance</name> <operator>=</operator> <name>value</name></expr>;</expr_stmt> </block_content>}</block></function>
    }</block></property>
}</block></class>
</unit>
]=])

    set(EXPECTED_OUTPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C#" filename="property.cs"><class st:stereotype="data-class small-class">class <name>Account</name> <block>{
    <decl_stmt><decl><type><specifier>private</specifier> <name>int</name></type> <name>balance</name></decl>;</decl_stmt>
    <property><type><specifier>public</specifier> <name>int</name></type> <name>Balance</name> <block>{
        <function st:stereotype="get"><name>get</name> <block>{<block_content> <return>return <expr><name>balance</name></expr>;</return> </block_content>}</block></function>
        <function st:stereotype="set"><name>set</name> <block>{<block_content> <expr_stmt><expr><name>balance</name> <operator>=</operator> <name>value</name></expr>;</expr_stmt> </block_content>}</block></function>
    }</block></property>
}</block></class>
</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${EXPECTED_OUTPUT}")
endfunction()

# Fails unless the verbose output of the last run names the threshold
function(expect_threshold description)
    if(NOT RUN_ERROR MATCHES "---Large-Class Threshold---\n${description}\n")
        message(FATAL_ERROR "The verbose output does not name the threshold '${description}':\n${RUN_ERROR}")
    endif()
endfunction()

# Fails unless the type columns of Mix in the CSV report, which -b starts with the file, give the stereotypes
function(expect_mix stereotypes)
    file(READ "${WORK_DIR}/input.stereotypes.csv" report)
    if(NOT report MATCHES "\n(\"mix.cpp\",)?\"Mix\",\"${stereotypes}\",")
        message(FATAL_ERROR "Mix is not ${stereotypes}:\n${report}")
    endif()
endfunction()

# Fails unless the classes of the CSV report of the last run have the stereotypes of the run without -t
function(expect_class_rows)
    file(READ "${WORK_DIR}/input.stereotypes.csv" report)
    string(REGEX MATCHALL "\"(Mix|First|Second)\",\"[^\"]*\"" rows "${report}")
    if(NOT rows STREQUAL CLASS_ROWS)
        message(FATAL_ERROR "The classes have other stereotypes than in the run without -t:\n${rows}\nExpected:\n${CLASS_ROWS}")
    endif()
endfunction()

# A large type has more methods than the mean plus the standard deviation of the methods per class over the classes
# of the run, whatever kinds of types -t, -u, -i, and -e add, unless -l sets the threshold. A run without a class takes
# the labeled types instead. The verbose output names the threshold of the run and its basis
#
# The archive holds these files:
#   mix.cpp      class Part;
#                class Mix { int m_a; public: int a() const { return m_a; } void setA(int v) { m_a = v; }
#                            void pass(Part* part) { part->use(m_a); } Part* make() { return new Part(); } };
#                class First {};
#                class Second {};
#                struct Wide { int w; int w1() { return w; } ... int w5() { return w; } };
#   Marker.java  interface Marker {}
#
# Mix has a get, a set, a controller, and a factory, so it is large once it has more methods than the threshold
# The archive structs.xml holds only Wide and struct Bare {};
function(check_large_class_threshold)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit revision="1.0.0" language="C++" filename="mix.cpp"><class_decl>class <name>Part</name>;</class_decl>
<class>class <name>Mix</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>m_a</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>a</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>m_a</name></expr>;</return> </block_content>}</block></function>
    <function><type><name>void</name></type> <name>setA</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><name>m_a</name> <operator>=</operator> <name>v</name></expr>;</expr_stmt> </block_content>}</block></function>
    <function><type><name>void</name></type> <name>pass</name><parameter_list>(<parameter><decl><type><name>Part</name><modifier>*</modifier></type> <name>part</name></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><call><name><name>part</name><operator>-&gt;</operator><name>use</name></name><argument_list>(<argument><expr><name>m_a</name></expr></argument>)</argument_list></call></expr>;</expr_stmt> </block_content>}</block></function>
    <function><type><name>Part</name><modifier>*</modifier></type> <name>make</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><operator>new</operator> <call><name>Part</name><argument_list>()</argument_list></call></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
<class>class <name>First</name> <block>{<private type="default"/>}</block>;</class>
<class>class <name>Second</name> <block>{<private type="default"/>}</block>;</class>
@WIDE@
</unit>

<unit revision="1.0.0" language="Java" filename="Marker.java"><interface>interface <name>Marker</name> <block>{}</block></interface>
</unit>

</unit>
]=])

    set(WIDE [=[<struct>struct <name>Wide</name> <block>{<public type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>w</name></decl>;</decl_stmt>
    <function><type><name>int</name></type> <name>w1</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function>
    <function><type><name>int</name></type> <name>w2</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function>
    <function><type><name>int</name></type> <name>w3</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function>
    <function><type><name>int</name></type> <name>w4</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function>
    <function><type><name>int</name></type> <name>w5</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</struct>]=])
    string(CONFIGURE "${INPUT}" INPUT @ONLY)

    set(STRUCTS [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="structs.cpp">@WIDE@
<struct>struct <name>Bare</name> <block>{<public type="default"/>}</block>;</struct>
</unit>
]=])
    string(CONFIGURE "${STRUCTS}" STRUCTS @ONLY)

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")
    file(WRITE "${WORK_DIR}/structs.xml" "${STRUCTS}")

    # The classes Mix, First, and Second have 4, 0, and 0 methods, while the struct and the interface do not count
    set(CLASS_THRESHOLD "More than 3 methods, the mean 1\\.33 plus the standard deviation 1\\.89 of the methods per class over the 3 classes of the run, rounded down")
    run_stereocode(input.xml -d -b)
    expect_exit(0)
    expect_threshold("${CLASS_THRESHOLD}")
    expect_mix("large-class")
    file(READ "${WORK_DIR}/input.stereotypes.csv" report)
    string(REGEX MATCHALL "\"(Mix|First|Second)\",\"[^\"]*\"" CLASS_ROWS "${report}")

    # With -t, the struct gets its stereotypes, and the classes keep theirs
    run_stereocode(input.xml -d -b -t)
    expect_exit(0)
    expect_threshold("${CLASS_THRESHOLD}")
    expect_class_rows()

    # With -i, the interface gets its stereotypes, and the classes keep theirs
    run_stereocode(input.xml -d -b -i -t)
    expect_exit(0)
    expect_threshold("${CLASS_THRESHOLD}")
    expect_class_rows()

    # A run without a class takes the labeled types, here Wide and Bare with 5 and 0 methods
    run_stereocode(structs.xml -d -b -t)
    expect_exit(0)
    expect_threshold("More than 5 methods, the mean 2\\.50 plus the standard deviation 2\\.50 of the methods per type over the 2 labeled types of the run, which has no class, rounded down")

    # -l sets the threshold
    run_stereocode(input.xml -d -b -l 4)
    expect_exit(0)
    expect_threshold("More than 4 methods, as given with -l")
    expect_mix("unclassified")

    # Only the verbose output names the threshold
    run_stereocode(input.xml -d -r)
    expect_exit(0)
    if(RUN_ERROR MATCHES "Large-Class Threshold")
        message(FATAL_ERROR "A run without -b names the threshold:\n${RUN_ERROR}")
    endif()
    expect_mix("large-class")
endfunction()

# A name after the first of a C++ return type in capital letters, digits, and underscores is a macro that leaves the type, as SCI_METHOD is,
# unless a type or an alias of the archive has the name, whatever the options: an alias such as POINT, an enum, a typedef of an enum
# without a name, a struct without -t, a union without -u, or a class that the archive only declares
# A specifiers file that removes the export macro LEXER_API leaves such a name as the whole return type, which makes the method a collaborator
#
# The C++ file:
#   struct Point { int x; int y; };
#   typedef Point POINT;
#   enum MODE_T { LOW, HIGH };
#   typedef enum { NEAR, FAR } RANGE_T;
#   struct SIZE_T { int width; };
#   union VALUE_U { int i; float f; };
#   class HANDLE_C;
#
#   class Lexer {
#       int count;
#   public:
#       LEXER_API POINT Origin() const { return {}; }
#       LEXER_API int SCI_METHOD Count() const { return count; }
#       LEXER_API MODE_T Mode() const { return LOW; }
#       LEXER_API RANGE_T Range() const { return NEAR; }
#       LEXER_API SIZE_T Size() const { return {}; }
#       LEXER_API VALUE_U Value() const { return {}; }
#       LEXER_API HANDLE_C* Handle() const { return 0; }
#   };
function(check_return_type_macros)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="lexer.cpp"><struct>struct <name>Point</name> <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt> <decl_stmt><decl><type><name>int</name></type> <name>y</name></decl>;</decl_stmt> </public>}</block>;</struct>
<typedef>typedef <type><name>Point</name></type> <name>POINT</name>;</typedef>
<enum>enum <name>MODE_T</name> <block>{ <decl><name>LOW</name></decl>, <decl><name>HIGH</name></decl> }</block>;</enum>
<typedef>typedef <type><enum>enum <block>{ <decl><name>NEAR</name></decl>, <decl><name>FAR</name></decl> }</block></enum></type> <name>RANGE_T</name>;</typedef>
<struct>struct <name>SIZE_T</name> <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>width</name></decl>;</decl_stmt> </public>}</block>;</struct>
<union>union <name>VALUE_U</name> <block>{<public type="default"> <decl_stmt><decl><type><name>int</name></type> <name>i</name></decl>;</decl_stmt> <decl_stmt><decl><type><name>float</name></type> <name>f</name></decl>;</decl_stmt> </public>}</block>;</union>
<class_decl>class <name>HANDLE_C</name>;</class_decl>

<class>class <name>Lexer</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>count</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>LEXER_API</name> <name>POINT</name></type> <name>Origin</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><block>{}</block></expr>;</return> </block_content>}</block></function>
    <function><type><name>LEXER_API</name> <name>int</name> <name>SCI_METHOD</name></type> <name>Count</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>count</name></expr>;</return> </block_content>}</block></function>
    <function><type><name>LEXER_API</name> <name>MODE_T</name></type> <name>Mode</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>LOW</name></expr>;</return> </block_content>}</block></function>
    <function><type><name>LEXER_API</name> <name>RANGE_T</name></type> <name>Range</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>NEAR</name></expr>;</return> </block_content>}</block></function>
    <function><type><name>LEXER_API</name> <name>SIZE_T</name></type> <name>Size</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><block>{}</block></expr>;</return> </block_content>}</block></function>
    <function><type><name>LEXER_API</name> <name>VALUE_U</name></type> <name>Value</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><block>{}</block></expr>;</return> </block_content>}</block></function>
    <function><type><name>LEXER_API</name> <name>HANDLE_C</name><modifier>*</modifier></type> <name>Handle</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><literal type="number">0</literal></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>
]=])

    set(EXPECTED_CSV [=[
type_name,type_stereotype,function_name,function_stereotype
"Lexer","data-provider boundary lazy-class degenerate","Origin","collaborator incidental"
"Lexer","data-provider boundary lazy-class degenerate","Count","get"
"Lexer","data-provider boundary lazy-class degenerate","Mode","collaborator incidental"
"Lexer","data-provider boundary lazy-class degenerate","Range","collaborator incidental"
"Lexer","data-provider boundary lazy-class degenerate","Size","collaborator incidental"
"Lexer","data-provider boundary lazy-class degenerate","Value","collaborator incidental"
"Lexer","data-provider boundary lazy-class degenerate","Handle","collaborator incidental"
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")
    file(WRITE "${WORK_DIR}/specifiers.txt" "LEXER_API\n")

    run_stereocode(input.xml -s specifiers.txt -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "${EXPECTED_CSV}")
endfunction()

run_check()

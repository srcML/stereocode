# SPDX-License-Identifier: GPL-3.0-only
# @file types.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Integration checks of the kinds of types, with nested, anonymous, and typedef types, C# properties, and large classes
# test/CMakeLists.txt registers each function check_<name> as the test integration/types/<name>

include(${CMAKE_CURRENT_LIST_DIR}/run_and_check.cmake)

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
    file(READ "${WORK_DIR}/input.stereotypes.xml" output)
    string(REGEX MATCHALL "st:stereotype=\"" labels "${output}")
    list(LENGTH labels labelCount)
    if(NOT labelCount EQUAL 9)
        message(FATAL_ERROR "Expected 9 labeled elements, got ${labelCount}:\n${output}")
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
    file(READ "${WORK_DIR}/input.stereotypes.xml" output)
    string(REGEX MATCHALL "st:stereotype=\"" labels "${output}")
    list(LENGTH labels labelCount)
    if(NOT labelCount EQUAL 1)
        message(FATAL_ERROR "Expected 1 labeled element, got ${labelCount}:\n${output}")
    endif()
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
    file(READ "${WORK_DIR}/input.stereotypes.xml" output)
    string(REGEX MATCHALL "st:stereotype=\"" labels "${output}")
    list(LENGTH labels labelCount)
    if(NOT labelCount EQUAL 10)
        message(FATAL_ERROR "Expected 10 labeled elements, got ${labelCount}:\n${output}")
    endif()
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

run_check()

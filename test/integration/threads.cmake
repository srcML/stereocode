# SPDX-License-Identifier: GPL-3.0-only
# @file threads.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Integration checks of the worker threads
# test/CMakeLists.txt registers each function check_<name> as the test integration/threads/<name>

include(${CMAKE_CURRENT_LIST_DIR}/run_and_check.cmake)

# The srcML output and the CSV reports are the same for one, two, and three worker threads, and -j rejects a count outside 1 to 256
# The first unit, big.cpp, takes longer than the others, so the workers finish the other units first
# The labels of Derived::setY in c.cpp depend on the type Derived of b.cpp, which inherits from the type Base of a.cpp,
# and the labels of Holder::read in d.cpp depend on the labels of the method getX of a.cpp that it calls on its data member
#
# big.cpp holds 300 copies of this function:
#   int twice(int v) { return 2 * v; }
#
# a.cpp, b.cpp, c.cpp, d.cpp, and Counter.java:
#   class Base {                       class Derived : public Base {        void Derived::setY(int v) { y = v; }
#   protected:                             int y;
#       int x;                         public:                              int twice(int v) { return 2 * v; }
#   public:                                void setY(int v);
#       int getX() const { return x; }     int sum() const { return x + y; }
#   };                                 };
#
#   class Holder {
#       Base m_base;
#   public:
#       int read() { return m_base.getX() + 1; }
#   };
#
#   class Counter {
#       private int count;
#       public int getCount() { return count; }
#       public void increment() { count++; }
#   }
function(check_thread_counts)
    set(ARCHIVE_START [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

]=])

    set(FUNCTION [=[
<function><type><name>int</name></type> <name>twice</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">2</literal> <operator>*</operator> <name>v</name></expr>;</return> </block_content>}</block></function>
]=])

    set(LABELED_FUNCTION [=[
<function st:stereotype="property"><type><name>int</name></type> <name>twice</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">2</literal> <operator>*</operator> <name>v</name></expr>;</return> </block_content>}</block></function>
]=])

    set(UNITS [=[
<unit revision="1.0.0" language="C++" filename="a.cpp" hash="aa2d31c745e1b409d5ae03d1bcf266bfc4305e24"><class>class <name>Base</name> <block>{<private type="default">
</private><protected>protected:
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</protected><public>public:
    <function><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit revision="1.0.0" language="C++" filename="b.cpp" hash="c65ba1b28a2070fde95790650f6caf02d0b019fd"><class>class <name>Derived</name> <super_list>: <super><specifier>public</specifier> <name>Base</name></super></super_list> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>y</name></decl>;</decl_stmt>
</private><public>public:
    <function_decl><type><name>void</name></type> <name>setY</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list>;</function_decl>
    <function><type><name>int</name></type> <name>sum</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name> <operator>+</operator> <name>y</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit revision="1.0.0" language="C++" filename="c.cpp" hash="c4aa719d4d10080de136f8482109e9a24ea680b5"><function><type><name>void</name></type> <name><name>Derived</name><operator>::</operator><name>setY</name></name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><name>y</name> <operator>=</operator> <name>v</name></expr>;</expr_stmt> </block_content>}</block></function>

<function><type><name>int</name></type> <name>twice</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">2</literal> <operator>*</operator> <name>v</name></expr>;</return> </block_content>}</block></function>
</unit>

<unit revision="1.0.0" language="C++" filename="d.cpp"><class>class <name>Holder</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>Base</name></type> <name>m_base</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>read</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><call><name><name>m_base</name><operator>.</operator><name>getX</name></name><argument_list>()</argument_list></call> <operator>+</operator> <literal type="number">1</literal></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit revision="1.0.0" language="Java" filename="Counter.java" hash="687fd9e8b64a96f6169e08c5210bfe24a76776bc"><class>class <name>Counter</name> <block>{
    <decl_stmt><decl><type><specifier>private</specifier> <name>int</name></type> <name>count</name></decl>;</decl_stmt>
    <function><type><specifier>public</specifier> <name>int</name></type> <name>getCount</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>count</name></expr>;</return> </block_content>}</block></function>
    <function><type><specifier>public</specifier> <name>void</name></type> <name>increment</name><parameter_list>()</parameter_list> <block>{<block_content> <expr_stmt><expr><name>count</name><operator>++</operator></expr>;</expr_stmt> </block_content>}</block></function>
}</block></class>
</unit>

</unit>
]=])

    set(LABELED_UNITS [=[
<unit xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="a.cpp" hash="aa2d31c745e1b409d5ae03d1bcf266bfc4305e24"><class st:stereotype="data-provider data-class small-class">class <name>Base</name> <block>{<private type="default">
</private><protected>protected:
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</protected><public>public:
    <function st:stereotype="get"><type><name>int</name></type> <name>getX</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="b.cpp" hash="c65ba1b28a2070fde95790650f6caf02d0b019fd"><class st:stereotype="small-class">class <name>Derived</name> <super_list>: <super><specifier>public</specifier> <name>Base</name></super></super_list> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>y</name></decl>;</decl_stmt>
</private><public>public:
    <function_decl><type><name>void</name></type> <name>setY</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list>;</function_decl>
    <function st:stereotype="property"><type><name>int</name></type> <name>sum</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>x</name> <operator>+</operator> <name>y</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="c.cpp" hash="c4aa719d4d10080de136f8482109e9a24ea680b5"><function st:stereotype="set"><type><name>void</name></type> <name><name>Derived</name><operator>::</operator><name>setY</name></name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><name>y</name> <operator>=</operator> <name>v</name></expr>;</expr_stmt> </block_content>}</block></function>

<function st:stereotype="property"><type><name>int</name></type> <name>twice</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">2</literal> <operator>*</operator> <name>v</name></expr>;</return> </block_content>}</block></function>
</unit>

<unit xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="C++" filename="d.cpp"><class st:stereotype="data-provider boundary small-class">class <name>Holder</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>Base</name></type> <name>m_base</name></decl>;</decl_stmt>
</private><public>public:
    <function st:stereotype="property collaborator"><type><name>int</name></type> <name>read</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><call><name><name>m_base</name><operator>.</operator><name>getX</name></name><argument_list>()</argument_list></call> <operator>+</operator> <literal type="number">1</literal></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit xmlns:st="http://www.srcML.org/srcML/stereotype" revision="1.0.0" language="Java" filename="Counter.java" hash="687fd9e8b64a96f6169e08c5210bfe24a76776bc"><class st:stereotype="data-class small-class">class <name>Counter</name> <block>{
    <decl_stmt><decl><type><specifier>private</specifier> <name>int</name></type> <name>count</name></decl>;</decl_stmt>
    <function st:stereotype="get"><type><specifier>public</specifier> <name>int</name></type> <name>getCount</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>count</name></expr>;</return> </block_content>}</block></function>
    <function st:stereotype="set"><type><specifier>public</specifier> <name>void</name></type> <name>increment</name><parameter_list>()</parameter_list> <block>{<block_content> <expr_stmt><expr><name>count</name><operator>++</operator></expr>;</expr_stmt> </block_content>}</block></function>
}</block></class>
</unit>

</unit>
]=])

    string(REPEAT "${FUNCTION}" 300 FUNCTIONS)
    string(REPEAT "${LABELED_FUNCTION}" 300 LABELED_FUNCTIONS)
    set(INPUT "${ARCHIVE_START}<unit revision=\"1.0.0\" language=\"C++\" filename=\"big.cpp\">${FUNCTIONS}</unit>\n\n${UNITS}")
    set(EXPECTED_OUTPUT "${ARCHIVE_START}<unit xmlns:st=\"http://www.srcML.org/srcML/stereotype\" revision=\"1.0.0\" language=\"C++\" filename=\"big.cpp\">${LABELED_FUNCTIONS}</unit>\n\n${LABELED_UNITS}")

    set(EXPECTED_TYPE_REPORT [=[
type_name,type_stereotype,function_name,function_stereotype
"Base","data-provider data-class small-class","getX","get"
"Derived","small-class","sum","property"
"Derived","small-class","setY","set"
"Holder","data-provider boundary small-class","read","property collaborator"
"Counter","data-class small-class","getCount","get"
"Counter","data-class small-class","increment","set"
]=])

    string(REPEAT "\"N/A\",\"N/A\",\"twice\",\"property\"\n" 301 FREE_FUNCTION_ROWS)
    set(EXPECTED_FREE_FUNCTION_REPORT "type_name,type_stereotype,function_name,function_stereotype\n${FREE_FUNCTION_ROWS}")

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    foreach(threads 1 2 3)
        run_stereocode(input.xml -f -r -j ${threads} -o output.${threads}.xml)
        expect_exit(0)
        expect_file_content("${WORK_DIR}/output.${threads}.xml" "${EXPECTED_OUTPUT}")
        expect_file_content("${WORK_DIR}/output.${threads}.stereotypes.csv" "${EXPECTED_TYPE_REPORT}")
        expect_file_content("${WORK_DIR}/output.${threads}.free_functions.stereotypes.csv" "${EXPECTED_FREE_FUNCTION_REPORT}")
    endforeach()

    expect_rejected_value(-j 0 "0 is not from 1 to 256")
    expect_rejected_value(-j 257 "257 is not from 1 to 256")
    expect_rejected_value(-j abc "abc is not a whole number")
endfunction()

# std::thread::hardware_concurrency() may return 0, and the run then uses one worker thread
# The library PRELOAD, preloaded through LD_PRELOAD, reports zero processors to glibc
function(check_zero_processors)
    file(WRITE "${WORK_DIR}/input.xml" "${SAMPLE_INPUT}")

    set(ENV{LD_PRELOAD} "${PRELOAD}")
    # AddressSanitizer otherwise requires its runtime to come first among the preloaded libraries
    set(ENV{ASAN_OPTIONS} "$ENV{ASAN_OPTIONS}:verify_asan_link_order=0")

    run_stereocode(input.xml -f)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${SAMPLE_OUTPUT}")
endfunction()

run_check()

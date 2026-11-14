# SPDX-License-Identifier: GPL-3.0-only
# @file reports.cmake
#
# @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
#
# This file is part of the Stereocode application.
#
# Integration checks of the CSV and text reports
# test/CMakeLists.txt registers each function check_<name> as the test integration/reports/<name>

include(${CMAKE_CURRENT_LIST_DIR}/run_and_check.cmake)

# The CSV reports of types and of free functions (-f -r)
# A STEREOCODE_SANITIZE build also detects undefined behavior while the reports are written
function(check_free_function_csv)
    file(WRITE "${WORK_DIR}/input.xml" "${SAMPLE_INPUT}")

    run_stereocode(input.xml -f -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${SAMPLE_OUTPUT}")
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "${SAMPLE_CSV}")
    expect_file_content("${WORK_DIR}/input.free_functions.stereotypes.csv" "${SAMPLE_FREE_FUNCTIONS_CSV}")
endfunction()

# With -d, the run writes the CSV reports and no srcML output archive
# A STEREOCODE_SANITIZE build also detects memory that the run never frees
function(check_csv_without_srcml_output)
    file(WRITE "${WORK_DIR}/input.xml" "${SAMPLE_INPUT}")

    run_stereocode(input.xml -d -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "${SAMPLE_CSV}")
    expect_missing("${WORK_DIR}/input.free_functions.stereotypes.csv")
    expect_missing("${WORK_DIR}/input.stereotypes.xml")
    expect_missing("${WORK_DIR}/input.stereotypes.xml.tmp")
endfunction()

# The CSV reports take the name of the srcML output archive and sit beside it, with or without -d
# A report that cannot be created stops the run with an error that names the report, and the run writes no output
# A report that cannot replace its file stops the run before the archive replaces its file, which may be the input
function(check_csv_beside_output)
    file(MAKE_DIRECTORY "${WORK_DIR}/input" "${WORK_DIR}/results")
    file(WRITE "${WORK_DIR}/input/sample.xml" "${SAMPLE_INPUT}")

    run_stereocode(input/sample.xml -f -r -o results/sample.out.xml)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/results/sample.out.xml" "${SAMPLE_OUTPUT}")
    expect_file_content("${WORK_DIR}/results/sample.out.stereotypes.csv" "${SAMPLE_CSV}")
    expect_file_content("${WORK_DIR}/results/sample.out.free_functions.stereotypes.csv" "${SAMPLE_FREE_FUNCTIONS_CSV}")
    expect_missing("${WORK_DIR}/input/sample.stereotypes.csv")
    expect_missing("${WORK_DIR}/input/sample.free_functions.stereotypes.csv")

    run_stereocode(input/sample.xml -d -r -o results/only_reports.xml)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/results/only_reports.stereotypes.csv" "${SAMPLE_CSV}")
    expect_missing("${WORK_DIR}/results/only_reports.xml")

    run_stereocode(input/sample.xml -d -r -o missing/sample.xml)
    expect_error()
    if(NOT "${RUN_ERROR}" MATCHES "missing/sample.stereotypes.csv")
        message(FATAL_ERROR "The error does not name the CSV report: ${RUN_ERROR}")
    endif()
    expect_missing("${WORK_DIR}/missing")

    # A folder in place of the temporary file of the report keeps the report from being created
    file(MAKE_DIRECTORY "${WORK_DIR}/results/blocked.free_functions.stereotypes.csv.tmp")
    run_stereocode(input/sample.xml -f -r -o results/blocked.xml)
    expect_error()
    if(NOT "${RUN_ERROR}" MATCHES "results/blocked.free_functions.stereotypes.csv")
        message(FATAL_ERROR "The error does not name the CSV report: ${RUN_ERROR}")
    endif()
    foreach(output blocked.xml blocked.xml.tmp blocked.stereotypes.csv blocked.stereotypes.csv.tmp)
        expect_missing("${WORK_DIR}/results/${output}")
    endforeach()

    # A folder in place of a report keeps the report from replacing it, so the archive stays a temporary file
    file(MAKE_DIRECTORY "${WORK_DIR}/results/moved.stereotypes.csv")
    run_stereocode(input/sample.xml -r -o results/moved.xml)
    expect_error()
    if(NOT "${RUN_ERROR}" MATCHES "results/moved.stereotypes.csv")
        message(FATAL_ERROR "The error does not name the CSV report: ${RUN_ERROR}")
    endif()
    foreach(output moved.xml moved.xml.tmp moved.stereotypes.csv.tmp)
        expect_missing("${WORK_DIR}/results/${output}")
    endforeach()

    # With -o naming the input archive, such a failure keeps the input
    file(WRITE "${WORK_DIR}/results/inplace.xml" "${SAMPLE_INPUT}")
    file(MAKE_DIRECTORY "${WORK_DIR}/results/inplace.stereotypes.csv")
    run_stereocode(results/inplace.xml -r -o results/inplace.xml)
    expect_error()
    expect_file_content("${WORK_DIR}/results/inplace.xml" "${SAMPLE_INPUT}")
    expect_missing("${WORK_DIR}/results/inplace.xml.tmp")
endfunction()

# The CSV reports hold a row for a type without methods, and the verbose report has a column for the stereotypes of the type
# The rows follow the order of the types in the input archive
#
# The C++ file:
#   class E {                          class A {
#       int x;                             int y;
#   };                                 public:
#                                          int getY() const { return y; }
#                                      };
function(check_csv_type_rows)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0" language="C++" filename="e.cpp"><class>class <name>E</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</private>}</block>;</class>

<class>class <name>A</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>y</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>getY</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>y</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>
]=])

    set(EXPECTED_CSV [=[
type_name,type_stereotype,function_name,function_stereotype
"E","empty","N/A","N/A"
"A","data-provider data-class small-class","getY","get"
]=])

    set(VERBOSE_HEADER "type_file_name,type_name,type_stereotype,type_parents,type_inherited_parents,type_unknown_parents,type_inherited_function_signatures,function_file_name,function_name,function_stereotype,function_line_number,function_signature,function_unit_language,function_parameters_list,function_return_type,function_specifiers,function_external_calls,function_internal_calls,function_attributes,function_field_used,function_fields_modified,function_calls_on_fields")
    set(VERBOSE_EMPTY_TYPE_ROW [=["e.cpp","E","empty","N/A","N/A","False","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A"]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -d -r)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.csv" "${EXPECTED_CSV}")

    run_stereocode(input.xml -d -b)
    expect_exit(0)
    file(STRINGS "${WORK_DIR}/input.stereotypes.csv" verboseRows)
    list(LENGTH verboseRows rowCount)
    if(NOT rowCount EQUAL 3)
        message(FATAL_ERROR "Expected a header and two rows in the verbose report, got ${rowCount} lines")
    endif()
    list(GET verboseRows 0 header)
    list(GET verboseRows 1 emptyTypeRow)
    list(GET verboseRows 2 typeRow)
    if(NOT header STREQUAL VERBOSE_HEADER)
        message(FATAL_ERROR "Unexpected header of the verbose report:\n${header}")
    endif()
    if(NOT emptyTypeRow STREQUAL VERBOSE_EMPTY_TYPE_ROW)
        message(FATAL_ERROR "Unexpected row of the type without methods:\n${emptyTypeRow}")
    endif()
    if(NOT typeRow MATCHES "^\"e.cpp\",\"A\",\"data-provider data-class small-class\",\"N/A\",\"N/A\",\"False\",\"N/A\",\"e.cpp\",\"getY\",\"get\",")
        message(FATAL_ERROR "Unexpected row of the method getY:\n${typeRow}")
    endif()
endfunction()

# Fails unless a row of the report starts with columns that match the regular expression, given in one or more parts
function(expect_row report)
    string(CONCAT rowStart ${ARGN})
    file(READ "${WORK_DIR}/${report}" content)
    if(NOT content MATCHES "\n${rowStart}")
        message(FATAL_ERROR "No row of ${report} starts with\n${rowStart}\n${content}")
    endif()
endfunction()

# The verbose CSV reports (-b) give the file of the type and the file of the function, list each inherited parent once,
# flag a parent that has the simple name of its child as unknown, list the specifiers of a function in source order,
# give each signature with the types of its parameters, without their names and specifiers, and name a nested type with the types around it
#
# The archive holds these files:
#   chain.hpp   class Base { protected: int a; };
#               class Mid : public Base { };
#               class Leaf : public Mid { public: int twice() const { return a * 2; } };
#               class Tip : public Leaf { public: int getA() const { return a; } };
#   self.hpp    class Widget : public ui::Widget { int w; public: int width() const { return w; } };
#   Self.cs     public class Logger : Framework.Logger { private int level; public int Level() { return level; } }
#   Spec.cs     class A { int x; protected internal virtual unsafe int GetX() { return x; } }
#   split.hpp   class Counter { int count; public: int get() const { return count; } void add(int n); };
#   split.cpp   void Counter::add(int n) { count += n; }
#               int twice(int v) { return 2 * v; }
#   overloads.hpp class Meter { int m_v; public: int scale(int unit, int count) const { return m_v * unit * count; }
#                               double scale(double unit, double count) const { return m_v * unit * count; }
#                               void reset(const Meter& other, unsigned long n = 2); };
#               class Probe : public Meter { };
#   nested.hpp  class Outer { public: class Inner { int v; public: int get() const { return v; } }; };
#   Nested.cs   class Box<T> { class Slot { int n; public int Get() { return n; } } }
function(check_verbose_csv)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit revision="1.0.0" language="C++" filename="chain.hpp"><class>class <name>Base</name> <block>{<private type="default"> </private><protected>protected: <decl_stmt><decl><type><name>int</name></type> <name>a</name></decl>;</decl_stmt> </protected>}</block>;</class>
<class>class <name>Mid</name> <super_list>: <super><specifier>public</specifier> <name>Base</name></super></super_list> <block>{<private type="default"> </private>}</block>;</class>
<class>class <name>Leaf</name> <super_list>: <super><specifier>public</specifier> <name>Mid</name></super></super_list> <block>{<private type="default"> </private><public>public: <function><type><name>int</name></type> <name>twice</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>a</name> <operator>*</operator> <literal type="number">2</literal></expr>;</return> </block_content>}</block></function> </public>}</block>;</class>
<class>class <name>Tip</name> <super_list>: <super><specifier>public</specifier> <name>Leaf</name></super></super_list> <block>{<private type="default"> </private><public>public: <function><type><name>int</name></type> <name>getA</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>a</name></expr>;</return> </block_content>}</block></function> </public>}</block>;</class>
</unit>

<unit revision="1.0.0" language="C++" filename="self.hpp"><class>class <name>Widget</name> <super_list>: <super><specifier>public</specifier> <name><name>ui</name><operator>::</operator><name>Widget</name></name></super></super_list> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>w</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>width</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>w</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
</unit>

<unit revision="1.0.0" language="C#" filename="Self.cs"><class><specifier>public</specifier> class <name>Logger</name> <super_list>: <super><name><name>Framework</name><operator>.</operator><name>Logger</name></name></super></super_list> <block>{
    <decl_stmt><decl><type><specifier>private</specifier> <name>int</name></type> <name>level</name></decl>;</decl_stmt>
    <function><type><specifier>public</specifier> <name>int</name></type> <name>Level</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>level</name></expr>;</return> </block_content>}</block></function>
}</block></class>
</unit>

<unit revision="1.0.0" language="C#" filename="Spec.cs"><class>class <name>A</name> <block>{
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
    <function><type><specifier>protected</specifier> <specifier>internal</specifier> <specifier>virtual</specifier> <specifier>unsafe</specifier> <name>int</name></type> <name>GetX</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>x</name></expr>;</return> </block_content>}</block></function>
}</block></class>
</unit>

<unit revision="1.0.0" language="C++" filename="split.hpp"><class>class <name>Counter</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>count</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>count</name></expr>;</return> </block_content>}</block></function>
    <function_decl><type><name>void</name></type> <name>add</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>n</name></decl></parameter>)</parameter_list>;</function_decl>
</public>}</block>;</class>
</unit>

<unit revision="1.0.0" language="C++" filename="split.cpp"><function><type><name>void</name></type> <name><name>Counter</name><operator>::</operator><name>add</name></name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>n</name></decl></parameter>)</parameter_list> <block>{<block_content> <expr_stmt><expr><name>count</name> <operator>+=</operator> <name>n</name></expr>;</expr_stmt> </block_content>}</block></function>

<function><type><name>int</name></type> <name>twice</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>v</name></decl></parameter>)</parameter_list> <block>{<block_content> <return>return <expr><literal type="number">2</literal> <operator>*</operator> <name>v</name></expr>;</return> </block_content>}</block></function>
</unit>

<unit revision="1.0.0" language="C++" filename="overloads.hpp"><class>class <name>Meter</name> <block>{<private type="default"> <decl_stmt><decl><type><name>int</name></type> <name>m_v</name></decl>;</decl_stmt> </private><public>public: <function><type><name>int</name></type> <name>scale</name><parameter_list>(<parameter><decl><type><name>int</name></type> <name>unit</name></decl></parameter>, <parameter><decl><type><name>int</name></type> <name>count</name></decl></parameter>)</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>m_v</name> <operator>*</operator> <name>unit</name> <operator>*</operator> <name>count</name></expr>;</return> </block_content>}</block></function> <function><type><name>double</name></type> <name>scale</name><parameter_list>(<parameter><decl><type><name>double</name></type> <name>unit</name></decl></parameter>, <parameter><decl><type><name>double</name></type> <name>count</name></decl></parameter>)</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>m_v</name> <operator>*</operator> <name>unit</name> <operator>*</operator> <name>count</name></expr>;</return> </block_content>}</block></function> <function_decl><type><name>void</name></type> <name>reset</name><parameter_list>(<parameter><decl><type><specifier>const</specifier> <name>Meter</name><modifier>&amp;</modifier></type> <name>other</name></decl></parameter>, <parameter><decl><type><name>unsigned</name> <name>long</name></type> <name>n</name> <init>= <expr><literal type="number">2</literal></expr></init></decl></parameter>)</parameter_list>;</function_decl> </public>}</block>;</class>
<class>class <name>Probe</name> <super_list>: <super><specifier>public</specifier> <name>Meter</name></super></super_list> <block>{<private type="default"> </private>}</block>;</class>
</unit>

<unit revision="1.0.0" language="C++" filename="nested.hpp"><class>class <name>Outer</name> <block>{<private type="default"> </private><public>public: <class>class <name>Inner</name> <block>{<private type="default"> <decl_stmt><decl><type><name>int</name></type> <name>v</name></decl>;</decl_stmt> </private><public>public: <function><type><name>int</name></type> <name>get</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>v</name></expr>;</return> </block_content>}</block></function> </public>}</block>;</class> </public>}</block>;</class>
</unit>

<unit revision="1.0.0" language="C#" filename="Nested.cs"><class>class <name><name>Box</name><argument_list type="generic">&lt;<argument><name>T</name></argument>&gt;</argument_list></name> <block>{ <class>class <name>Slot</name> <block>{ <decl_stmt><decl><type><name>int</name></type> <name>n</name></decl>;</decl_stmt> <function><type><specifier>public</specifier> <name>int</name></type> <name>Get</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>n</name></expr>;</return> </block_content>}</block></function> }</block></class> }</block></class>
</unit>

</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f -d -b)
    expect_exit(0)

    # The type columns: file, name, stereotypes, parents, inherited parents, unknown parents
    expect_row(input.stereotypes.csv [=["chain.hpp","Base","[^"]*","N/A","N/A","False",]=])
    expect_row(input.stereotypes.csv [=["chain.hpp","Mid","[^"]*","Base","N/A","False",]=])
    expect_row(input.stereotypes.csv [=["chain.hpp","Leaf","[^"]*","Mid","Base","False",]=])
    expect_row(input.stereotypes.csv [=["chain.hpp","Tip","[^"]*","Leaf","Mid Base","False",]=])
    expect_row(input.stereotypes.csv [=["self.hpp","Widget","[^"]*","Widget","N/A","True",]=])
    expect_row(input.stereotypes.csv [=["Self.cs","Logger","[^"]*","Logger","N/A","True",]=])

    # The function columns of an out-of-class method and of a free function start with the file of the function
    expect_row(input.stereotypes.csv [=["split.hpp","Counter","[^"]*","N/A","N/A","False","N/A","split.cpp","add",]=])
    expect_row(input.free_functions.stereotypes.csv [=["N/A","N/A","N/A","N/A","N/A","N/A","N/A","split.cpp","twice",]=])

    # A nested type takes the names of the types around it, as in the report without -b, and the generic arguments of every name stay out
    expect_row(input.stereotypes.csv [=["nested.hpp","Outer::Inner","[^"]*","N/A","N/A","False","N/A","nested.hpp","get",]=])
    expect_row(input.stereotypes.csv [=["Nested.cs","Box.Slot","[^"]*","N/A","N/A","False","N/A","Nested.cs","Get",]=])

    # A signature holds the types of the parameters, which tell overloads of one name and count apart
    expect_row(input.stereotypes.csv [=["overloads.hpp","Meter","[^"]*","N/A","N/A","False","N/A","overloads.hpp","scale","[^"]*","N/A","scale\(int,int\)",]=])
    expect_row(input.stereotypes.csv [=["overloads.hpp","Meter","[^"]*","N/A","N/A","False","N/A","overloads.hpp","scale","[^"]*","N/A","scale\(double,double\)",]=])
    expect_row(input.stereotypes.csv [=["overloads.hpp","Probe","[^"]*","Meter","N/A","False","scale\(double,double\) scale\(int,int\) reset\(Meter&,unsignedlong\)",]=])

    file(READ "${WORK_DIR}/input.stereotypes.csv" report)
    if(NOT report MATCHES "\n\"Spec.cs\",\"A\",[^\n]*,\"GetX\",[^\n]*,\"protected internal virtual unsafe\",")
        message(FATAL_ERROR "The specifiers of GetX are not in source order:\n${report}")
    endif()

    # The types follow the order of the input archive
    string(REGEX MATCHALL "\n\"[^\"]*\",\"[^\"]*\"" typeColumns "${report}")
    list(REMOVE_DUPLICATES typeColumns)
    string(REPLACE "\n" "" typeColumns "${typeColumns}")
    set(expectedTypes [=["chain.hpp","Base"]=] [=["chain.hpp","Mid"]=] [=["chain.hpp","Leaf"]=] [=["chain.hpp","Tip"]=]
                      [=["self.hpp","Widget"]=] [=["Self.cs","Logger"]=] [=["Spec.cs","A"]=] [=["split.hpp","Counter"]=]
                      [=["overloads.hpp","Meter"]=] [=["overloads.hpp","Probe"]=] [=["nested.hpp","Outer"]=] [=["nested.hpp","Outer::Inner"]=]
                      [=["Nested.cs","Box"]=] [=["Nested.cs","Box.Slot"]=])
    if(NOT typeColumns STREQUAL expectedTypes)
        message(FATAL_ERROR "The types do not follow the order of the input archive:\n${typeColumns}")
    endif()
endfunction()

# The verbose CSV report gives the line of each function in an archive that srcml --position writes, the calls of a method to the
# other methods of its type apart from its other calls, and the attributes of C# and the annotations of Java, with each quote doubled
#
# Report.cs                                          Clock.java
#   public class Report                                public class Clock {
#   {                                                      private int ticks;
#       private int total;
#                                                          @Deprecated
#       [System.Obsolete("Use Sum")]                       public int ticks() { return ticks; }
#       public int Total() { return total; }           }
#
#       public int Twice() { return Double(Total()); }
#
#       private static int Double(int value) { return 2 * value; }
#   }
function(check_verbose_csv_details)
    set(INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" xmlns:pos="http://www.srcML.org/srcML/position" revision="1.0.0" pos:tabs="8">

<unit revision="1.0.0" language="C#" filename="Report.cs" pos:tabs="8"><class pos:start="1:1" pos:end="11:1"><specifier pos:start="1:1" pos:end="1:6">public</specifier> class <name pos:start="1:14" pos:end="1:19">Report</name>
<block pos:start="2:1" pos:end="11:1">{
    <decl_stmt pos:start="3:5" pos:end="3:22"><decl pos:start="3:5" pos:end="3:21"><type pos:start="3:5" pos:end="3:15"><specifier pos:start="3:5" pos:end="3:11">private</specifier> <name pos:start="3:13" pos:end="3:15">int</name></type> <name pos:start="3:17" pos:end="3:21">total</name></decl>;</decl_stmt>

    <function pos:start="5:5" pos:end="6:40"><attribute pos:start="5:5" pos:end="5:32">[<expr pos:start="5:6" pos:end="5:31"><call pos:start="5:6" pos:end="5:31"><name pos:start="5:6" pos:end="5:20"><name pos:start="5:6" pos:end="5:11">System</name><operator pos:start="5:12" pos:end="5:12">.</operator><name pos:start="5:13" pos:end="5:20">Obsolete</name></name><argument_list pos:start="5:21" pos:end="5:31">(<argument pos:start="5:22" pos:end="5:30"><expr pos:start="5:22" pos:end="5:30"><literal type="string" pos:start="5:22" pos:end="5:30">"Use Sum"</literal></expr></argument>)</argument_list></call></expr>]</attribute>
    <type pos:start="6:5" pos:end="6:14"><specifier pos:start="6:5" pos:end="6:10">public</specifier> <name pos:start="6:12" pos:end="6:14">int</name></type> <name pos:start="6:16" pos:end="6:20">Total</name><parameter_list pos:start="6:21" pos:end="6:22">()</parameter_list> <block pos:start="6:24" pos:end="6:40">{<block_content pos:start="6:25" pos:end="6:39"> <return pos:start="6:26" pos:end="6:38">return <expr pos:start="6:33" pos:end="6:37"><name pos:start="6:33" pos:end="6:37">total</name></expr>;</return> </block_content>}</block></function>

    <function pos:start="8:5" pos:end="8:50"><type pos:start="8:5" pos:end="8:14"><specifier pos:start="8:5" pos:end="8:10">public</specifier> <name pos:start="8:12" pos:end="8:14">int</name></type> <name pos:start="8:16" pos:end="8:20">Twice</name><parameter_list pos:start="8:21" pos:end="8:22">()</parameter_list> <block pos:start="8:24" pos:end="8:50">{<block_content pos:start="8:25" pos:end="8:49"> <return pos:start="8:26" pos:end="8:48">return <expr pos:start="8:33" pos:end="8:47"><call pos:start="8:33" pos:end="8:47"><name pos:start="8:33" pos:end="8:38">Double</name><argument_list pos:start="8:39" pos:end="8:47">(<argument pos:start="8:40" pos:end="8:46"><expr pos:start="8:40" pos:end="8:46"><call pos:start="8:40" pos:end="8:46"><name pos:start="8:40" pos:end="8:44">Total</name><argument_list pos:start="8:45" pos:end="8:46">()</argument_list></call></expr></argument>)</argument_list></call></expr>;</return> </block_content>}</block></function>

    <function pos:start="10:5" pos:end="10:62"><type pos:start="10:5" pos:end="10:22"><specifier pos:start="10:5" pos:end="10:11">private</specifier> <specifier pos:start="10:13" pos:end="10:18">static</specifier> <name pos:start="10:20" pos:end="10:22">int</name></type> <name pos:start="10:24" pos:end="10:29">Double</name><parameter_list pos:start="10:30" pos:end="10:40">(<parameter pos:start="10:31" pos:end="10:39"><decl pos:start="10:31" pos:end="10:39"><type pos:start="10:31" pos:end="10:33"><name pos:start="10:31" pos:end="10:33">int</name></type> <name pos:start="10:35" pos:end="10:39">value</name></decl></parameter>)</parameter_list> <block pos:start="10:42" pos:end="10:62">{<block_content pos:start="10:43" pos:end="10:61"> <return pos:start="10:44" pos:end="10:60">return <expr pos:start="10:51" pos:end="10:59"><literal type="number" pos:start="10:51" pos:end="10:51">2</literal> <operator pos:start="10:53" pos:end="10:53">*</operator> <name pos:start="10:55" pos:end="10:59">value</name></expr>;</return> </block_content>}</block></function>
}</block></class>
</unit>

<unit revision="1.0.0" language="Java" filename="Clock.java" pos:tabs="8"><class pos:start="1:1" pos:end="6:1"><specifier pos:start="1:1" pos:end="1:6">public</specifier> class <name pos:start="1:14" pos:end="1:18">Clock</name> <block pos:start="1:20" pos:end="6:1">{
    <decl_stmt pos:start="2:5" pos:end="2:22"><decl pos:start="2:5" pos:end="2:21"><type pos:start="2:5" pos:end="2:15"><specifier pos:start="2:5" pos:end="2:11">private</specifier> <name pos:start="2:13" pos:end="2:15">int</name></type> <name pos:start="2:17" pos:end="2:21">ticks</name></decl>;</decl_stmt>

    <function pos:start="4:5" pos:end="5:40"><annotation pos:start="4:5" pos:end="4:15">@<name pos:start="4:6" pos:end="4:15">Deprecated</name></annotation>
    <type pos:start="5:5" pos:end="5:14"><specifier pos:start="5:5" pos:end="5:10">public</specifier> <name pos:start="5:12" pos:end="5:14">int</name></type> <name pos:start="5:16" pos:end="5:20">ticks</name><parameter_list pos:start="5:21" pos:end="5:22">()</parameter_list> <block pos:start="5:24" pos:end="5:40">{<block_content pos:start="5:25" pos:end="5:39"> <return pos:start="5:26" pos:end="5:38">return <expr pos:start="5:33" pos:end="5:37"><name pos:start="5:33" pos:end="5:37">ticks</name></expr>;</return> </block_content>}</block></function>
}</block></class>
</unit>

</unit>
]=])

    file(WRITE "${WORK_DIR}/input.xml" "${INPUT}")

    run_stereocode(input.xml -f -d -b)
    expect_exit(0)

    # The columns after the name: stereotypes, line, signature, language, parameters, return type, specifiers, external calls,
    # internal calls, and attributes
    set(typeColumns [=["Report.cs","Report","[^"]*","N/A","N/A","False","N/A","Report.cs"]=])
    expect_row(input.stereotypes.csv "${typeColumns}"
               [=[,"Total","[^"]*","6","Total\(\)","C#","\(\)","public int","public","N/A","N/A","\[System\.Obsolete\(""Use Sum""\)\]",]=])
    expect_row(input.stereotypes.csv "${typeColumns}"
               [=[,"Twice","[^"]*","8","Twice\(\)","C#","\(\)","public int","public","Double\(_\)","Total\(\)","N/A",]=])
    expect_row(input.stereotypes.csv [=["Clock.java","Clock","[^"]*","N/A","N/A","False","N/A","Clock.java"]=]
               [=[,"ticks","[^"]*","5","ticks\(\)","Java","\(\)","public int","public","N/A","N/A","@Deprecated",]=])
    expect_row(input.free_functions.stereotypes.csv [=["N/A","N/A","N/A","N/A","N/A","N/A","N/A","Report.cs"]=]
               [=[,"Double","[^"]*","10","Double\(int\)","C#","\(int value\)","private static int","private static","N/A","N/A","N/A",]=])
endfunction()

# Gets a row of a text report, with the first column filled with spaces to 70 characters
function(txt_row first second result)
    string(LENGTH "${first}" length)
    math(EXPR padding "70 - ${length}")
    string(REPEAT " " ${padding} spaces)
    set(${result} "${first}${spaces}${second}\n" PARENT_SCOPE)
endfunction()

# With -x, the text reports of the ICSME 2024 paper sit beside the srcML output archive and take its name like the CSV reports
# Each type gets a block with its name, its stereotypes, and its methods, closed by a line of dashes, and -f adds the report of the free functions
# The first column is 70 characters wide, a longer name keeps one space before the stereotypes, and a name over two lines takes one line
# A report that cannot be created stops the run with an error that names the report, and the run writes no output
#
# The files of names.xml:
#   template<typename T, typename U> class Pair {};       class ThermometerCalibrationRecordForTheNorthernWeatherStationNetworkArchive2026 {
#   template<> class Pair<int,                                private int value;
#                        long> {                              public int getValue() { return value; }
#       int first;                                        }
#   public:
#       int getFirst() const { return first; }
#   };
#   class E {
#       int x;
#   };
function(check_txt_report)
    set(NAMES_INPUT [=[
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<unit xmlns="http://www.srcML.org/srcML/src" revision="1.0.0">

<unit revision="1.0.0" language="C++" filename="pair.cpp" hash="d495a9654bcf3ef3ef5c55fbbe419a942b05b00b"><class><template>template<parameter_list>&lt;<parameter><type><name>typename</name></type> <name>T</name></parameter>, <parameter><type><name>typename</name></type> <name>U</name></parameter>&gt;</parameter_list></template> class <name>Pair</name> <block>{<private type="default"/>}</block>;</class>
<class><template>template<parameter_list>&lt;&gt;</parameter_list></template> class <name><name>Pair</name><argument_list type="generic">&lt;<argument><expr><name>int</name></expr></argument>,
                     <argument><expr><name>long</name></expr></argument>&gt;</argument_list></name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>first</name></decl>;</decl_stmt>
</private><public>public:
    <function><type><name>int</name></type> <name>getFirst</name><parameter_list>()</parameter_list> <specifier>const</specifier> <block>{<block_content> <return>return <expr><name>first</name></expr>;</return> </block_content>}</block></function>
</public>}</block>;</class>
<class>class <name>E</name> <block>{<private type="default">
    <decl_stmt><decl><type><name>int</name></type> <name>x</name></decl>;</decl_stmt>
</private>}</block>;</class>
</unit>

<unit revision="1.0.0" language="Java" filename="Long.java" hash="0fca5a3fe13024ff699b5cefe13528bf6364be57"><class>class <name>ThermometerCalibrationRecordForTheNorthernWeatherStationNetworkArchive2026</name> <block>{
    <decl_stmt><decl><type><specifier>private</specifier> <name>int</name></type> <name>value</name></decl>;</decl_stmt>
    <function><type><specifier>public</specifier> <name>int</name></type> <name>getValue</name><parameter_list>()</parameter_list> <block>{<block_content> <return>return <expr><name>value</name></expr>;</return> </block_content>}</block></function>
}</block></class>
</unit>

</unit>
]=])

    string(REPEAT "-" 140 SEPARATOR)
    txt_row("Class Name:" "Class Stereotype:" CLASS_HEADER)
    txt_row("Method Name:" "Method Stereotype:" METHOD_HEADER)
    txt_row("Free Function Name:" "Free Function Stereotype:" FREE_FUNCTION_HEADER)

    txt_row("A" "data-provider data-class small-class" ROW_A)
    txt_row("getX" "get" ROW_GET_X)
    txt_row("twice" "property" ROW_TWICE)
    set(SAMPLE_TXT "${CLASS_HEADER}${ROW_A}\n${METHOD_HEADER}${ROW_GET_X}${SEPARATOR}\n")
    set(SAMPLE_FREE_FUNCTIONS_TXT "${FREE_FUNCTION_HEADER}${ROW_TWICE}${SEPARATOR}\n")

    txt_row("Pair" "empty" ROW_PAIR)
    txt_row("Pair<int, long>" "data-provider data-class small-class" ROW_PAIR_SPECIALIZATION)
    txt_row("getFirst" "get" ROW_GET_FIRST)
    txt_row("E" "empty" ROW_E)
    txt_row("getValue" "get" ROW_GET_VALUE)
    set(ROW_LONG_NAME "ThermometerCalibrationRecordForTheNorthernWeatherStationNetworkArchive2026 data-provider data-class small-class\n")
    set(NAMES_TXT
        "${CLASS_HEADER}${ROW_PAIR}\n${METHOD_HEADER}${SEPARATOR}\n"
        "${CLASS_HEADER}${ROW_PAIR_SPECIALIZATION}\n${METHOD_HEADER}${ROW_GET_FIRST}${SEPARATOR}\n"
        "${CLASS_HEADER}${ROW_E}\n${METHOD_HEADER}${SEPARATOR}\n"
        "${CLASS_HEADER}${ROW_LONG_NAME}\n${METHOD_HEADER}${ROW_GET_VALUE}${SEPARATOR}\n")
    string(CONCAT NAMES_TXT ${NAMES_TXT})

    file(MAKE_DIRECTORY "${WORK_DIR}/results")
    file(WRITE "${WORK_DIR}/input.xml" "${SAMPLE_INPUT}")
    file(WRITE "${WORK_DIR}/names.xml" "${NAMES_INPUT}")

    run_stereocode(--help)
    expect_exit(0)
    if(NOT RUN_OUTPUT MATCHES "\n  -x, +--txt-report +Writes a text report of the stereotypes")
        message(FATAL_ERROR "The help does not describe -x:\n${RUN_OUTPUT}")
    endif()

    run_stereocode(input.xml -f -x)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/input.stereotypes.txt" "${SAMPLE_TXT}")
    expect_file_content("${WORK_DIR}/input.free_functions.stereotypes.txt" "${SAMPLE_FREE_FUNCTIONS_TXT}")
    expect_file_content("${WORK_DIR}/input.stereotypes.xml" "${SAMPLE_OUTPUT}")
    expect_missing("${WORK_DIR}/input.stereotypes.csv")

    # Without -f, the run writes no report of the free functions, and -r adds the CSV reports to the text reports
    run_stereocode(input.xml -x -r -o results/types.xml)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/results/types.stereotypes.txt" "${SAMPLE_TXT}")
    expect_file_content("${WORK_DIR}/results/types.stereotypes.csv" "${SAMPLE_CSV}")
    expect_missing("${WORK_DIR}/results/types.free_functions.stereotypes.txt")

    # With -d, the text reports are the only output, and -d without a report names -x among the reports
    run_stereocode(input.xml -d -x -f -o results/only_reports.xml)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/results/only_reports.stereotypes.txt" "${SAMPLE_TXT}")
    expect_file_content("${WORK_DIR}/results/only_reports.free_functions.stereotypes.txt" "${SAMPLE_FREE_FUNCTIONS_TXT}")
    expect_missing("${WORK_DIR}/results/only_reports.xml")

    run_stereocode(input.xml -d -o results/nothing.xml)
    expect_error()
    if(NOT RUN_ERROR MATCHES "--txt-report \\(-x\\)")
        message(FATAL_ERROR "The error of -d without a report does not name -x: ${RUN_ERROR}")
    endif()
    expect_missing("${WORK_DIR}/results/nothing.stereotypes.txt")

    # Types without methods, a name over two lines, and a name of more than 70 characters
    run_stereocode(names.xml -d -x)
    expect_exit(0)
    expect_file_content("${WORK_DIR}/names.stereotypes.txt" "${NAMES_TXT}")

    # A folder in place of the temporary file of a report keeps the report from being created
    file(MAKE_DIRECTORY "${WORK_DIR}/results/blocked.free_functions.stereotypes.txt.tmp")
    run_stereocode(input.xml -f -x -r -o results/blocked.xml)
    expect_error()
    if(NOT RUN_ERROR MATCHES "results/blocked.free_functions.stereotypes.txt")
        message(FATAL_ERROR "The error does not name the text report: ${RUN_ERROR}")
    endif()
    foreach(output blocked.xml blocked.xml.tmp blocked.stereotypes.csv blocked.stereotypes.txt blocked.stereotypes.txt.tmp)
        expect_missing("${WORK_DIR}/results/${output}")
    endforeach()

    # A folder in place of a report keeps the report from replacing it, and the srcML output archive then stays a temporary file
    file(MAKE_DIRECTORY "${WORK_DIR}/results/moved.stereotypes.txt")
    run_stereocode(input.xml -x -o results/moved.xml)
    expect_error()
    if(NOT RUN_ERROR MATCHES "results/moved.stereotypes.txt")
        message(FATAL_ERROR "The error does not name the text report: ${RUN_ERROR}")
    endif()
    foreach(output moved.xml moved.xml.tmp moved.stereotypes.txt.tmp)
        expect_missing("${WORK_DIR}/results/${output}")
    endforeach()
endfunction()

run_check()

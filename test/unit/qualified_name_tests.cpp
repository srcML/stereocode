// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file qualified_name_tests.cpp
 *
 * @copyright Copyright (C) 2021-2026 srcML, LLC. (www.srcML.org)
 *
 * This file is part of the Stereocode application.
 */

#include "doctest.h"
#include "qualified_name.hpp"

#include <string>
#include <vector>

using parts = std::vector<std::string>;

TEST_CASE("a qualified name splits at the separators of its language outside generic argument lists") {
    CHECK(splitQualifiedName("Widget::Impl::get", "C++") == parts{"Widget", "Impl", "get"});
    CHECK(splitQualifiedName("Box<std::string>::set", "C++") == parts{"Box<std::string>", "set"});
    CHECK(splitQualifiedName("ratio_string<std::micro>::symbol", "C++") == parts{"ratio_string<std::micro>", "symbol"});
    CHECK(splitQualifiedName("::std::string", "C++") == parts{"std", "string"});
    CHECK(splitQualifiedName("a.b.Node<T>", "Java") == parts{"a", "b", "Node<T>"});
    CHECK(splitQualifiedName("Map<K,List<V>>.Entry", "C#") == parts{"Map<K,List<V>>", "Entry"});
    CHECK(splitQualifiedName("Node", "Java") == parts{"Node"});
}

TEST_CASE("the angle brackets of an operator name are no generic argument list") {
    CHECK(splitQualifiedName("Point::operator<", "C++") == parts{"Point", "operator<"});
    CHECK(splitQualifiedName("Stream::operator<<", "C++") == parts{"Stream", "operator<<"});
    CHECK(splitQualifiedName("Box<T>::operator->", "C++") == parts{"Box<T>", "operator->"});
    CHECK(withoutGenericArguments("operator<<") == "operator<<");
    CHECK(withoutGenericArguments("Box<T>::operator<=>") == "Box::operator<=>");
    CHECK(genericSkeleton("operator<") == "operator<");
}

TEST_CASE("the generic argument lists of a name come off whole") {
    CHECK(withoutGenericArguments("Box<T>::touch") == "Box::touch");
    CHECK(withoutGenericArguments("Holder<std::pair<int,int>>::Inner") == "Holder::Inner");
    CHECK(withoutGenericArguments("Collections.<String>emptyList") == "Collections.emptyList");
    CHECK(genericArguments("Box<int>") == "<int>");
    CHECK(genericArguments("Box") == "");
}

TEST_CASE("the skeleton of a name keeps the brackets and the commas of each generic argument list") {
    CHECK(genericSkeleton("Map<K,List<V>>") == "Map<,>");
    CHECK(genericSkeleton("ns::Box<int,std::pair<int,int>>") == "ns::Box<,>");
    CHECK(genericSkeleton("Box<T>::touch") == "Box<>::touch");
}

TEST_CASE("the arity of a name counts the parameters of its last generic argument list") {
    CHECK(genericArity("Node") == 0);
    CHECK(genericArity("Node<T>") == 1);
    CHECK(genericArity("Map<K,List<V>>") == 2);
    CHECK(genericArity("Outer<A>.Inner<B,C,D>") == 3);
}

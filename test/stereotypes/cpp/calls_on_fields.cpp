// One call on a data member without a write is no set, and a const method writes no data member
// (Method-Stereotypes wiki, set and command)

#include <algorithm>
#include <vector>

class Bag {
  std::vector<int> m_items;
public:
  // @stereotype property
  int count() const { return m_items.size(); }
  // @stereotype predicate
  bool isEmpty() const { return m_items.empty(); }
  // @stereotype command
  void add(int x) { m_items.push_back(x); }
  // A call of a query method on a data member reads the data member, also in a method that is not const
  // @stereotype predicate
  bool hasItems() { return !m_items.empty(); }
  // A call that gives write access to the data member, as front, begin, or at do, may change the data member
  // @stereotype property non-void-command
  int first() { return m_items.front(); }
  // A write through the result of a query method changes the data member
  // @stereotype command
  void setFirst(int x) { m_items.front() = x; }
  // @stereotype command
  void bumpLast() { ++m_items.back(); }
  // @stereotype command
  void clearAt(int i) { m_items.at(i) = 0; }
  // An iterator or a reference that such a call gives may write the data member later
  // @stereotype command
  void fillZero() { std::fill(m_items.begin(), m_items.end(), 0); }
  // @stereotype command
  void viaRef(int i) { int& r = m_items.at(i); r = 2; }
  // The capacity is a query of the data member
  // @stereotype property
  int room() { return m_items.capacity() - m_items.size(); }
};

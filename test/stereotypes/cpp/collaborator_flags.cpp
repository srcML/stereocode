// A method that works with an object of another class is a collaborator whatever the order of its
// parameters, locals, and data members (Method-Stereotypes wiki, collaborator)

class Foo;
class C { int n; Foo* ext;
public:
  // @stereotype set collaborator
  void ownFirst(C* own, Foo* ext) { n = 1; }
  // @stereotype set collaborator
  void extFirst(Foo* ext, C* own) { n = 1; }
  // @stereotype set collaborator
  void localsOwnFirst() { C* own = 0; Foo* ext = 0; n = 1; }
  // A data member that is an object of another class makes a collaborator
  // @stereotype property collaborator
  int onlyExt() { return ext ? 1 : 0; }
  // @stereotype property collaborator
  int both() { return n + (ext ? 1 : 0); }
};

class Widget;

class HolderA {
  Widget* w;
  int n;
public:
  // @stereotype property collaborator
  int score() const { return (w ? 1 : 0) + n; }
};

class HolderB {
  Widget* w;
  int n;
public:
  // @stereotype property collaborator
  int score() const { return n + (w ? 1 : 0); }
};

class Node {
  int n;
public:
  // @stereotype set collaborator
  void absorbA(const Node& other, Widget* w) { n = other.n; }
  // @stereotype set collaborator
  void absorbB(Widget* w, const Node& other) { n = other.n; }
  // @stereotype set collaborator
  void localsA() { Widget* w = 0; Node copy = *this; n = 1; }
  // @stereotype set collaborator
  void localsB() { Node copy = *this; Widget* w = 0; n = 1; }
};

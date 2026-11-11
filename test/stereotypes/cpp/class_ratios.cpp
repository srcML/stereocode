// Class rules with the fractions of the Class-Stereotypes wiki: |degenerate| > 1/3 |methods| (lazy-class),
// |controller| < 1/3 |methods| (boundary), and |factory| > 2/3 |methods| (factory and controller)

class Widget { public: void poke(); };

// One degenerate method of three is not more than one third
// @stereotype unclassified
class Lazy3 {
  int m_a;
public:
  int a() const { return m_a; }
  void setA(int v) { m_a = v; }
  int one() const { return 1; }
};

// 33 controllers of 100 methods are fewer than one third, beside 67 get methods
// @stereotype data-provider boundary
class Boundary100 {
  int m_a;
  int m_b;
public:
  void k1(Widget* w) { w->poke(); }
  void k2(Widget* w) { w->poke(); }
  void k3(Widget* w) { w->poke(); }
  void k4(Widget* w) { w->poke(); }
  void k5(Widget* w) { w->poke(); }
  void k6(Widget* w) { w->poke(); }
  void k7(Widget* w) { w->poke(); }
  void k8(Widget* w) { w->poke(); }
  void k9(Widget* w) { w->poke(); }
  void k10(Widget* w) { w->poke(); }
  void k11(Widget* w) { w->poke(); }
  void k12(Widget* w) { w->poke(); }
  void k13(Widget* w) { w->poke(); }
  void k14(Widget* w) { w->poke(); }
  void k15(Widget* w) { w->poke(); }
  void k16(Widget* w) { w->poke(); }
  void k17(Widget* w) { w->poke(); }
  void k18(Widget* w) { w->poke(); }
  void k19(Widget* w) { w->poke(); }
  void k20(Widget* w) { w->poke(); }
  void k21(Widget* w) { w->poke(); }
  void k22(Widget* w) { w->poke(); }
  void k23(Widget* w) { w->poke(); }
  void k24(Widget* w) { w->poke(); }
  void k25(Widget* w) { w->poke(); }
  void k26(Widget* w) { w->poke(); }
  void k27(Widget* w) { w->poke(); }
  void k28(Widget* w) { w->poke(); }
  void k29(Widget* w) { w->poke(); }
  void k30(Widget* w) { w->poke(); }
  void k31(Widget* w) { w->poke(); }
  void k32(Widget* w) { w->poke(); }
  void k33(Widget* w) { w->poke(); }
  int x34(Widget* w) const { return m_a; }
  int x35(Widget* w) const { return m_a; }
  int x36(Widget* w) const { return m_a; }
  int x37(Widget* w) const { return m_a; }
  int x38(Widget* w) const { return m_a; }
  int x39(Widget* w) const { return m_a; }
  int x40(Widget* w) const { return m_a; }
  int x41(Widget* w) const { return m_a; }
  int x42(Widget* w) const { return m_a; }
  int x43(Widget* w) const { return m_a; }
  int x44(Widget* w) const { return m_a; }
  int x45(Widget* w) const { return m_a; }
  int x46(Widget* w) const { return m_a; }
  int x47(Widget* w) const { return m_a; }
  int x48(Widget* w) const { return m_a; }
  int x49(Widget* w) const { return m_a; }
  int x50(Widget* w) const { return m_a; }
  int x51(Widget* w) const { return m_a; }
  int g52() const { return m_a; }
  int g53() const { return m_a; }
  int g54() const { return m_a; }
  int g55() const { return m_a; }
  int g56() const { return m_a; }
  int g57() const { return m_a; }
  int g58() const { return m_a; }
  int g59() const { return m_a; }
  int g60() const { return m_a; }
  int g61() const { return m_a; }
  int g62() const { return m_a; }
  int g63() const { return m_a; }
  int g64() const { return m_a; }
  int g65() const { return m_a; }
  int g66() const { return m_a; }
  int g67() const { return m_a; }
  int g68() const { return m_a; }
  int g69() const { return m_a; }
  int g70() const { return m_a; }
  int g71() const { return m_a; }
  int g72() const { return m_a; }
  int g73() const { return m_a; }
  int g74() const { return m_a; }
  int g75() const { return m_a; }
  int g76() const { return m_a; }
  int g77() const { return m_a; }
  int g78() const { return m_a; }
  int g79() const { return m_a; }
  int g80() const { return m_a; }
  int g81() const { return m_a; }
  int g82() const { return m_a; }
  int g83() const { return m_a; }
  int g84() const { return m_a; }
  int g85() const { return m_a; }
  int g86() const { return m_a; }
  int g87() const { return m_a; }
  int g88() const { return m_a; }
  int g89() const { return m_a; }
  int g90() const { return m_a; }
  int g91() const { return m_a; }
  int g92() const { return m_a; }
  int g93() const { return m_a; }
  int g94() const { return m_a; }
  int g95() const { return m_a; }
  int g96() const { return m_a; }
  int g97() const { return m_a; }
  int g98() const { return m_a; }
  int g99() const { return m_a; }
  int g100() const { return m_a; }
};

// 67 factories of 100 methods are more than two thirds, beside 33 get methods
// @stereotype factory controller lazy-class degenerate
class Factory100 {
  int m_a;
  int m_b;
public:
  Widget* f1() { return new Widget(); }
  Widget* f2() { return new Widget(); }
  Widget* f3() { return new Widget(); }
  Widget* f4() { return new Widget(); }
  Widget* f5() { return new Widget(); }
  Widget* f6() { return new Widget(); }
  Widget* f7() { return new Widget(); }
  Widget* f8() { return new Widget(); }
  Widget* f9() { return new Widget(); }
  Widget* f10() { return new Widget(); }
  Widget* f11() { return new Widget(); }
  Widget* f12() { return new Widget(); }
  Widget* f13() { return new Widget(); }
  Widget* f14() { return new Widget(); }
  Widget* f15() { return new Widget(); }
  Widget* f16() { return new Widget(); }
  Widget* f17() { return new Widget(); }
  Widget* f18() { return new Widget(); }
  Widget* f19() { return new Widget(); }
  Widget* f20() { return new Widget(); }
  Widget* f21() { return new Widget(); }
  Widget* f22() { return new Widget(); }
  Widget* f23() { return new Widget(); }
  Widget* f24() { return new Widget(); }
  Widget* f25() { return new Widget(); }
  Widget* f26() { return new Widget(); }
  Widget* f27() { return new Widget(); }
  Widget* f28() { return new Widget(); }
  Widget* f29() { return new Widget(); }
  Widget* f30() { return new Widget(); }
  Widget* f31() { return new Widget(); }
  Widget* f32() { return new Widget(); }
  Widget* f33() { return new Widget(); }
  Widget* f34() { return new Widget(); }
  Widget* f35() { return new Widget(); }
  Widget* f36() { return new Widget(); }
  Widget* f37() { return new Widget(); }
  Widget* f38() { return new Widget(); }
  Widget* f39() { return new Widget(); }
  Widget* f40() { return new Widget(); }
  Widget* f41() { return new Widget(); }
  Widget* f42() { return new Widget(); }
  Widget* f43() { return new Widget(); }
  Widget* f44() { return new Widget(); }
  Widget* f45() { return new Widget(); }
  Widget* f46() { return new Widget(); }
  Widget* f47() { return new Widget(); }
  Widget* f48() { return new Widget(); }
  Widget* f49() { return new Widget(); }
  Widget* f50() { return new Widget(); }
  Widget* f51() { return new Widget(); }
  Widget* f52() { return new Widget(); }
  Widget* f53() { return new Widget(); }
  Widget* f54() { return new Widget(); }
  Widget* f55() { return new Widget(); }
  Widget* f56() { return new Widget(); }
  Widget* f57() { return new Widget(); }
  Widget* f58() { return new Widget(); }
  Widget* f59() { return new Widget(); }
  Widget* f60() { return new Widget(); }
  Widget* f61() { return new Widget(); }
  Widget* f62() { return new Widget(); }
  Widget* f63() { return new Widget(); }
  Widget* f64() { return new Widget(); }
  Widget* f65() { return new Widget(); }
  Widget* f66() { return new Widget(); }
  Widget* f67() { return new Widget(); }
  int g68() const { return m_a; }
  int g69() const { return m_a; }
  int g70() const { return m_a; }
  int g71() const { return m_a; }
  int g72() const { return m_a; }
  int g73() const { return m_a; }
  int g74() const { return m_a; }
  int g75() const { return m_a; }
  int g76() const { return m_a; }
  int g77() const { return m_a; }
  int g78() const { return m_a; }
  int g79() const { return m_a; }
  int g80() const { return m_a; }
  int g81() const { return m_a; }
  int g82() const { return m_a; }
  int g83() const { return m_a; }
  int g84() const { return m_a; }
  int g85() const { return m_a; }
  int g86() const { return m_a; }
  int g87() const { return m_a; }
  int g88() const { return m_a; }
  int g89() const { return m_a; }
  int g90() const { return m_a; }
  int g91() const { return m_a; }
  int g92() const { return m_a; }
  int g93() const { return m_a; }
  int g94() const { return m_a; }
  int g95() const { return m_a; }
  int g96() const { return m_a; }
  int g97() const { return m_a; }
  int g98() const { return m_a; }
  int g99() const { return m_a; }
  int g100() const { return m_a; }
};

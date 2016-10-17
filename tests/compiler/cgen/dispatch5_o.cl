class A {
  f():Int {5};
};

class B inherits A {

};

class C inherits B {
  f():Int {6};
};

class Main inherits IO {
  a:A <- new A;
  b:A <- new B;
  c:A <- new C;
  bb:B <- new B;
  cc:C <- new C;
  main():SELF_TYPE {{
    out_int(a.f());      -- 5
    out_int(b.f());      -- 5
    out_int(c.f());      -- 6
    out_int(cc.f());     -- 6
    out_int(cc@C.f());   -- 6
    out_int(cc@B.f());   -- 5
    out_int(cc@A.f());   -- 5
    out_int(bb.f());     -- 5
    out_int(bb@B.f());   -- 5
    out_int(bb@A.f());   -- 5
    out_int(a@A.f());    -- 5
  }};
};

class A inherits IO {
  o : Object <- f();
  x : Int <- 12;
  f() : Object {{
    out_string("A.f: x = ");out_int(x);out_string("\n");
  }};
};

class B inherits A {
  y : Int <- {
    out_string("initializing B.y. here x = ");out_int(x);out_string("\n");
    13;};
  f() : Object {{
    out_string("B.f: y = ");out_int(y);out_string("\n");
  }};
};

class Main inherits IO {
  main() : Object {{
    out_string("new A\n");
    new A;
    out_string("new B\n");
    new B;
  }};
};

class Foo {
  f() : Object {
    (new IO).out_string("Hi from Foo_f\n")
  };
};

class Main {
  f : Foo;
  main() : Object {
    f.f()
  };
};

-- Tests dispatch on void

class A {
  f():Int {5};
};

class Main inherits IO {
  a:A;
  main():Int { a.f() };
};

-- A simple case test.

class A {};
class B inherits A {};
class B1 inherits B {};
class B2 inherits B {};
class E inherits A {};
class F inherits A {};

class Main inherits IO {
  main():Object {
    let 
      b:B <- new B,
      x:Object,
      result:Int
    in
        out_int(
          case b of
            x:A => 2;
            x:B1 => 3;
            x:E => 17;
          esac)
  };
};


class A {
  hello:String <- "Hello!\n";
  getGreetings():String { hello };
};

class Main inherits IO {

  main():Object {
    let
      a:A <- new A,
      copy_a:A
    in {
      copy_a <- a.copy();
      out_string(copy_a.getGreetings());
    }
  };

};

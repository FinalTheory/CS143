class A {
  a1:Int;
  a2:Bool;
  a3:String;
  a4:Int;
};

class AA inherits A {
  aa1:Bool;
  aa2:Int;
  set(x:Int):Int { aa2 <- x };
  get():Int { aa2 };
};

class Main inherits IO {

  main():SELF_TYPE {
    let
      a:A,
      copy_a:A
    in {
      a <- new AA;
      case a of
        aa:AA => aa.set(42);
        o:Object => out_string("Error1\n");
      esac;
      copy_a <- a.copy();
      case copy_a of
        aa:AA => out_int(aa.get());
        o:Object => out_string("Error2\n");
      esac;
      self;
    }
  };
};

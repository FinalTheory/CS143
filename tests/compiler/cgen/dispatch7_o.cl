class Foo{
  a():SELF_TYPE{ self };
};

class Bar inherits Foo{
  a():SELF_TYPE{ self };
  b(y:Int):Int{ 1 };
};

class Main inherits IO{
  main():Object{
    out_int((new Bar)@Foo.a().b(1))
  };
};

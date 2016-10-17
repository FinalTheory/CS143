class V
{
  s : String <- "init\n";
  set(x : String) : Object { s <- x };
  get() : String { s };
};

class Main inherits IO
{
  field : V;
  main() : Object
  {{
    field <- new V;
    let w : V <- new V in test(w);
    let w : V <- new V in { w.set("test\n"); test(w); };
    test(field);
  }};
  test(arg : V) : Object
  {{
    out_string(arg.get());
    arg.set("whoops\n");
    out_string(arg.get());
    out_string(field.get());
  }};
};

class MyIo inherits IO
{
  out_str : String;
  set_out_str(str : String) : Object
  {{
    out_str <- str;
  }};
  out() : SELF_TYPE
  {{
    out_string(out_str);
  }};
};

class Main
{
  io : MyIo;
  hello : String <- "Hello world.\n";
  main() : Object
  {{
    let new_io : MyIo <- new MyIo in io <- new_io;
    out(hello);
  }};
  out(str : String) : Object
  {{
    io.set_out_str(str);
    io.out();
  }};
};
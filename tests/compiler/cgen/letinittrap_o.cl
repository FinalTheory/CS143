class Main inherits IO {
  main(): Object {{
    let
      x:Bool <- true,
      x:Bool <- x
    in
      if x then out_int(1) else out_int(0) fi;
    self;
  }};
};

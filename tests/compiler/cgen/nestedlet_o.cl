class Main inherits IO {

  main(): Object {
    let
      x:Int <- 10,
      y:Int <- let x:Int <- 20 in x+2
    in
       out_int(x+y)
  };

};

--Test of attributes of type SELF_TYPE
(*Requires dispatch and new to work*)

class Alpha
{
   s:SELF_TYPE <- self;
   getNum() : Int {5};
};

class Beta inherits Alpha
{
   i:Int <- 10;
   getSelf() : SELF_TYPE {s};
   getNum() : Int {i};
};

class Main inherits IO
{
   b:Beta <- new Beta;
   main() : Object {{
     out_int(b.getSelf().getNum());
     self;
   }};
};

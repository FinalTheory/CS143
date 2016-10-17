--Test of attributes in another class
(*Requires dispatch and new to work*)

class Alpha
{
   x:Int<-67;
   getX() : Int {x};
};

class Main inherits IO
{
   x:Alpha<-new Alpha;
   main() : Object {{out_int(x.getX());self;}};
};

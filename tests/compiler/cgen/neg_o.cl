-- Test of primitive initialization with new

class Main inherits IO
{
   x:Int <- 10;

   main() : Object {{ x<-~x; out_int(x);self;}};
}; 

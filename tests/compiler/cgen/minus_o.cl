-- Test of primitive initialization with new

class Main inherits IO
{
   x:Int <- 10;
   y:Int <- 30;

   main() : Object {{ out_int(x-y);self;}};
}; 

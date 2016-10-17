--Test of attributes in main

class Main inherits IO
{
   x:Int<-247;
   main() : Object{{ out_int(x); self;}};
};

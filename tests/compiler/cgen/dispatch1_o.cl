-- simple method dispatch on self

class Main inherits IO
{
   niam() : Int {5};
   main() : Object {{ out_int(niam()); self;}};
}; 

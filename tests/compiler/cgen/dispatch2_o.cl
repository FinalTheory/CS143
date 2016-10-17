-- dispatch on another class

class Alpha
{
   niam() : Int {90};
};

class Main inherits IO
{
   func() : Int {(new Alpha).niam()};
   main() : Object {{ out_int(func()); self; }};
};

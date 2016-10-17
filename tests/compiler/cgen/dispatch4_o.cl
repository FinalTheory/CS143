-- static dispatch on another class

class Alpha inherits Beta
{
   niam() : Int {90};
};

class Beta
{
   niam() : Int {42};
   bugger() : Int {57};
};

class Main inherits IO
{
   func() : Int {(new Alpha)@Beta.niam()};
   main() : Object {{ out_int(func()); self; }};
};

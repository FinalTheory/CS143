--Test of IF

class Main inherits IO
{
   func() : Int {if true then if false then 4 else 8 fi else 6 fi};
   main() : Object {{ out_int(func()); self; }};
};

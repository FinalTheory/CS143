-- More complicated type case check

class Alpha inherits Beta
{
};

class Beta inherits Gamma
{
};

class Gamma {};

class Main inherits IO
{
   xyz:Gamma <- new Alpha;
   func() : Int
   {
      case xyz of
        b : Beta => 1;
        a : Alpha => 2;
        g : Gamma => 3;
        o : Object => 0;
      esac 
   };
 
   main() : Int {{
	out_int(func()); 0;
   }};
};

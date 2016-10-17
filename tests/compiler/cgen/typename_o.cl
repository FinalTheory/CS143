-- Typename test

(* Object.type_name returns a string representation of the dynamic type of the dispatching object.*)


class Base 
{
  identify( thing : Object , s : String) : String
  {
    {
      s.concat( thing.type_name() ).concat( "\n" );
    }
  };

  test(s : String) : String
  {
    {
      s<- s.concat(identify( new Base , s)).concat(identify( new Derived ,s)).concat(identify( new Main ,s));

      let poly : Base <- new Derived in
	  s<-identify( poly ,s );

      identify( self ,s);
    }
  };
};


class Derived inherits Base
{
};


class Main inherits IO
{
  func() : String
  {
    (new Derived).test("")
  };
  main() : Object {{
 	out_string(func());
	self;
  }};
};

-- Basic class built in functions

class Main inherits IO
{
   main() : Object
   {{
      out_string(type_name());
      out_int(879);
      out_string(type_name().concat("Frugel"));
      out_int(type_name().length());
      out_string(type_name().substr(2,2));
	  self;
   }};
};

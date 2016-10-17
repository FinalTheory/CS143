-- Tests string constants

class Main inherits IO
{
   func() : String {"Yoyoyo"};
   main() : Object {{
		out_string(func()); self;
   }};
};

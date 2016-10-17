-- Test of assignment, and some recursion

class Main inherits IO {

   foo(x:Int) : Int {
		if x < 10 then {
	    	x <- x+foo(x+1);
			x + 1;
		}
		else
			1
		fi
   };

   x : Int <- 0;
   main() : Object {{
     x <- foo(4);
	 out_int(x);
     self;
   }};
};
   

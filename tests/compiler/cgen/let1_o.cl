-- Simple let test

class Main inherits IO
{
    main() : Object {{
		out_int(let x:Int<-798 in x);
		self;
	}};
};

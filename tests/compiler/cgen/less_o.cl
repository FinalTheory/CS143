-- Less than test

class Main inherits IO
{
	func():Bool {2<5};
	main():Object {{ 
		if func() then out_int(1) else out_int(0) fi;
 		self;
	}};
};

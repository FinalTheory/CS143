-- Multiple let test, with LT

class Main inherits IO
{
    func() : Bool {let x:Int<-798, y:Bool<-true, z:Int<-5, x:Int<-100 in z<x};
	main() : Object {{
		if func() then out_int(1) else out_int(0) fi;
		self;
	}};
};

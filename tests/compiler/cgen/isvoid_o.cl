-- Test of isvoid

class Main inherits IO
{
  x:Main;
  func() : Bool {isvoid x};
  main() : Object {{
	if func() then out_string("ok") else out_string("not ok") fi;
	self;
  }};
};

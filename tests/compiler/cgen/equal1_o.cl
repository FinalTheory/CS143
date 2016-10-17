-- Equality test

class Main inherits IO
{
  func():Bool {2=5};
  main():SELF_TYPE { 
	  if func() then out_string("not ok!\n") else out_string("ok!\n") fi
	};
};

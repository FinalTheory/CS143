-- Boolean not test

class Main inherits IO
{
  func():Bool {not false};
  main():Object {{
	if func() then out_int(1) else out_int(0) fi;
	self;
  }};
};

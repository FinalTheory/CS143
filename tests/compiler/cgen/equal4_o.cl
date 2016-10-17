-- Tests equality on void objects

class A {
  a:Int <- 2;
};

class B {
  b:Int <- 3;
};

class Main inherits IO
{
   a1:A <- new A;
   a2:A;
   a3:A;
   b:B;
   main() : SELF_TYPE {
     if a1 = a2 then
       out_string("not ok!\n")
     else
       if a2 = a3 then
         if b = a2 then
           out_string("ok!\n")
         else
           out_string("not ok!\n")
         fi
       else
         out_string("not ok!\n")
       fi
     fi
   };
};

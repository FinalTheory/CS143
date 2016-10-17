-- Tests equality on objects

class A {
  a:Int <- 2;
};

class Main inherits IO
{
   a1:A <- new A;
   a2:A <- new A;
   a3:A <- a2; 
   main() : SELF_TYPE {
     if a1 = a2 then
       out_string("not ok!\n")
     else
       if a2 = a3 then
         out_string("ok!\n")
       else
         out_string("not ok!\n")
       fi
     fi
   };
};

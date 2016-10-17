-- Tests equality on strings

class Main inherits IO
{
   main() : SELF_TYPE {
    if "s1" = "s1" then out_string("ok!\n") else out_string("not ok!\n") fi
   };
};

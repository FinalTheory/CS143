-- Tests self in another class
(* requires dispatch*)

class Alpha
{
   getSelf() : Alpha {self};
};

class Main
{
   main() : Alpha {(new Alpha).getSelf()};
};

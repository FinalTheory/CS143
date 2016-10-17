-- test of SELF_TYPE and new

class A
{
    me() : SELF_TYPE { new SELF_TYPE };
};

class B inherits A
{
};

class Main
{
   main() : String {(new B).me().type_name()};
};

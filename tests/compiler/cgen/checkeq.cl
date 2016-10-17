class Main inherits IO
{
  obj:Object;
  main(): Object {
        { if self = obj
            then out_string ("self = obj\n")
            else out_string ("self != obj\n")
          fi;
          if 1 = 0
            then out_string ("1 = 0\n")
            else out_string ("1 != 0\n")
          fi;
          if true = false
            then out_string ("true = false\n")
            else out_string ("true != false\n")
          fi;
          if "abc" = "bbc"
            then out_string ("\"abc\" = \"bbc\"\n")
            else out_string ("\"abc\" != \"bbc\"\n")
          fi;
      }
  };
};

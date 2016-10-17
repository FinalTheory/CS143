-- A simple case test.

class A {};
class B inherits A {};
class C inherits B {};
class D inherits C {};
class E inherits D {};
class F inherits E {};
class G inherits F {};
class H inherits G {};
class I inherits H {};
class J inherits I {};
class K inherits J {};
class L inherits K {};
class M inherits L {};
class N inherits M {};
class O inherits N {};
class P inherits O {};
class Q inherits P {};

class Main inherits IO {
  main():Object {
    let 
      i:Int <- 0,
      x:Int <- 0,
      a:Object <- new A,
      q:Object <- new Q,
      result:Int
    in {
      while i < 100 loop {
        result <-
          case q of
            x:A => 1;
            x:B => 2;
            x:C => 3;
            x:D => 4;
            x:E => 5;
            x:F => 6;
            x:G => 7;
            x:H => 8;
            x:I => 9;
            x:J => 10;
            x:K => 11;
            x:L => 12;
            x:M => 13;
            x:N => 14;
            x:O => 15;
            x:P => 16;
            x:Q => 17;
          esac;
        i <- i + 1;
      } pool;
      if not result = 17 then x <- x + 1 else 0 fi;

      i <- 0;
      while i < 10 loop {
        result <-
          case a of
            x:A => 1;
            x:B => 2;
            x:C => 3;
            x:D => 4;
            x:E => 5;
            x:F => 6;
            x:G => 7;
            x:H => 8;
            x:I => 9;
            x:J => 10;
            x:K => 11;
            x:L => 12;
            x:M => 13;
            x:N => 14;
            x:O => 15;
            x:P => 16;
            x:Q => 17;
          esac;
        i <- i + 1;
      } pool;
      if not result = 1 then x <- x + 1 else 0 fi;
      out_int(x);
	  self;
    }
  };
};





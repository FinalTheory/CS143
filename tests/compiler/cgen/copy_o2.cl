class A {
  answerToAll:Int <- 42;
  getAnswerToAll():Int { answerToAll };
  setAnswerToAll(myAns:Int):SELF_TYPE {{ answerToAll <- myAns; self; }};
};

class Main inherits IO {

  main():Object {
    let
      a:A <- new A,
      copy_a:A
    in {
      copy_a <- a.copy();
      copy_a.setAnswerToAll(21);
      out_int(a.getAnswerToAll());
      out_string(" ");
      out_int(copy_a.getAnswerToAll());
    }
  };

};

let tap = (f, x) => {
  f(x);
  x;
};
<<<<<<< HEAD
=======

let rec repeat = (~count, f) =>
  if (count > 0) {
    f();
    repeat(~count=count - 1, f);
  };
>>>>>>> 3151b18315c6d79f1249cc61c606389d229e947f

let tap = (f, x) => {
  f(x);
  x;
};

let rec repeat = (~count, f) =>
  if (count > 0) {
    f();
    repeat(~count=count - 1, f);
  };

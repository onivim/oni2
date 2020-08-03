module Zed_utf8 = ZedBundled;

let repeatInput = (reps, input) => {
  let rec loop = (reps, context) =>
    if (reps > 0) {
      let context = Vim.input(~context, input);
      loop(reps - 1, context);
    } else {
      context;
    };

  loop(reps, Vim.Context.current());
};

let inputString = input => {
  Vim.input(~context=Vim.Context.current(), input);
};

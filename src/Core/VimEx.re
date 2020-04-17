module Zed_utf8 = ZedBundled;

let repeatInput = (reps, input) => {
  let rec loop = (reps, context) =>
    if (reps > 0) {
      let (context, _eff) = Vim.input(~context, input);
      loop(reps - 1, context);
    } else {
      context;
    };

  loop(reps, Vim.Context.default());
};

let inputString = input =>
  Zed_utf8.fold(
    (char, context) => {
      let (context, _eff) = Vim.input(~context, Zed_utf8.singleton(char));
      context;
    },
    input,
    Vim.Context.default(),
  );

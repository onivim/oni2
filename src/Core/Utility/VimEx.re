module Zed_utf8 = Kernel.ZedBundled;

let repeatInput = (reps, input) => {
  let rec loop = (reps, cursors) =>
    if (reps > 0) {
      loop(reps - 1, Vim.input(input));
    } else {
      cursors;
    };

  loop(reps, []);
};

let inputString = input =>
  Zed_utf8.fold(
    (char, _) => Vim.input(Zed_utf8.singleton(char)),
    input,
    [],
  );

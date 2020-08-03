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

let inputString = input =>
  Zed_utf8.fold(
    (char, context) => {
      let str = Zed_utf8.singleton(char);
      prerr_endline ("INPUTTING: " ++ str);
      prerr_endline ("CHAR CODE: " ++ string_of_int(Uchar.to_int(char)));
      let context = Vim.input(~context, Zed_utf8.singleton(char));
      context;
    },
    input,
    Vim.Context.current(),
  );

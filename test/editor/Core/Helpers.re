exception OptionInvalidException(string);
let getOrThrow: option('a) => 'a =
  v =>
    switch (v) {
    | Some(v) => v
    | None => raise(OptionInvalidException("Excepted 'Some' but got 'None'"))
    };

let repeat = (~iterations: int=5, f) => {
  let count = ref(0);

  while (count^ < iterations) {
    f();
    count := count^ + 1;
  };
};


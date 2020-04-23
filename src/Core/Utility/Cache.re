let create = (~initialSize, f) => {
  let cache = Hashtbl.create(initialSize);

  input =>
    switch (Hashtbl.find_opt(cache, input)) {
    | Some(output) => output
    | None =>
      let output = f(input);
      Hashtbl.add(cache, input, output);
      output;
    };
};

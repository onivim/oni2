open Rench;

exception EnvironmentVariableNotFound;

let optOrThrow = (s: option(string)) => {
  switch (s) {
  | Some(v) => v
  | _ => raise(EnvironmentVariableNotFound)
  };
};

let getNeovimPath = () =>
  Environment.getEnvironmentVariable("ONI2_NEOVIM_PATH") |> optOrThrow;

let repeat = (times: int, f) => {
  let count = ref(0);

  while (count^ < times) {
    f();
    count := count^ + 1;
  };
};

let exists = filePath =>
  try(Unix.stat(filePath).st_kind == S_REG) {
  | Unix.Unix_error(_) => false
  };

let readAllLines = filePath => {
  let lines = ref([]);

  let channel = open_in(filePath);
  try(
    {
      while (true) {
        let line = input_line(channel);
        lines := [line, ...lines^];
      };
      lines^ |> List.rev;
    }
  ) {
  | End_of_file =>
    close_in(channel);
    lines^ |> List.rev;
  };
};

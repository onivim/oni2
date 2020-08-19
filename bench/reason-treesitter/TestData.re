let dir = Filename.dirname(Sys.argv[0]);

let read_file = filename => {
  let lines = ref([]);
  let chan = open_in(filename);
  try(
    {
      while (true) {
        // print_endline("Reading line...");
        lines := [input_line(chan), ...lines^];
                                                // print_endline("Line read!");
      };
      lines^;
    }
  ) {
  | End_of_file =>
    close_in(chan);
    print_endline("EOF");
    List.rev(lines^);
  };
};

print_endline("Loading test data prior to benchmark...");
let largeJson = read_file(dir ++ "/" ++ "canada.json");
let largeJsonString = largeJson |> String.concat("\n");
let largeJsonArray = largeJson |> Array.of_list;

let largeC = read_file(dir ++ "/" ++ "sqlite3.c");
let largeCString = largeC |> String.concat("\n");
let largeCArray = largeC |> Array.of_list;
print_endline("Finished loading.");

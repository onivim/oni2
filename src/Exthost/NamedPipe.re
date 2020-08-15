type t = string;

let create = {
  let lastId = ref(0);
  pipeName => {
    incr(lastId);
    let id = lastId^ |> string_of_int;
    if (Sys.win32) {
      Printf.sprintf("\\\\.\\pipe\\%s%s", pipeName, id);
    } else {
      let name = Filename.temp_file("exthost-", "-sock" ++ id);
      Unix.unlink(name);
      name;
    };
  };
};

let toString = v => v;

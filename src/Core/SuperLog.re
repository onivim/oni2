let pid = Luv.Pid.getpid() |> string_of_int;
let logPath = "/Users/bryphe/log-file-" ++ (Luv.Pid.getpid() |> string_of_int) ++ ".txt";

let start = () => {
  let oc = open_out(logPath);
   Printf.fprintf(oc,"%s\n", "Starting log file: " ++ pid);
   close_out(oc);
};

let write = (msg) => {
    let oc = open_out_gen([Open_append], 0o600, logPath);
   Printf.fprintf(oc, "%s\n", msg);
   close_out(oc);
};

start();

Printexc.record_backtrace(true);
  Printexc.set_uncaught_exception_handler((e, bt) => {
    write(Printf.sprintf(
        "Exception %s:\n%s",
        Printexc.to_string(e),
        Printexc.raw_backtrace_to_string(bt),
      )
    );
    flush_all();
  });


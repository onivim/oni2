/*
 * Log.re
 *
 * Simple console logger
 */

let canPrint = ref(false);
let debugLogging = ref(false);

let enablePrinting = () => {
  canPrint := true;
};

let enableDebugLogging = () => {
  debugLogging := true;
};

let isDebugLoggingEnabled = () => debugLogging^;

let print = msg => canPrint^ ? print_endline(msg) : ();

let prerr = msg => canPrint^ ? prerr_endline(msg) : ();

let fileChannel =
  switch (Sys.getenv_opt("ONI2_LOG_FILE")) {
  | Some(v) =>
    let oc = open_out(v);
    Printf.fprintf(oc, "Starting log file.\n");
    flush(oc);
    Some(oc);
  | None => None
  };

let _ =
  switch (Sys.getenv_opt("ONI2_DEBUG"), Sys.getenv_opt("ONI2_LOG_FILE")) {
  | (Some(_), _) => debugLogging := true
  | (_, Some(_)) => debugLogging := true
  | _ => ()
  };

let logCore = (~error=false, msg) => {
  switch (fileChannel) {
  | Some(oc) =>
    Printf.fprintf(oc, "%s\n", msg);
    flush(oc);
  | None =>
    switch (error) {
    | false => print(msg)
    | true => prerr(msg)
    }
  };
};

let info = msg => logCore("[INFO] " ++ msg);

let debug = msg => debugLogging^ ? logCore("[DEBUG] " ++ msg) : ();

let error = msg => logCore(~error=true, "[ERROR] " ++ msg);

let perf = (msg, f) => {
  let startTime = Unix.gettimeofday();
  f();
  let endTime = Unix.gettimeofday();
  logCore(
    "[PERF] "
    ++ msg
    ++ " took "
    ++ string_of_float(endTime -. startTime)
    ++ "s",
  );
};

let () =
  switch (debugLogging^) {
  | false => ()
  | true =>
    debug("Recording backtraces");
    Printexc.record_backtrace(true);
    Printexc.set_uncaught_exception_handler((e, bt) => {
      error(
        "Exception "
        ++ Printexc.to_string(e)
        ++ ":\n"
        ++ Printexc.raw_backtrace_to_string(bt),
      );
      flush_all();
    });
  };

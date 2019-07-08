/*
 * Log.re
 *
 * Simple console logger
 */

let canPrint = ref(false);

let enablePrinting = () => {
  canPrint := true;
};

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

let isDebugLoggingEnabled =
  switch (Sys.getenv_opt("ONI2_DEBUG"), Sys.getenv_opt("ONI2_LOG_FILE")) {
  | (Some(_), _) => true
  | (_, Some(_)) => true
  | _ => false
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

let debug = msg => isDebugLoggingEnabled ? logCore("[DEBUG] " ++ msg) : ();

let error = msg => logCore(~error=true, "[ERROR] " ++ msg);

let () =
  switch (isDebugLoggingEnabled) {
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

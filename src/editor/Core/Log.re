/*
 * Log.re
 *
 * Simple console logger
 */

let canPrint = ref(false);

let enablePrinting = () => canPrint := true;

let print = msg => switch (canPrint^) {
| true => print_endline (msg);
| false => ();
};

let prerr = msg => switch (canPrint^) {
| true => prerr_endline (msg);
| false => ();
};

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

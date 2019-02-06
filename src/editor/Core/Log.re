/*
 * Log.re
 *
 * Simple console logger
 */

let isDebugLoggingEnabled = switch(Sys.getenv_opt("ONI2_DEBUG")) {
| Some(_) => true
| _ => false
}


let info = (msg) => print_endline(["[INFO] " ++ msg);

let debug = (msg) => isDebugLoggingEnabled ? print_endline("[DEBUG] " ++ msg);

let error = (msg) => prerr_endline("[ERROR] " ++ msg);

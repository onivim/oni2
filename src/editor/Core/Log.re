/*
 * Log.re
 *
 * Simple console logger
 */

open Types;

let isDebugLoggingEnabled = switch(Sys.getenv_opt("ONI2_DEBUG")) {
| Some(_) => true
| _ => false
}


let info = (msg) => print_endline(msg);

let error = (msg) => isDebugLoggingEnabled ? prerr_endline(msg) : ();

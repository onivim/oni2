include Timber.Log;

module type Logger = Timber.Logger;

module Env = {
  let logFile = Sys.getenv_opt("ONI2_LOG_FILE");
  let debug = Sys.getenv_opt("ONI2_DEBUG");
  let filter = Sys.getenv_opt("ONI2_LOG_FILTER");
};

switch (Env.debug, Env.logFile) {
| (None, None) => Logs.set_level(Some(Logs.Info))
| _ => Logs.set_level(Some(Logs.Debug))
};

Env.logFile |> Option.iter(Timber.App.setLogFile);
Env.filter |> Option.iter(Timber.App.setNamespaceFilter);

let writeExceptionLog = (e, bt) => {
  let oc = Stdlib.open_out("onivim2-crash.log");
  Printf.fprintf(
    oc,
    "%s:\n%s",
    Printexc.to_string(e),
    Printexc.raw_backtrace_to_string(bt),
  );
  Stdlib.close_out(oc);
};

let enableDebug = () => {
  Timber.App.setLevel(Timber.Level.debug);
  module Log = (val withNamespace("Oni2.Exception"));

  Log.debug("Recording backtraces");
  Printexc.record_backtrace(true);
  Printexc.set_uncaught_exception_handler((e, bt) => {
    Log.errorf(m =>
      m(
        "Exception %s:\n%s",
        Printexc.to_string(e),
        Printexc.raw_backtrace_to_string(bt),
      )
    );
    flush_all();
    writeExceptionLog(e, bt);
  });
};

let enableTrace = () => {
  enableDebug();
  Timber.App.setLevel(Timber.Level.trace);
};

let enableQuiet = () => {
  Timber.App.setLevel(Timber.Level.error);
};

if (Timber.App.isLevelEnabled(Timber.Level.debug)) {
  enableDebug();
} else {
  // Even if we're not debugging.... at least emit the exception
  Printexc.set_uncaught_exception_handler((e, bt) => {
    writeExceptionLog(e, bt)
  });
};

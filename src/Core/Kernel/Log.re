include Timber.Log;

module type Logger = Timber.Logger;

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

let init = () =>
  if (Timber.App.isLevelEnabled(Timber.Level.debug)) {
    enableDebug();
  } else {
    // Even if we're not debugging.... at least emit the exception
    Printexc.set_uncaught_exception_handler((e, bt) => {
      writeExceptionLog(e, bt)
    });
  };

/*
 * Log.re
 *
 * Simple console logger
 */

let fileChannel =
  switch (Sys.getenv_opt("ONI2_LOG_FILE")) {
  | Some(v) =>
    let oc = open_out(v);
    Printf.fprintf(oc, "Starting log file.\n");
    flush(oc);
    Some(oc);
  | None => None
  };

let fileReporter =
  switch (Sys.getenv_opt("ONI2_LOG_FILE")) {
  | Some(path) =>
    let channel = open_out(path);
    let ppf = Format.formatter_of_out_channel(channel);
    Printf.fprintf(channel, "Starting log file.\n%!");

    Logs.{
      report: (_src, level, ~over, k, msgf) => {
        let k = _ => {
          over();
          k();
        };

        msgf((~header=?, ~tags as _=?, fmt) => {
          Format.kfprintf(
            k,
            ppf,
            "%a@[" ^^ fmt ^^ "@]@.",
            pp_header,
            (level, header),
          )
        });
      },
    };

  | None => Logs.nop_reporter
  };

let consoleReporter = Logs_fmt.reporter(~pp_header=Logs_fmt.pp_header, ());

let reporter =
  Logs.{
    report: (src, level, ~over, k, msgf) => {
      let kret = consoleReporter.report(src, level, ~over=() => (), k, msgf);
      fileReporter.report(src, level, ~over, () => kret, msgf);
    },
  };

// defaults
let () = {
  Fmt_tty.setup_std_outputs(~style_renderer=`Ansi_tty, ());
  switch (Sys.getenv_opt("ONI2_DEBUG"), Sys.getenv_opt("ONI2_LOG_FILE")) {
  | (Some(_), _)
  | (_, Some(_)) => Logs.set_level(Some(Logs.Debug))
  | _ => Logs.set_level(Some(Logs.Info))
  };
};

let enablePrinting = () => Logs.set_reporter(reporter);
let isPrintingEnabled = () => Logs.reporter() === Logs.nop_reporter;

let enableDebugLogging = () =>
  Logs.Src.set_level(Logs.default, Some(Logs.Debug));
let isDebugLoggingEnabled = () =>
  Logs.Src.level(Logs.default) == Some(Logs.Debug);

let infof = msgf => Logs.info(msgf);
let info = msg => infof(m => m("%s", msg));
let debugf = msgf => Logs.debug(msgf);
let debug = msgf => debugf(m => m("%s", msgf()));
let errorf = msgf => Logs.err(msgf);
let error = msg =>errorf(m => m("%s", msg));

let perf = (msg, f) => {
  let startTime = Unix.gettimeofday();
  let ret = f();
  let endTime = Unix.gettimeofday();
  debug(() =>
    "[PERF] "
    ++ msg
    ++ " took "
    ++ string_of_float(endTime -. startTime)
    ++ "s"
  );
  ret;
};

let () =
  switch (isDebugLoggingEnabled()) {
  | false => ()
  | true =>
    debug(() => "Recording backtraces");
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

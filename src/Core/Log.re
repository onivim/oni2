/*
 * Log.re
 *
 * Simple console logger
 */

// TODO: Remove after 4.08
module Option = {
  let bind = (o, f) =>
    switch (o) {
    | Some(x) => f(x)
    | None => None
    };

  let iter = f =>
    fun
    | Some(x) => f(x)
    | None => ();
};

module Constants = {
  let colors =
    Fmt.(
      [|
        // `Black
        `Blue,
        `Cyan,
        `Green,
        `Magenta,
        `Red,
        //`White,
        `Yellow,
        `Hi(`Black),
        `Hi(`Blue),
        `Hi(`Cyan),
        `Hi(`Green),
        `Hi(`Magenta),
        `Hi(`Red),
        `Hi(`White),
        `Hi(`Yellow),
      |]
    );

  module Env = {
    let logFile = "ONI2_LOG_FILE";
    let debug = "ONI2_DEBUG";
  };
};

type msgf('a, 'b) = (format4('a, Format.formatter, unit, 'b) => 'a) => 'b;

let pickColor = i => Constants.colors[i mod Array.length(Constants.colors)];

let namespaceTag = Logs.Tag.def("namespace", Format.pp_print_string);

let pp_namespace = ppf =>
  Option.iter(namespace => {
    let color = pickColor(Hashtbl.hash(namespace));
    let style = Fmt.(styled(`Fg(color), string));

    Fmt.pf(ppf, "[%a]", style, namespace);
  });

let fileReporter =
  switch (Sys.getenv_opt(Constants.Env.logFile)) {
  | Some(path) =>
    let channel = open_out(path);
    Printf.fprintf(channel, "Starting log file.\n%!");
    let ppf = Format.formatter_of_out_channel(channel);

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

let consoleReporter =
  Logs.{
    report: (_src, level, ~over, k, msgf) => {
      let k = _ => {
        over();
        k();
      };

      msgf((~header=?, ~tags=?, fmt) => {
        let namespace = Option.bind(tags, Logs.Tag.find(namespaceTag));

        Format.kfprintf(
          k,
          Format.err_formatter,
          "%a%a@[" ^^ fmt ^^ "@]@.",
          Logs_fmt.pp_header,
          (level, header),
          pp_namespace,
          namespace,
        );
      });
    },
  };

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
  switch (
    Sys.getenv_opt(Constants.Env.debug),
    Sys.getenv_opt(Constants.Env.logFile),
  ) {
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

let infof = msgf => Logs.info(m => msgf(m(~header=?None, ~tags=?None)));
let info = msg => infof(m => m("%s", msg));

let debugf = msgf => Logs.debug(m => msgf(m(~header=?None, ~tags=?None)));
let debug = msgf => debugf(m => m("%s", msgf()));

let errorf = msgf => Logs.err(m => msgf(m(~header=?None, ~tags=?None)));
let error = msg => errorf(m => m("%s", msg));

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

module type Logger = {
  let infof: msgf(_, unit) => unit;
  let info: string => unit;
};

let withNamespace = namespace => {
  let namespace = Logs.Tag.(empty |> add(namespaceTag, namespace));

  (
    (module
     {
       let infof = msgf =>
         Logs.info(m => msgf(m(~header=?None, ~tags=namespace)));
       let info = msg => infof(m => m("%s", msg));
     }): (module Logger)
  );
};

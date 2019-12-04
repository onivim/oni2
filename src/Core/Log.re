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
  let colors = [|
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
  |];
};

module Env = {
  let logFile = Sys.getenv_opt("ONI2_LOG_FILE");
  let debug = Sys.getenv_opt("ONI2_DEBUG");
  let filter = Sys.getenv_opt("ONI2_LOG_FILTER");
};

module Namespace = {
  let pickColor = i => Constants.colors[i mod Array.length(Constants.colors)];

  let tag = Logs.Tag.def("namespace", Format.pp_print_string);

  let pp = ppf =>
    Option.iter(namespace => {
      let color = pickColor(Hashtbl.hash(namespace));
      let style = Fmt.(styled(`Fg(color), string));

      Fmt.pf(ppf, "%a :", style, namespace);
    });

  let includes = ref([]);
  let excludes = ref([]);

  let isEnabled = namespace => {
    let test = re => Re.execp(re, namespace);
    let included = includes^ == [] || List.exists(test, includes^);
    let excluded = List.exists(test, excludes^);

    included && !excluded;
  };

  let setFilter = filter => {
    let filters =
      filter |> String.split_on_char(',') |> List.map(String.trim);

    let (incs, excs) =
      filters
      |> List.fold_left(
           ((includes, excludes), filter) =>
             if (filter == "") {
               (includes, excludes);
             } else if (filter.[0] == '-') {
               let filter =
                 String.sub(filter, 1, String.length(filter) - 2)
                 |> Re.Glob.glob
                 |> Re.compile;

               (includes, [filter, ...excludes]);
             } else {
               let filter = filter |> Re.Glob.glob |> Re.compile;

               ([filter, ...includes], excludes);
             },
           ([], []),
         );

    includes := incs;
    excludes := excs;
  };
};

type msgf('a, 'b) = (format4('a, Format.formatter, unit, 'b) => 'a) => 'b;

let logFileChannel = ref(None);

let setLogFile = path => {
  Option.iter(close_out, logFileChannel^);
  let channel = open_out(path);
  Printf.fprintf(channel, "Starting log file.\n%!");
  logFileChannel := Some(channel);
};

let fileReporter =
  Logs.{
    report: (_src, level, ~over, k, msgf) => {
      let k = _ => {
        over();
        k();
      };

      switch (logFileChannel^) {
      | Some(channel) =>
        let ppf = Format.formatter_of_out_channel(channel);

        msgf((~header=?, ~tags=?, fmt) => {
          let namespace = Option.bind(tags, Tag.find(Namespace.tag));

          Format.kfprintf(
            k,
            ppf,
            "%a %a @[" ^^ fmt ^^ "@]@.",
            pp_header,
            (level, header),
            Namespace.pp,
            namespace,
          );
        });

      | None => k()
      };
    },
  };

let consoleReporter = {
  let pp_level = (ppf, level) => {
    let str =
      switch (level) {
      | Logs.App => ""
      | Logs.Error => "ERROR"
      | Logs.Warning => "WARN"
      | Logs.Info => "INFO"
      | Logs.Debug => "DEBUG"
      };

    Fmt.pf(ppf, "%-7s", "[" ++ str ++ "]");
  };

  let pp_level_styled = (ppf, level) => {
    let (fg, bg) =
      switch (level) {
      | Logs.App => (`White, `Cyan)
      | Logs.Error => (`White, `Red)
      | Logs.Warning => (`White, `Yellow)
      | Logs.Info => (`Black, `Blue)
      | Logs.Debug => (`Black, `Green)
      };

    let style = Fmt.(styled(`Fg(fg), styled(`Bg(bg), pp_level)));
    Fmt.pf(ppf, "%7a", style, level);
  };

  Logs.{
    report: (_src, level, ~over, k, msgf) => {
      let k = _ => {
        over();
        k();
      };

      msgf((~header as _=?, ~tags=?, fmt) => {
        let namespace = Option.bind(tags, Tag.find(Namespace.tag));

        Format.kfprintf(
          k,
          Format.err_formatter,
          "%a %a @[" ^^ fmt ^^ "@]@.",
          pp_level_styled,
          level,
          Namespace.pp,
          namespace,
        );
      });
    },
  };
};

let reporter =
  Logs.{
    report: (src, level, ~over, k, msgf) => {
      let kret = consoleReporter.report(src, level, ~over=() => (), k, msgf);
      fileReporter.report(src, level, ~over, () => kret, msgf);
    },
  };

let enablePrinting = () => Logs.set_reporter(reporter);
let isPrintingEnabled = () => Logs.reporter() === Logs.nop_reporter;

let enableDebugLogging = () =>
  Logs.Src.set_level(Logs.default, Some(Logs.Debug));
let isDebugLoggingEnabled = () =>
  Logs.Src.level(Logs.default) == Some(Logs.Debug);

let log = (~namespace="Global", level, msgf) =>
  Logs.msg(level, m =>
    if (Namespace.isEnabled(namespace)) {
      let tags = Logs.Tag.(empty |> add(Namespace.tag, namespace));
      msgf(m(~header=?None, ~tags));
    }
  );

let info = msg => log(Logs.Info, m => m("%s", msg));
let debug = msgf => log(Logs.Debug, m => m("%s", msgf()));
let error = msg => log(Logs.Error, m => m("%s", msg));

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

module type Logger = {
  let errorf: msgf(_, unit) => unit;
  let error: string => unit;
  let warnf: msgf(_, unit) => unit;
  let warn: string => unit;
  let infof: msgf(_, unit) => unit;
  let info: string => unit;
  let debugf: msgf(_, unit) => unit;
  let debug: string => unit;
};

let withNamespace = namespace => {
  let logf = (level, msgf) => log(~namespace, level, msgf);
  let log = (level, msg) => logf(level, m => m("%s", msg));

  (
    (module
     {
       let errorf = msgf => logf(Logs.Error, msgf);
       let error = log(Logs.Error);
       let warnf = msgf => logf(Logs.Warning, msgf);
       let warn = log(Logs.Warning);
       let infof = msgf => logf(Logs.Info, msgf);
       let info = log(Logs.Info);
       let debugf = msgf => logf(Logs.Debug, msgf);
       let debug = log(Logs.Debug);
     }): (module Logger)
  );
};

// init
let () = Fmt_tty.setup_std_outputs(~style_renderer=`Ansi_tty, ());

switch (Env.debug, Env.logFile) {
| (None, None) => Logs.set_level(Some(Logs.Info))
| _ => Logs.set_level(Some(Logs.Debug))
};

Env.logFile |> Option.iter(setLogFile);
Env.filter |> Option.iter(Namespace.setFilter);

if (isDebugLoggingEnabled()) {
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

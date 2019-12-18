type msgf('a, 'b) = (format4('a, Format.formatter, unit, 'b) => 'a) => 'b;

let disableColors: unit => unit;

let enablePrinting: unit => unit;
let isPrintingEnabled: unit => bool;

let enableDebugLogging: unit => unit;
let isDebugLoggingEnabled: unit => bool;

let setLogFile: string => unit;

let info: string => unit;
let debug: (unit => string) => unit;
let error: string => unit;
let perf: (string, unit => 'a) => 'a;

module Namespace: {
  let isEnabled: string => bool;

  /**
   * setFilter(filters)
   *
   * where `filter` is a comma-separated list of glob patterns to include,
   * optionally prefixed with a `-` to negate it. A blank string includes
   * everything, and is the default.
   *
   * E.g.
   *   setFilter("Oni2.*, -Revery*")
   */
  let setFilter: string => unit;
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

let withNamespace: string => (module Logger);

type msgf('a, 'b) = (format4('a, Format.formatter, unit, 'b) => 'a) => 'b;

let enablePrinting: unit => unit;
let isPrintingEnabled: unit => bool;

let enableDebugLogging: unit => unit;
let isDebugLoggingEnabled: unit => bool;

let info: string => unit;
let debug: (unit => string) => unit;
let error: string => unit;
let perf: (string, unit => 'a) => 'a;

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

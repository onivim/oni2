let isPrintingEnabled: unit => bool;
let isDebugLoggingEnabled: unit => bool;

let perf: (string, unit => 'a) => 'a;
let fn: (string, 'a => 'b, 'a) => 'b;

module type Logger = {
  let errorf: Timber.msgf(_, unit) => unit;
  let error: string => unit;
  let warnf: Timber.msgf(_, unit) => unit;
  let warn: string => unit;
  let infof: Timber.msgf(_, unit) => unit;
  let info: string => unit;
  let debugf: Timber.msgf(_, unit) => unit;
  let debug: string => unit;
  let fn: (string, 'a => 'b, 'a) => 'b;
};

let withNamespace: string => (module Logger);

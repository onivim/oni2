type msgf('a, 'b) = (format4('a, Format.formatter, unit, 'b) => 'a) => 'b;

let enablePrinting: unit => unit;
let isPrintingEnabled: unit => bool;

let enableDebugLogging: unit => unit;
let isDebugLoggingEnabled: unit => bool;

let infof: msgf('a, unit) => unit;
let info: string => unit;

let debugf: msgf('a, unit) => unit;
let debug: (unit => string) => unit;

let errorf: msgf('a, unit) => unit;
let error: string => unit;

let perf: (string, unit => 'a) => 'a;

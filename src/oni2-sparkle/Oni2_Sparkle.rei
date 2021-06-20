let init: unit => unit;
let version: option(string);

module Updater: {
  type t;

  let getInstance: unit => t;

  let setFeedURL: (t, string) => unit;
  let getFeedURL: t => string;

  let setAutomaticallyChecksForUpdates: (t, bool) => unit;
  let getAutomaticallyChecksForUpdates: t => bool;

  let checkForUpdates: t => unit;
};

module Debug: {
  let toString: _ => string;
  let log: _ => unit;
};

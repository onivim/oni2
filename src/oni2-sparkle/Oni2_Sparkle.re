%import
"config.h";

%ifdef
ENABLE_AUTOUPDATE;

external init: unit => unit = "oni2_SparkleInit";

external c_version: unit => string = "oni2_SparkleVersion";

let version = Some(c_version());

module Updater = {
  type t;

  external getInstance: unit => t = "oni2_SparkleGetSharedInstance";

  external setFeedURL: (t, string) => unit = "oni2_SparkleSetFeedURL";
  external getFeedURL: t => string = "oni2_SparkleGetFeedURL";

  external setAutomaticallyChecksForUpdates: (t, bool) => unit =
    "oni2_SparkleSetAutomaticallyChecksForUpdates";
  external getAutomaticallyChecksForUpdates: t => bool =
    "oni2_SparkleGetAutomaticallyChecksForUpdates";

  external checkForUpdates: t => unit = "oni2_SparkleCheckForUpdates";
};

module Debug = {
  // WARNING: These have open type declarations, meaning you can pass
  // anything into them. They should only be used with NSObjects.
  external toString: _ => string = "oni2_SparkleDebugToString";
  external log: _ => unit = "oni2_SparkleDebugLog";
};

[%%else];

let init = () => ();

let version = None;

module Updater = {
  type t = unit;

  let getInstance = () => ();

  let setFeedURL = (_instance, _feedURL) => ();
  let getFeedURL = _instance => "UNIMPLEMENTED";

  let setAutomaticallyChecksForUpdates = (_instance, _check) => ();
  let getAutomaticallyChecksForUpdates = _instance => false;

  let checkForUpdates = _instance => ();
};

module Debug = {
  let toString = _ => "UNIMPLEMENTED";
  let log = _ => ();
};

[%%endif];

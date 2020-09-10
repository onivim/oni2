open Oni_Core;
open Oni_Core.Utility;

module Constants = {
  let baseUrl = "http://127.0.0.1:8080/_publish/this_is_fake";
};

type model = {
  automaticallyChecksForUpdates: bool,
  licenseKey: string,
};

let initial = {automaticallyChecksForUpdates: true, licenseKey: ""};

[@deriving show({with_path: false})]
type command =
  | CheckForUpdates;

[@deriving show({with_path: false})]
type msg =
  | Subscription(Service_AutoUpdate.msg)
  | Command(command);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

module Configuration = {
  open Config.Schema;

  let automaticallyChecksForUpdates =
    setting("oni.app.automaticallyChecksForUpdates", bool, ~default=true);
  let licenseKey = setting("oni.app.licenseKey", string, ~default="");
};

module Commands = {
  open Feature_Commands.Schema;

  let checkForUpdates =
    define(
      ~category="Oni2",
      ~title="Check for updates",
      "oni.app.checkForUpdates",
      Command(CheckForUpdates),
    );
};

module Contributions = {
  let configuration = [
    Configuration.automaticallyChecksForUpdates.spec,
    Configuration.licenseKey.spec,
  ];

  let commands = Commands.[checkForUpdates];
};

let update = (model, msg) =>
  switch (msg) {
  | Subscription(AutoCheckChanged(automaticallyChecksForUpdates)) => (
      {...model, automaticallyChecksForUpdates},
      Effect(
        Service_AutoUpdate.Effect.setAutomaticallyChecksForUpdates(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
          ~automaticallyChecksForUpdates,
        ),
      ),
    )
  | Subscription(LicenseKeyChanged(licenseKey)) => (
      {...model, licenseKey},
      Effect(
        Service_AutoUpdate.Effect.setFeedUrl(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
          ~url=Constants.baseUrl ++ "?licenseKey=" ++ licenseKey,
        ),
      ),
    )
  | Command(CheckForUpdates) => (
      model,
      Effect(
        Service_AutoUpdate.Effect.checkForUpdates(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
        ),
      ),
    )
  };

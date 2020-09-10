open Oni_Core;
open Oni_Core.Utility;

module Constants = {
  let baseUrl = "https://onivim.io/this_is_fake";
};

type model = {
  automaticallyChecksForUpdates: bool,
  licenseKey: string,
};

let initial = {automaticallyChecksForUpdates: true, licenseKey: ""};

[@deriving show({with_path: false})]
type msg =
  Service_AutoUpdate.msg =
    | AutoCheckChanged(bool) | LicenseKeyChanged(string);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

module Configuration = {
  open Config.Schema;

  let automaticallyChecksForUpdates =
    setting("oni.app.automaticallyChecksForUpdates", bool, ~default=true);
  let licenseKey = setting("oni.app.licenseKey", string, ~default="");
};

module Contributions = {
  let configuration = [
    Configuration.automaticallyChecksForUpdates.spec,
    Configuration.licenseKey.spec,
  ];
};

let update = (model, msg) =>
  switch (msg) {
  | AutoCheckChanged(automaticallyChecksForUpdates) => (
      {...model, automaticallyChecksForUpdates},
      Effect(
        Service_AutoUpdate.Effect.setAutomaticallyChecksForUpdates(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
          ~automaticallyChecksForUpdates,
        ),
      ),
    )
  | LicenseKeyChanged(licenseKey) => (
      {...model, licenseKey},
      Effect(
        Service_AutoUpdate.Effect.setFeedUrl(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
          ~url=Constants.baseUrl ++ licenseKey,
        ),
      ),
    )
  };

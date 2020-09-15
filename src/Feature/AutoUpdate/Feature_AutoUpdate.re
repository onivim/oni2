open Oni_Core;

module Constants = {
  let baseUrl = "https://v2.onivim.io/appcast";
};

type model = {
  automaticallyChecksForUpdates: bool,
  licenseKey: string,
  releaseChannel: [ | `Nightly | `Master | `Test],
};

let initial = {
  automaticallyChecksForUpdates: true,
  licenseKey: "",
  releaseChannel: `Master,
};

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

let releaseChannelToString =
  fun
  | `Nightly => "nightly"
  | `Master => "master"
  | `Test => "test-update";

let platformStr =
  switch (Revery.Environment.os) {
  | Mac => "macos"
  | Linux => "linux"
  | Windows => "windows"
  | _ => ""
  };

let urlOfState = state =>
  Constants.baseUrl
  ++ "?channel="
  ++ releaseChannelToString(state.releaseChannel)
  ++ "&licenseKey="
  ++ state.licenseKey
  ++ "&platform="
  ++ platformStr;

module Configuration = {
  open Config.Schema;

  let automaticallyChecksForUpdates =
    setting("oni.app.automaticallyChecksForUpdates", bool, ~default=true);
  let licenseKey = setting("oni.app.licenseKey", string, ~default="");

  let releaseChannelCodec =
    custom(
      ~decode=
        Json.Decode.(
          string
          |> map(
               fun
               | "nightly" => `Nightly
               | "test-update"
               | "test" => `Test
               | "master"
               | _ => `Master,
             )
        ),
      ~encode=
        Json.Encode.(
          fun
          | `Nightly => string("nightly")
          | `Master => string("master")
          | `Test => string("test-update")
        ),
    );

  let releaseChannel =
    setting(
      "oni.app.updateReleaseChannel",
      releaseChannelCodec,
      ~default=`Master,
    );
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
    Configuration.releaseChannel.spec,
  ];

  let commands = Commands.[checkForUpdates];
};

let sub = (~config) => {
  let automaticallyChecksForUpdates =
    Configuration.automaticallyChecksForUpdates.get(config);
  let licenseKey = Configuration.licenseKey.get(config);
  let releaseChannel = Configuration.releaseChannel.get(config);

  Service_AutoUpdate.Sub.autoUpdate(
    ~uniqueId="autoUpdate",
    ~licenseKey,
    ~releaseChannel,
    ~automaticallyChecksForUpdates,
  )
  |> Isolinear.Sub.map(msg => Subscription(msg));
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
  | Subscription(LicenseKeyChanged(licenseKey)) =>
    let newModel = {...model, licenseKey};
    (
      newModel,
      Effect(
        Service_AutoUpdate.Effect.setFeedUrl(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
          ~url=urlOfState(newModel),
        ),
      ),
    );
  | Subscription(ReleaseChannelChanged(releaseChannel)) =>
    let newModel = {...model, releaseChannel};
    (
      newModel,
      Effect(
        Service_AutoUpdate.Effect.setFeedUrl(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
          ~url=urlOfState(newModel),
        ),
      ),
    );
  | Command(CheckForUpdates) => (
      model,
      Effect(
        Service_AutoUpdate.Effect.checkForUpdates(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
        ),
      ),
    )
  };
